@echo off
setlocal

cd /d "%~dp0.."

set testName=%~1
set testFolder=%~2
set exePath=build-tests/backend/tests/%testFolder%Debug/%testName%.exe

shift
shift

if not exist "build-tests" (
	echo [ERROR] build-tests directory was not found - please run ./scripts/configure_tests.bat first
	exit /b
)

cmake --build build-tests --target %testName% --config Debug --parallel

if exist "%exePath%" (
	echo [INFO] "Running %testName%..."
	"%exePath%" --gtest_color=yes --gtest_print_time %*
) else (
	echo [ERROR] "%testName%.exe was not found!"
)

endlocal
