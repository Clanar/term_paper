#include "thread_pool.h"
#include "inverted_index.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace fs = std::filesystem;

class TaskServer {
public:
    TaskServer(thread_pool<int>& worker_pool, thread_pool<int>& scheduler_pool, unsigned short port, const std::string& base_dir)
        : worker_pool_(worker_pool), scheduler_pool_(scheduler_pool), file_manager_(base_dir), port_(port) {}

    void start() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            throw std::runtime_error("Failed to create socket");
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw std::runtime_error("Bind failed");
        }

        if (listen(server_fd, 5) < 0) {
            throw std::runtime_error("Listen failed");
        }

        std::cout << "Server running on port " << port_ << std::endl;

        while (true) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd >= 0) {
                scheduler_pool_.add_task([this, client_fd] {
                    this->handle_client(client_fd);
                    return 0;
                });
            }
        }

        close(server_fd);
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

    void handle_client(int client_fd) {
        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read > 0) {
            std::istringstream stream(buffer);
            std::string command, filename, content;
            stream >> command;

            if (command == "ADD") {
                stream >> filename;
                std::getline(stream, content);
                if (file_manager_.add_file(filename, content)) {
                    worker_pool_.add_task([this, filename, content] {
                        index_.add_file(filename, content);
                        return 0;
                    });
                    send_response(client_fd, "File added successfully\n");
                } else {
                    send_response(client_fd, "Failed to add file\n");
                }
            } else if (command == "DELETE") {
                stream >> filename;
                if (file_manager_.delete_file(filename)) {
                    worker_pool_.add_task([this, filename] {
                        index_.delete_file(filename);
                        return 0;
                    });
                    send_response(client_fd, "File deleted successfully\n");
                } else {
                    send_response(client_fd, "Failed to delete file\n");
                }
            } else if (command == "SEARCH") {
                stream >> content;
                auto results = index_.search(content);
                std::ostringstream response;
                for (const auto& result : results) {
                    response << result << "\n";
                }
                send_response(client_fd, response.str());
            } else if (command == "CHECK_INDEX") {
                size_t count = index_.get_indexed_files_count();
            send_response(client_fd, std::to_string(count) + "\n");
            } else {
                send_response(client_fd, "Invalid command\n");
            }
        }

        close(client_fd);
    }

    void send_response(int client_fd, const std::string& message) {
        send(client_fd, message.c_str(), message.size(), 0);
    }

    thread_pool<int>& worker_pool_;
    thread_pool<int>& scheduler_pool_;
    FileManager file_manager_;
    InvertedIndex index_;
    unsigned short port_;
};

int main() {
    thread_pool<int> worker_pool;
    worker_pool.initialize(5);

    thread_pool<int> scheduler_pool;
    scheduler_pool.initialize(5);

    const unsigned short port = 12345;
    const std::string base_dir = "./server_files";
    TaskServer server(worker_pool, scheduler_pool, port, base_dir);

    try {
        server.start();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}