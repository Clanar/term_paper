#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <future>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <condition_variable>
#include <algorithm>
#include <chrono>
#include <arpa/inet.h>

namespace fs = std::filesystem;

class Client {
public:
    Client(const std::string& server_address, unsigned short server_port, size_t thread_count)
        : server_address_(server_address), server_port_(server_port), thread_count_(thread_count) {}

    void add_files(const std::string& folder_path) {
        std::vector<std::future<void>> futures;
        for (const auto& entry : fs::directory_iterator(folder_path)) {
            if (entry.is_regular_file()) {
                futures.emplace_back(std::async(std::launch::async, &Client::process_add_file, this, entry.path()));
            }
        }
        for (auto& future : futures) {
            future.get();
        }
    }

    void delete_files(const std::vector<std::string>& files_to_delete) {
        std::vector<std::future<void>> futures;
        for (const auto& filename : files_to_delete) {
            futures.emplace_back(std::async(std::launch::async, &Client::process_delete_file, this, filename));
        }
        for (auto& future : futures) {
            future.get();
        }
    }

    void search_word(const std::string& word) {
        std::vector<std::future<void>> futures;
        for (size_t i = 0; i < thread_count_; ++i) {
            futures.emplace_back(std::async(std::launch::async, &Client::process_search_word, this, word));
        }
        for (auto& future : futures) {
            future.get();
        }
    }

private:
    void process_add_file(const fs::path& file_path) {
        try {
            std::ifstream file(file_path, std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << file_path << "\n";
                return;
            }

            std::ostringstream content;
            content << file.rdbuf();
            file.close();

            std::string sanitized_content = content.str();
            std::replace(sanitized_content.begin(), sanitized_content.end(), '\n', ' ');

            std::string request = "ADD " + file_path.filename().string() + " " + sanitized_content + "\n";
            send_request(request);
        } catch (const std::exception& ex) {
            std::cerr << "Error processing file " << file_path << ": " << ex.what() << "\n";
        }
    }

    void process_delete_file(const std::string& filename) {
        try {
            std::string request = "DELETE " + filename + "\n";
            send_request(request);
        } catch (const std::exception& ex) {
            std::cerr << "Error deleting file " << filename << ": " << ex.what() << "\n";
        }
    }

    void process_search_word(const std::string& word) {
        try {
            std::string request = "SEARCH " + word + "\n";
            send_request(request);
        } catch (const std::exception& ex) {
            std::cerr << "Error searching for word " << word << ": " << ex.what() << "\n";
        }
    }

    void send_request(const std::string& request) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("Socket creation failed");
        }

        sockaddr_in server_address{};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(server_port_);
        server_address.sin_addr.s_addr = inet_addr(server_address_.c_str());

        if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
            close(sock);
            throw std::runtime_error("Connection failed");
        }

        send(sock, request.c_str(), request.size(), 0);

        char buffer[1024] = {0};
        read(sock, buffer, sizeof(buffer));

        std::cout << "Server response: " << buffer << std::endl;

        close(sock);
    }

    std::string server_address_;
    unsigned short server_port_;
    size_t thread_count_;
};

int main() {
    const std::string server_address = "127.0.0.1";
    const unsigned short server_port = 12345;
    const size_t thread_count = 4; // Change the number of threads here

    const std::string datasets_folder = "./datasets";
    if (!fs::exists(datasets_folder) || !fs::is_directory(datasets_folder)) {
        std::cerr << "Datasets folder does not exist or is not a directory.\n";
        return 1;
    }

    Client client(server_address, server_port, thread_count);

    auto overall_start_time = std::chrono::high_resolution_clock::now();

    try {
        std::cout << "Adding files to the server:\n";
        client.add_files(datasets_folder);

        std::cout << "Deleting files from the server:\n";
        client.delete_files({"5.txt", "6.txt"});

        std::cout << "Searching for the word \"program\":\n";
        client.search_word("program");

    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    auto overall_end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> overall_elapsed_seconds = overall_end_time - overall_start_time;

    std::cout << "Total execution time: " << overall_elapsed_seconds.count() << " seconds\n";

    return 0;
}
