cmake_minimum_required(VERSION 3.10)
project(math_compiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(math-compiler
    main.cpp
    compiler.cpp
)

# Link with math library
target_link_libraries(math-compiler m)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install target
install(TARGETS math-compiler DESTINATION bin) 