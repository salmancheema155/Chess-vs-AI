# backend/CMakeLists.txt

set(This Backend)
project(${This} LANGUAGES CXX)

file(GLOB_RECURSE BACKEND_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER BACKEND_SOURCES EXCLUDE REGEX ".+_debug\\.cpp$")

add_library(${This} STATIC ${BACKEND_SOURCES})

target_include_directories(${This} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()