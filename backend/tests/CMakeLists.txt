# backend/tests/CMakeLists.txt

set(This BackendTests)

include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${This} 
	${TEST_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/board/board_debug.cpp)

target_link_libraries(${This} PRIVATE Backend gtest_main)

target_include_directories(${This} PRIVATE 
	${CMAKE_SOURCE_DIR}/backend/include
	${CMAKE_SOURCE_DIR}/backend
)

add_subdirectory(board)
add_subdirectory(move)

gtest_discover_tests(${This})