cmake_minimum_required(VERSION 3.10)

project(cpuLoad_Linux)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include subfolders
#add_subdirectory(old_cpuLoad_standalone)
add_subdirectory(cpu_load_generator)
