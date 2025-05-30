cmake_minimum_required(VERSION 3.16)
project(SecureDiskManagerPro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Root include paths
include_directories(
    ${PROJECT_SOURCE_DIR}/core
    ${PROJECT_SOURCE_DIR}/cli
    ${PROJECT_SOURCE_DIR}/utils
    ${PROJECT_SOURCE_DIR}/identity
    ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/include
    ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/plugins
    ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/net
)

# Gather source files
file(GLOB_RECURSE CORE_SRC         ${PROJECT_SOURCE_DIR}/core/*.cpp)
file(GLOB_RECURSE CLI_SRC          ${PROJECT_SOURCE_DIR}/cli/*.cpp)
file(GLOB_RECURSE UTILS_SRC        ${PROJECT_SOURCE_DIR}/utils/*.cpp)
file(GLOB_RECURSE IDENTITY_SRC     ${PROJECT_SOURCE_DIR}/identity/*.cpp)
file(GLOB_RECURSE GHOST_SRC        ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/src/*.cpp)
file(GLOB_RECURSE GHOST_NET_SRC    ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/net/*.cpp)
file(GLOB_RECURSE GHOST_PLUGIN_SRC ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/plugins/*.cpp)

# Optional: allow .rs files to be tracked (not compiled here)
file(GLOB_RECURSE GHOST_RS_SRC ${PROJECT_SOURCE_DIR}/GhostMode_Vanisher/plugins/*.rs)

add_executable(SecureDiskManagerPro
    ${CORE_SRC}
    ${CLI_SRC}
    ${UTILS_SRC}
    ${IDENTITY_SRC}
    ${GHOST_SRC}
    ${GHOST_NET_SRC}
    ${GHOST_PLUGIN_SRC}
)

find_package(CURL REQUIRED)
target_link_libraries(SecureDiskManagerPro PRIVATE CURL::libcurl)
target_link_libraries(SecureDiskManagerPro PRIVATE crypt32 tinyxml2)

# Optional: Link system libraries here
# target_link_libraries(SecureDiskManagerPro PRIVATE zstd ssl crypto curl pthread)

# Print build summary
message(STATUS "SecureDiskManagerPro configured.")
