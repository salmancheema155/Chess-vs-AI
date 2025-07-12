# backend/CMakeLists.txt

set(This Backend)

file(GLOB_RECURSE BACKEND_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(${This} STATIC ${BACKEND_SOURCES})

target_include_directories(${This} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()

if(EMSCRIPTEN)
	set(wasm_target ${This}_wasm)

	add_executable(${wasm_target} ${BACKEND_SOURCES})
	target_include_directories(${wasm_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

	target_link_options(${wasm_target} PRIVATE
		"-s WASM=1"
		"-s EXPORTED_FUNCTIONS=['_initialiseGame', '_getLegalMoves', '_makeMove', '_undo']"
		"-s EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']"
	)

	set_target_properties(${wasm_target} PROPERTIES OUTPUT_NAME "chess_backend")
endif()