#include "cpu_load_generator_impl.hpp"
#include <cstring>
#include <csignal>

/* global variables */

static int load = 0;     // CPU load
static int CPU = -1;     // No CPU specified
static int runtime = -1; // Initialize load running time
CPULoadGenerator loadGenerator;

/* functions */

void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    loadGenerator.stop();

    exit(signum);
}

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

    // register signal SIGINT and signal handler
    signal(SIGINT, signalHandler);

    load = std::stoi(argv[1]);
    // Validate load value between 1-100
    if ((load < 1) || (load > 100))
    {
        std::cout << "Load value not between 1-100 % !" << std::endl;
        return 0;
    }

    // Parse command line arguments
    parse_arguments(argc, argv);

    loadGenerator.start(load, CPU);
    if (runtime != -1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(runtime));
        loadGenerator.stop();
    }
    else
    {
        // wait infinite
        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}