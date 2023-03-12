#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>
#include <mutex>
#include <condition_variable>

#define TOTALTIME (100) // milliseconds
#define LOADTIME(x) ((x * TOTALTIME) / 100)
#define FINITE_RUNTIME(x) (x >= 0 ? x : 0) // If runtime parameter is given (!= -1) set value else set 0

/* global variables */

int load = 0;     // CPU load
int CPU = -1;     // No CPU specified
int runtime = -1; // Initialize load running time
std::vector<std::thread> threads;
std::mutex m_mutex;
std::condition_variable m_condVar;
int threadsReadyCnt = 0;

/* functions */

void parse_arguments(int argc, char *argv[])
{
    int i;

    for (i = 2; i < argc; i++) /* Skip argv[0] (program name) and argv[1] (load). */
    {
        /* Process optional arguments. */
        if (strcmp(argv[i], "-c") == 0)
        {
            CPU = std::stoi(argv[i + 1]); // Logical CPU
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            runtime = std::stoi(argv[i + 1]); // Process runtime
        }
        else
        {
            /* Process non-optional arguments here. */
        }
    }
}

void generate_load(int load, int nr_of_threads, int instance)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    threadsReadyCnt++;
    while (threadsReadyCnt != nr_of_threads)
    {
        std::cout << "waiting.." << instance << std::endl;
        m_condVar.wait(lock);
    }
    std::cout << "starting.." << instance << std::endl;
    m_condVar.notify_all();
    lock.unlock();

    auto runtime_end = std::chrono::high_resolution_clock::now() + std::chrono::seconds(FINITE_RUNTIME(runtime));
    auto start = std::chrono::high_resolution_clock::now();

    while ((runtime < 0) || (std::chrono::high_resolution_clock::now() < runtime_end))
    {
        auto end = std::chrono::high_resolution_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (elapsed.count() >= LOADTIME(load))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(TOTALTIME - LOADTIME(load)));
            start = std::chrono::high_resolution_clock::now();
        }
    }
}

void all_cpu_load(int cores)
{
    for (int i = 0; i < cores; i++)
    {
        threads.push_back(std::thread(generate_load, load, cores, i));
    }
}

void one_cpu_load()
{
    std::cout << "CPU " << CPU << " will take the load" << std::endl;

    threads.push_back(std::thread(generate_load, load, 1, 0));

    /* create a CPU set structure and set to CPU to be used */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(CPU, &cpuset);

    /* set CPU scheduler affinity to the CPU set pointed by cpuset */
    pthread_setaffinity_np(threads[0].native_handle(), sizeof(cpu_set_t), &cpuset);
}

int main(int argc, char *argv[])
{
    /* Helper */
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <CPU load percentage>"
                  << " [-c <CPU Core>]"
                  << " [-r <runtime seconds>]" << std::endl;
        return 1;
    }

    // CPU load
    load = std::stoi(argv[1]);

    // Parse command line arguments
    parse_arguments(argc, argv);

    // Get number of CPU cores
    unsigned int cores = std::thread::hardware_concurrency();
    std::cout << cores << " CPU cores" << std::endl;

    if (CPU != -1)
    {
        one_cpu_load(); // Logical CPU to take the load is specified and will be used
    }
    else
    {
        all_cpu_load(cores); // No logical CPU selected, load distributed automatically
    }

    for (std::thread &t : threads)
    {
        t.join();
    }

    return 0;
}
