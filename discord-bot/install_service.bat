@echo off
:: รัน script นี้ใน Command Prompt แบบ "Run as Administrator"
cd /d "%~dp0"

echo ============================================
echo  rAthena Discord Bot - Windows Service Setup
echo ============================================
echo.

:: ตรวจสอบว่ามี NSSM ไหม
where nssm >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] ไม่พบ NSSM — กรุณาดาวน์โหลดจาก https://nssm.cc/download
    echo     แล้วแตก nssm.exe ไว้ใน folder เดียวกับ script นี้ หรือ PATH
    pause
    exit /b 1
)

:: หา python path
for /f "delims=" %%i in ('where python') do set PYTHON_EXE=%%i
if "%PYTHON_EXE%"=="" (
    echo [!] ไม่พบ Python — กรุณาติดตั้ง Python 3.11+ และตั้งค่า PATH
    pause
    exit /b 1
)

set SERVICE_NAME=RathenaDiscordBot
set BOT_DIR=%~dp0
set BOT_SCRIPT=%BOT_DIR%bot.py

echo [*] Python: %PYTHON_EXE%
echo [*] Bot Dir: %BOT_DIR%
echo [*] Service: %SERVICE_NAME%
echo.

:: ลบ service เก่า (ถ้ามี)
nssm stop %SERVICE_NAME% 2>nul
nssm remove %SERVICE_NAME% confirm 2>nul

:: ติดตั้ง service ใหม่
nssm install %SERVICE_NAME% "%PYTHON_EXE%" "%BOT_SCRIPT%"
nssm set %SERVICE_NAME% AppDirectory "%BOT_DIR%"
nssm set %SERVICE_NAME% DisplayName "rAthena Discord Bot"
nssm set %SERVICE_NAME% Description "Discord Bot for rAthena Ragnarok Online Server"
nssm set %SERVICE_NAME% Start SERVICE_AUTO_START
nssm set %SERVICE_NAME% AppStdout "%BOT_DIR%logs\bot.log"
nssm set %SERVICE_NAME% AppStderr "%BOT_DIR%logs\bot_error.log"
nssm set %SERVICE_NAME% AppRotateFiles 1
nssm set %SERVICE_NAME% AppRotateOnline 1
nssm set %SERVICE_NAME% AppRotateBytes 10485760

:: สร้าง logs folder
if not exist "%BOT_DIR%logs" mkdir "%BOT_DIR%logs"

:: เริ่ม service
nssm start %SERVICE_NAME%

echo.
echo [OK] Service "%SERVICE_NAME%" ติดตั้งและเริ่มทำงานแล้ว
echo      จัดการได้ที่ Services (services.msc) หรือใช้คำสั่ง:
echo      nssm start/stop/restart %SERVICE_NAME%
echo.
pause
