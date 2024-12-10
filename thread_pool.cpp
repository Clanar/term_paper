#include "thread_pool.h"
#include <iostream>
#include <chrono>
#include <memory>

template <typename Result>
thread_pool<Result>::~thread_pool() {
    terminate();
}

template <typename Result>
void thread_pool<Result>::initialize(const size_t worker_count) {
    terminate();
    m_initialized = true;
    m_terminated = false;
    m_workers.reserve(worker_count);
    for (size_t i = 0; i < worker_count; ++i) {
        m_workers.emplace_back([this] { routine(); });
    }
}

template <typename Result>
void thread_pool<Result>::terminate() {
    if (!m_initialized)
        return;

    {
        std::unique_lock<std::mutex> lock(m_rw_lock);
        m_terminated = true;
    }
    m_task_waiter.notify_all();

    for (auto &worker : m_workers) {
        if (worker.joinable())
            worker.join();
    }

    m_initialized = false;
}

template <typename Result>
void thread_pool<Result>::pause() {
    {
        std::unique_lock<std::mutex> lock(m_pause_mutex);
        m_paused = true;
        m_task_waiter.wait(lock);
    }
    std::cout << "Pause" << std::endl;
}

template <typename Result>
void thread_pool<Result>::resume() {
    {
        std::unique_lock<std::mutex> lock(m_pause_mutex);
        m_paused = false;
        m_task_waiter.notify_all();
    }
    std::cout << "Resume" << std::endl;
}

template <typename Result>
void thread_pool<Result>::routine() {
    while (true) {
        std::tuple<int, std::function<void(int)>> task;
        {
            std::unique_lock<std::mutex> lock(m_rw_lock);
            m_task_waiter.wait(lock, [this] { return m_terminated || !m_tasks.empty(); });
            if (m_terminated && m_tasks.empty())
                return;
            if (m_paused)
                continue;
            m_tasks.pop(task);
        }
        std::get<1>(task)(std::get<0>(task));
        std::this_thread::sleep_for(std::chrono::seconds(5 + std::rand() % 6));
    }
}

template <typename Result>
bool thread_pool<Result>::working() const {
    std::lock_guard<std::mutex> lock(m_rw_lock);
    return m_initialized && !m_terminated;
}

template <typename Result>
template <typename Function>
int thread_pool<Result>::add_task(Function&& func) {
    {
        std::lock_guard<std::mutex> lock(m_add_task);
        lastId++;

        std::function<void(int)> wrapped_task = [func = std::forward<Function>(func)](int id) {
            func();
        };

        m_tasks.emplace({lastId, std::move(wrapped_task)});
        m_task_waiter.notify_one();
    }

    return lastId;
}

template <typename Result>
Result thread_pool<Result>::get_results(int id) {
    std::cout << "Getting results from the map " << id << std::endl;
    std::lock_guard<std::mutex> lock(m_rw_lock);
    return m_results[id];
}

template <typename Result>
const task_queue& thread_pool<Result>::get_task_queue() const {
    return m_tasks;
}