set( SOURCES
    main.cpp
    cpu_load_generator_impl.cpp
)

# Create an executable from the gathered sources
add_executable(cpuLoad_Linux ${SOURCES})
target_link_libraries(cpuLoad_Linux pthread)
