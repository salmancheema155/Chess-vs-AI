@echo off
setlocal

cd /d "%~dp0.."

set testName=%~1
set testFolder=%~2
set exePath=build/backend/tests/%testFolder%Debug/%testName%.exe

shift
shift

if not exist "build" (
	echo [ERROR] build directory was not found - please run ./scripts/configure.bat first
	exit /b
)

cmake --build build --target %testName% --config Debug --parallel

if exist "%exePath%" (
	echo [INFO] "Running %testName%..."
	"%exePath%" --gtest_color=yes --gtest_print_time %*
) else (
	echo [ERROR] "%testName%.exe was not found!"
)

endlocal
