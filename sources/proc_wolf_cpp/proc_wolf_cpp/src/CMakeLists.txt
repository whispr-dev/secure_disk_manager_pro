set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include core)

# Find required packages
find_package(SQLite3 REQUIRED)

# Core engine
add_library(procwolf_core
    core/ProcWolfEngine.cpp
)

# CLI tool
add_executable(ProcWolfCLI
    cli/ProcWolfCLI.cpp
)

# Link libraries
target_link_libraries(ProcWolfCLI
    procwolf_core
    SQLite::SQLite3
    psapi
)

# Windows-specific flags
if (WIN32)
    target_link_libraries(ProcWolfCLI
        ws2_32
    )
endif()

# Service app
add_executable(ProcWolfService
    service/ProcWolfService.cpp
)
target_link_libraries(ProcWolfService
    procwolf_core
    SQLite::SQLite3
    psapi
    advapi32
)

# Stealth Tray App
add_executable(ProcWolfTray
    stealth/ProcWolfTray.cpp
)
target_link_libraries(ProcWolfTray
    procwolf_core
    SQLite::SQLite3
    psapi
    shell32
    user32
    gdi32
)

# Whitelist Manager GUI
add_executable(WhitelistManager
    tools/WhitelistManager.cpp
)
target_link_libraries(WhitelistManager
    user32
    gdi32
)

# Icon Resource
set(APP_ICON resources/ProcWolfResources.rc)
enable_language(RC)

# Function to link resource to executables
macro(link_icon exe)
    target_sources(${exe} PRIVATE ${APP_ICON})
endmacro()

# Apply to all executables
link_icon(ProcWolfCLI)
link_icon(ProcWolfService)
link_icon(ProcWolfTray)
link_icon(WhitelistManager)