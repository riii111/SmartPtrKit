cmake_minimum_required(VERSION 3.14)
project(SmartPtrKit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Main library
add_library(smart_ptr_kit
    src/unique_ptr.cpp
    src/shared_ptr.cpp
    src/weak_ptr.cpp
)

target_include_directories(smart_ptr_kit PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Tests
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS smart_ptr_kit
    EXPORT smart_ptr_kitTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)