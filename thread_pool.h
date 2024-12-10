#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "task_queue.h"
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <vector>
#include <tuple>

template <typename Result>
class thread_pool {
public:
    thread_pool() = default;
    ~thread_pool();

    void initialize(const size_t worker_count);
    void terminate();
    void pause();
    void resume();
    bool working() const;

    template <typename Function>
int add_task(Function&& func);


    Result get_results(int id);

    const task_queue& get_task_queue() const;

private:
    void routine();

    mutable std::mutex m_rw_lock;
    mutable std::mutex m_add_task;
    mutable std::mutex m_pause_mutex;
    std::condition_variable_any m_task_waiter;

    std::vector<std::thread> m_workers;
    task_queue m_tasks;
    std::map<int, Result> m_results;

    bool m_initialized = false;
    bool m_terminated = false;
    bool m_paused = false;
    int lastId = 0;
};

#include "thread_pool.cpp"

#endif // THREAD_POOL_H