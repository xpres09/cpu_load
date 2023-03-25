#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <exception>
#include <condition_variable>

#pragma once

class CPULoadGenerator
{
public:
    void start(int load, int cpu = -1);
    void stop();

private:
    int load{0};
    int cpu{-1};
    int threadsReadyCnt{0};
    bool started{false};
    std::vector<std::thread> threads_vector;
    std::mutex m_mutex;
    std::condition_variable m_condVar;

    void generate_load(int load, int nr_of_threads);
    void one_cpu_load();
    void all_cpu_load();
};
