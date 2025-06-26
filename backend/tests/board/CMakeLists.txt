# backend/tests/board/CMakeLists.txt

set(This BoardTests)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${This}
	${TEST_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/../../src/board/board_debug.cpp
)

target_link_libraries(${This} PRIVATE Backend gtest_main)
target_include_directories(${This} PRIVATE 
	${CMAKE_SOURCE_DIR}/backend/include
	${CMAKE_SOURCE_DIR}/backend
)

gtest_discover_tests(${This})