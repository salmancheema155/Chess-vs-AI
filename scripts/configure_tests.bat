@echo off

echo [INFO] Configuring project with CMake

cd /d "%~dp0.."
cmake -S . -B build-tests -DBUILD_TESTING=ON -DEMSCRIPTEN=OFF

if %errorlevel% neq 0 (
	echo [ERROR] CMake configuration failed
	pause
	exit /b %errorlevel%
)

echo [INFO] Configuration complete
pause