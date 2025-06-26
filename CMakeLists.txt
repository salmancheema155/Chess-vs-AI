# ROOT

cmake_minimum_required(VERSION 3.15)

set(This Chess)
project(${This} LANGUAGES CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(BUILD_TESTING "Enable Test Builds" ON)
if(BUILD_TESTING)
	enable_testing()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(backend)