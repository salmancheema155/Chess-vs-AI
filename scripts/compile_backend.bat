@echo off
setlocal

set EMSDK_PATH=%~1
if "%EMSDK_PATH%"=="" (
	echo [INFO] emsdk_env.bat path was not specified
	echo [INFO] Using environment variable path instead
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

if not exist frontend\src\wasm (
    mkdir frontend\src\wasm
)

copy /Y build-wasm\backend\wasm_module.mjs frontend\src\wasm\wasm_module.mjs

echo [INFO] Compilation complete
pause