@echo off

cd /d "%~dp0.."

cmake -S . -B build
cmake --build build --target BoardTests --config Debug --parallel

if exist "build/backend/tests/board/Debug/BoardTests.exe" (
	echo Running BoardTests...
	"build/backend/tests/board/Debug/BoardTests.exe" --gtest_color=yes --gtest_print_time --gtest_brief=0 %*
) else (
	echo BoardTests.exe was not found!
)

pause