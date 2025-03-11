cmake_minimum_required(VERSION 3.25)

set (CMAKE_CXX_STANDARD 20)

project(ACCU2025)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Use the CMake Package Manager
include(CPM)

# Download stdexec
CPMAddPackage(
  NAME stdexec
  GITHUB_REPOSITORY NVIDIA/stdexec
  GIT_TAG main
)

set (STDEXEC_BUILD_TESTS OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(timer_example sender_and_receivers_timer.cpp)

    target_link_libraries(timer_example STDEXEC::stdexec)
    target_link_libraries(timer_example Threads::Threads)
    target_link_libraries(timer_example ${Boost_LIBRARIES})
endif()
