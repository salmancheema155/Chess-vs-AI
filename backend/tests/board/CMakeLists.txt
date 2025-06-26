# backend/tests/board/CMakeLists.txt

set(This, BoardTests)

add_executable(${This}
	${CMAKE_CURRENT_SOURCE_DIR}/../test_main.cpp
	board_tests.cpp
	board_utilities.cpp
)

target_link_libraries(${This} PRIVATE Backend gtest_main)
target_include_directories(${This} PRIVATE 
	${CMAKE_SOURCE_DIR}/backend/include
	${CMAKE_SOURCE_DIR}/backend
)

gtest_discover_tests(${This})