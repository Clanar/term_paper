#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

void send_request(tcp::socket& socket, const std::string& request) {
    boost::asio::write(socket, boost::asio::buffer(request));
}

std::string receive_response(tcp::socket& socket) {
    boost::asio::streambuf buffer;
    boost::asio::read(socket, buffer, boost::asio::transfer_at_least(1));
    std::istream response_stream(&buffer);
    std::ostringstream response;
    response << response_stream.rdbuf();
    return response.str();
}

void add_files(const std::string& server_address, unsigned short server_port, const std::string& folder_path) {
    boost::asio::io_context io_context;

    for (const auto& entry : fs::directory_iterator(folder_path)) {
        if (entry.is_regular_file()) {
            try {
                std::ifstream file(entry.path(), std::ios::binary); 
                if (!file.is_open()) {
                    std::cerr << "Failed to open file: " << entry.path() << "\n";
                    continue; 
                }

                std::ostringstream content;
                content << file.rdbuf();
                file.close();

                tcp::socket socket(io_context);
                tcp::resolver resolver(io_context);
                boost::asio::connect(socket, resolver.resolve(server_address, std::to_string(server_port)));

                std::string sanitized_content = content.str();
                std::replace(sanitized_content.begin(), sanitized_content.end(), '\n', ' ');

                std::string request = "ADD " + entry.path().filename().string() + " " + sanitized_content + "\n";
                send_request(socket, request);

                std::string response = receive_response(socket);
                std::cout << "Server response for " << entry.path().filename().string() << ": " << response << "\n";

                socket.close();

            } catch (const std::exception& ex) {
                std::cerr << "Error processing file " << entry.path() << ": " << ex.what() << "\n";
                continue;
            }
        }
    }
}



void delete_files(const std::string& server_address, unsigned short server_port, const std::vector<std::string>& files_to_delete) {
    boost::asio::io_context io_context;

    for (const auto& filename : files_to_delete) {
        try {
            tcp::socket socket(io_context);
            tcp::resolver resolver(io_context);
            boost::asio::connect(socket, resolver.resolve(server_address, std::to_string(server_port)));

            std::string request = "DELETE " + filename + "\n";
            send_request(socket, request);

            std::string response = receive_response(socket);
            std::cout << "Server response for DELETE " << filename << ": " << response << "\n";

            socket.close();

        } catch (const std::exception& ex) {
            std::cerr << "Error deleting file " << filename << ": " << ex.what() << "\n";
            continue;
        }
    }
}


void search_word(const std::string& server_address, unsigned short server_port, const std::string& word) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(socket, resolver.resolve(server_address, std::to_string(server_port)));

        std::string request = "SEARCH " + word + "\n";
        send_request(socket, request);

        std::string response = receive_response(socket);
        std::cout << "Search results for \"" << word << "\":\n" << response << "\n";

        socket.close();

    } catch (const std::exception& ex) {
        std::cerr << "Error searching for word \"" << word << "\": " << ex.what() << "\n";
    }
}


int main() {
    const std::string server_address = "127.0.0.1";
    const unsigned short server_port = 12345;

    const std::string datasets_folder = "./datasets";
    if (!fs::exists(datasets_folder) || !fs::is_directory(datasets_folder)) {
        std::cerr << "Datasets folder does not exist or is not a directory.\n";
        return 1;
    }

    try {
        std::cout << "Adding files to the server:\n";
        add_files(server_address, server_port, datasets_folder);

        std::cout << "Deleting files from the server:\n";
        delete_files(server_address, server_port, {"14984.txt", "14997.txt"}); 

        std::cout << "Searching for the word \"program\":\n";
        search_word(server_address, server_port, "program");

    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}
