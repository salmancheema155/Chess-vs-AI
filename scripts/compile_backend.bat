@echo off
setlocal

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

if not exist build-wasm (
    echo [ERROR] build-wasm directory was not found - please run ./scripts/configure_backend.bat first
    pause
	exit /b
)

cd build-wasm

cmake --build . --config Release 

cd ..

if not exist frontend\public\wasm (
    mkdir frontend\public\wasm
)

copy /Y build-wasm\backend\chess_backend.js frontend\public\wasm\chess_backend.js
copy /Y build-wasm\backend\chess_backend.wasm frontend\public\wasm\chess_backend.wasm

echo [INFO] Compilation complete
pause