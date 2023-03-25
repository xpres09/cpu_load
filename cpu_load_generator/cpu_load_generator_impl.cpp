#include "cpu_load_generator_impl.hpp"
#include <sched.h>

#define TOTALTIME (100)                     // Time unit in milliseconds
#define LOADTIME(x) ((x * TOTALTIME) / 100) // Convert load in percentage

void CPULoadGenerator::start(int load, int cpu)
{
    this->load = load;
    this->cpu = cpu;
    started = true; // Set started flag to true to check in generate_load while loop

    if (cpu != -1)
    {
        one_cpu_load(); // Logical CPU to take the load is specified and will be used
    }
    else
    {
        all_cpu_load(); // No logical CPU selected, load distributed automatically
    }
    std::cout << "Started generating CPU load.." << std::endl;
}

void CPULoadGenerator::stop()
{
    // If started set variable to false to exit from generate_load while loop
    if (true == started)
    {
        started = false;
        for (std::thread &tmpThread : threads_vector)
        {
            tmpThread.join();
        }
        std::cout << "Stopped generating CPU load.." << std::endl;
    }
    else
    {
        std::cout << "Generator already stopped.." << std::endl;
    }
}

void CPULoadGenerator::generate_load(int load, const int nr_of_threads)
{
    // Sync threads
    std::unique_lock<std::mutex> lock(m_mutex);
    threadsReadyCnt++;
    if (threadsReadyCnt != nr_of_threads)
    {
        m_condVar.wait(lock);
    }
    else
    {
        m_condVar.notify_all();
        threadsReadyCnt = 0; // Reset check variable
    }
    lock.unlock();

    auto start = std::chrono::high_resolution_clock::now();
    while (started)
    {
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (elapsed.count() >= LOADTIME(load))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(TOTALTIME - LOADTIME(load))); // Sleep rest of time
            start = std::chrono::high_resolution_clock::now();
        }
    }
}

void CPULoadGenerator::one_cpu_load()
{
    std::cout << "CPU " << cpu << " will take the load" << std::endl;

    threads_vector.push_back(std::thread([this]
                                         { generate_load(load, 1); }));

    /* create a CPU set structure and set to CPU to be used */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    /* set CPU scheduler affinity to the CPU set pointed by cpuset */
    pthread_setaffinity_np(threads_vector[0].native_handle(), sizeof(cpu_set_t), &cpuset);
}

void CPULoadGenerator::all_cpu_load()
{
    // Get number of CPU cores
    try
    {
        int cores = std::thread::hardware_concurrency();
        for (int i = 0; i < cores; i++)
        {
            threads_vector.push_back(std::thread([this, cores]
                                                 { generate_load(load, cores); }));
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "Failed to get number of hardware CPU cores" << e.what() << std::endl;
        return;
    }
}