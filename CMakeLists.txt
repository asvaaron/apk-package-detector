cmake_minimum_required(VERSION 3.16)
project(pkg_lang_info VERSION 1.0.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Compiler settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# Core library: helpers + tables + output (shared by the app and the tests)
# ---------------------------------------------------------------------------
add_library(pkg_lang_info_core STATIC
    src/helpers.cpp
    src/tables.cpp
    src/output.cpp
)
target_include_directories(pkg_lang_info_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
add_executable(pkg_lang_info
    src/main.cpp
)
target_link_libraries(pkg_lang_info PRIVATE pkg_lang_info_core)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_executable(test_pkg_lang_info tests/test_pkg_lang_info.cpp)
target_link_libraries(test_pkg_lang_info PRIVATE pkg_lang_info_core)
add_test(NAME pkg_lang_info_smoke COMMAND test_pkg_lang_info)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(TARGETS pkg_lang_info RUNTIME DESTINATION bin)
