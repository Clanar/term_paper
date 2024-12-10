#include "task_queue.h"

bool task_queue::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.empty();
}

size_t task_queue::size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

void task_queue::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    while (!m_tasks.empty()) {
        m_tasks.pop();
    }
}

bool task_queue::pop(std::tuple<int, std::function<void(int)>> & task) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_tasks.empty()) {
        return false;
    }
    task = std::move(m_tasks.front());
    m_tasks.pop();
    return true;
}

bool task_queue::emplace(std::tuple<int, std::function<void(int)>> task ) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_tasks.size() >= 20) {
        if (!is_full) {
            is_full = true;
            full_time = std::chrono::steady_clock::now();
        }
        missing_tasks++;
        return false;
    }
    m_tasks.emplace(task);
    return true;
}

int task_queue::get_missing_tasks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return missing_tasks;
}

std::chrono::steady_clock::time_point task_queue::get_full_time() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return full_time;
}
