@echo off

cd /d "%~dp0.."

cmake -S . -B build
cmake --build build --target BackendTests --config Debug --parallel

if exist "build/backend/tests/Debug/BackendTests.exe" (
	echo Running BackendTests...
	"build/backend/tests/Debug/BackendTests.exe" --gtest_color=yes --gtest_print_time --gtest_brief=0 %*
) else (
	echo BackendTests.exe was not found!
)

pause