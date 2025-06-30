@echo off
setlocal

cd /d "%~dp0"

set testName=BoardTests
set testFolder=board\

call tests_setup.bat "%testName%" "%testFolder%" %*

endlocal
pause