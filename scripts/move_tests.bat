@echo off
setlocal

cd /d "%~dp0"

set testName=MoveTests
set testFolder=move\

call tests_setup.bat "%testName%" "%testFolder%" %*

endlocal
pause