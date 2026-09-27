# ============================================================================
# CMakeLists.txt — pkg_lang_info
#
# Layout:
#   include/   public headers  (helpers.h, tables.h, output.h)
#   src/       implementation (main.cpp, helpers.cpp, tables.cpp, output.cpp)
#
# Build:
#   cmake -B build
#   cmake --build build
#   ./build/bin/pkg_lang_info
# ============================================================================

cmake_minimum_required(VERSION 3.16)

project(pkg_lang_info
    VERSION 1.0.0
    DESCRIPTION "Show installed package managers and programming languages on Unix systems"
    LANGUAGES CXX
)

# ----------------------------------------------------------------------------
# Compiler settings
# ----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)   # gnu++17 mode: keeps POSIX APIs (strtok_r, popen) visible
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # compile_commands.json for tooling

# Default to an optimized build when the user hasn't picked one
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Release/Debug)" FORCE)
endif()

# ----------------------------------------------------------------------------
# Executable
# ----------------------------------------------------------------------------
add_executable(pkg_lang_info
    src/main.cpp
    src/helpers.cpp
    src/tables.cpp
    src/output.cpp
)

target_include_directories(pkg_lang_info PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(pkg_lang_info PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra>
)

# Convenience: put the binary in <build>/bin
set_target_properties(pkg_lang_info PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# ----------------------------------------------------------------------------
# Install (optional: cmake --install build  or  make install)
# ----------------------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS pkg_lang_info
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
