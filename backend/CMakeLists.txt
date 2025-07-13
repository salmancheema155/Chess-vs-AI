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

	target_compile_options(${wasm_target} PRIVATE -O3)

	target_link_options(${wasm_target} PRIVATE
		"-sWASM=1"
		"-sMODULARIZE=1"
		"-sEXPORT_NAME=createModule"
		"-sSINGLE_FILE=1"
		"-sEXPORTED_FUNCTIONS=['_initialiseGame', '_getLegalMoves', '_makeMove', '_undo']"
		"-sEXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'UTF8ToString']"
	)

	set_target_properties(${wasm_target} PROPERTIES 
		OUTPUT_NAME "wasm_module"
		SUFFIX ".mjs"
	)
endif()