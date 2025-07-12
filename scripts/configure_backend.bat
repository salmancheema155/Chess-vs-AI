@echo off
setlocal

echo [INFO] Configuring project with CMake

set EMSDK_PATH=%~1
if "%EMSDK_PATH%"=="" (
	echo [INFO] emsdk_env.bat path was not specified
	echo [INFO] Using environment variable path
) else (
	if not exist "%EMSDK_PATH%" (
		echo [ERROR] Could not find "%EMSDK_PATH%"
		pause
		exit /b 1
	)
	call "%EMSDK_PATH%"
)

cd /d "%~dp0.."
cmake -G "Ninja" -S . -B build-wasm -DBUILD_TESTING=OFF -DEMSCRIPTEN=ON -DCMAKE_TOOLCHAIN_FILE=%EMSDK%/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

if %errorlevel% neq 0 (
	echo [ERROR] CMake configuration failed
	pause
	exit /b %errorlevel%
)

echo [INFO] Configuration complete
pause