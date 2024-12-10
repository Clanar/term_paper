#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <functional>
#include <mutex>
#include <tuple>
#include <chrono>

class task_queue {
    using task_queue_implementation = std::queue<std::tuple<int, std::function<void(int)>>>;

public:
    task_queue() = default;
    ~task_queue() { clear(); }
    bool empty() const;
    size_t size() const;
    void clear();
    bool pop(std::tuple<int, std::function<void(int)>> &task);
    bool emplace(std::tuple<int, std::function<void(int)>> task);
    int get_missing_tasks() const;
    std::chrono::steady_clock::time_point get_full_time() const;

private:
    mutable std::mutex m_mutex;
    task_queue_implementation m_tasks;
    int missing_tasks = 0;
    bool is_full = false;
    std::chrono::steady_clock::time_point full_time;
};

#include "task_queue.cpp"

#endif // TASK_QUEUE_H
