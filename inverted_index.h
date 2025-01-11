#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>

class InvertedIndex {
private:
    std::unordered_map<std::string, std::vector<std::string>> index_;
    mutable std::mutex index_mutex_;
    size_t indexed_files_count_ = 0; 

public:
    void add_file(const std::string& filename, const std::string& content);
    void delete_file(const std::string& filename);
    std::vector<std::string> search(const std::string& term) const;
    size_t get_indexed_files_count() const;
};

#endif // INVERTED_INDEX_H