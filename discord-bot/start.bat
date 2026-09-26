@echo off
cd /d "%~dp0"
echo Starting rAthena Discord Bot...
python bot.py
if %ERRORLEVEL% NEQ 0 (
    echo Bot exited with error %ERRORLEVEL%. Restarting in 10 seconds...
    timeout /t 10 /nobreak
    call "%~f0"
)
