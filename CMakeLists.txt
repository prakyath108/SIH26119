cmake_minimum_required(VERSION 3.20)
project(SovereignOptimization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_SERVER "Build solver HTTP/WebSocket server" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_PYTHON_BINDINGS "Build Python bindings via pybind11" OFF)

# Find dependencies
find_package(Threads REQUIRED)

# Crow for HTTP server (fetch if not found)
if (BUILD_SERVER)
    find_package(Crow REQUIRED)
    find_package(nlohmann_json REQUIRED)
endif()

# Core library
add_subdirectory(src)

if (BUILD_SERVER)
    add_subdirectory(backend)
endif()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install targets
install(TARGETS sop_lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/sop/ DESTINATION include/sop)