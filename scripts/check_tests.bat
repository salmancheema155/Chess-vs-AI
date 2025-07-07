@echo off
setlocal

cd /d "%~dp0"

set testName=CheckTests
set testFolder=check\

call tests_setup.bat "%testName%" "%testFolder%" %*

endlocal
pause