#include "inverted_index.h"
#include <sstream>

void InvertedIndex::add_file(const std::string& filename, const std::string& content) {
    std::lock_guard<std::mutex> lock(index_mutex_);

    std::istringstream stream(content);
    std::string word;
    while (stream >> word) {
        index_[word].push_back(filename);
    }
    indexed_files_count_++;
}

void InvertedIndex::delete_file(const std::string& filename) {
    std::lock_guard<std::mutex> lock(index_mutex_);

    for (auto& [word, files] : index_) {
        files.erase(std::remove(files.begin(), files.end(), filename), files.end());
    }
    indexed_files_count_--;
}

std::vector<std::string> InvertedIndex::search(const std::string& term) const {
    std::lock_guard<std::mutex> lock(index_mutex_);

    auto it = index_.find(term);
    if (it != index_.end()) {
        return it->second;
    }
    return {};
}

size_t InvertedIndex::get_indexed_files_count() const {
    std::lock_guard<std::mutex> lock(index_mutex_);
    return indexed_files_count_;
}