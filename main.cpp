#include "thread_pool.h"
#include "inverted_index.h"
#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

class TaskServer {
public:
    TaskServer(thread_pool<int>& pool, unsigned short port, const std::string& base_dir)
        : pool_(pool), file_manager_(base_dir), acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {}

    void start() {
        accept_connections();
        io_context_.run();
    }

private:
    class FileManager {
    public:
        FileManager(const std::string& base_dir) : base_dir_(base_dir) {
            if (!fs::exists(base_dir_)) {
                fs::create_directory(base_dir_);
            }
        }

        bool add_file(const std::string& filename, const std::string& content) {
            std::ofstream file(base_dir_ + "/" + filename, std::ios::out);
            if (file.is_open()) {
                file << content;
                file.close();
                return true;
            }
            return false;
        }

        bool delete_file(const std::string& filename) {
            return fs::remove(base_dir_ + "/" + filename);
        }

        std::string read_file(const std::string& filename) {
            std::ifstream file(base_dir_ + "/" + filename);
            std::ostringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

    private:
        std::string base_dir_;
    };

    void accept_connections() {
        auto socket = std::make_shared<tcp::socket>(io_context_);
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code error) {
            if (!error) {
                handle_client(socket);
            }
            accept_connections();
        });
    }

    void handle_client(std::shared_ptr<tcp::socket> socket) {
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(*socket, *buffer, '\n',
            [this, socket, buffer](boost::system::error_code error, std::size_t) {
                if (!error) {
                    std::istream stream(buffer.get());
                    std::string command, filename, content;
                    stream >> command;

                    if (command == "ADD") {
                        stream >> filename;
                        std::getline(stream, content);
                        if (file_manager_.add_file(filename, content)) {
                            pool_.add_task([this, filename, content] {
                                index_.add_file(filename, content);
                                return 0;
                            });
                            respond(socket, "File added successfully\n");
                        } else {
                            respond(socket, "Failed to add file\n");
                        }
                    } else if (command == "DELETE") {
                        stream >> filename;
                        if (file_manager_.delete_file(filename)) {
                            pool_.add_task([this, filename] {
                                index_.delete_file(filename);
                                return 0;
                            });
                            respond(socket, "File deleted successfully\n");
                        } else {
                            respond(socket, "Failed to delete file\n");
                        }
                    } else if (command == "SEARCH") {
                        stream >> content;
                        auto results = index_.search(content);
                        std::ostringstream response;
                        for (const auto& result : results) {
                            response << result << "\n";
                        }
                        respond(socket, response.str());
                    } else {
                        respond(socket, "Invalid command\n");
                    }
                }
            });
    }

    void respond(std::shared_ptr<tcp::socket> socket, const std::string& message) {
        boost::asio::async_write(*socket, boost::asio::buffer(message),
            [socket](boost::system::error_code, std::size_t) {});
    }

    thread_pool<int>& pool_;
    FileManager file_manager_;
    InvertedIndex index_;
    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
};

int main() {
    thread_pool<int> pool;
    pool.initialize(6);

    const unsigned short port = 12345;
    const std::string base_dir = "./server_files";
    TaskServer server(pool, port, base_dir);

    std::cout << "Server running on port " << port << std::endl;
    server.start();

    return 0;
}
