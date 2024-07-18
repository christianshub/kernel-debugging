@echo off

echo Add kernel debug print
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter" /v DEFAULT /t REG_DWORD /d 0xf /f

@REM echo Disable UAC
@REM reg add "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /d 0 /t REG_DWORD /f /reg:64

echo Enable test signing mode
bcdedit /set testsigning on
if %ERRORLEVEL% neq 0 (
  echo Failed to enable test signing mode
  pause
  exit /b %ERRORLEVEL%
)

@REM echo Enable kernel debugging
@REM bcdedit /debug on
@REM if %ERRORLEVEL% neq 0 (
@REM   echo Failed to enable kernel debugging
@REM   pause
@REM   exit /b %ERRORLEVEL%
@REM )

echo All needed changes have been made.
echo Remember to run Virtual Box Guest Tools!

echo.
echo A restart is required for the changes to take effect.
choice /M "Do you want to restart now?"

if %ERRORLEVEL% == 1 (
  shutdown /r /t 0
) else (
  echo Restart aborted. Please restart your computer manually to apply the changes.
)

pause
