cmake_minimum_required(VERSION 3.16)
project(SecureDiskManagerPro)

set(CMAKE_CXX_STANDARD 17)

# Source directories
file(GLOB_RECURSE SOURCES
    src/*.cpp
    core/*.cpp
    utils/*.cpp
    cli/*.cpp
    plugins/*.cpp
    net/*.cpp
)

# Header paths
include_directories(
    include
    core
    utils
    cli
    plugins
    net
)

add_executable(SecureDiskManagerPro ${SOURCES})

# Link libraries if needed
# target_link_libraries(SecureDiskManagerPro PRIVATE pthread zstd crypto curl)
