@echo off

echo Add kernel debug print
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter" /v DEFAULT /t REG_DWORD /d 0xf /f

echo Disable UAC
reg add "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /d 0 /t REG_DWORD /f /reg:64

echo Disable firewall
reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\SharedAccess\Parameters" /v Start /t REG_DWORD /d 4 /f

echo Set network to Private
powershell.exe -command "Set-NetConnectionProfile -InterfaceAlias 'Ethernet' -NetworkCategory Private"

echo Check if the WinRM service is running, otherwise set it up
sc query WinRM | find "STATE" | find "RUNNING" >nul
if %ERRORLEVEL% == 0 (
  echo WinRM service is already running
) else (
  echo Starting WinRM service
  winrm quickconfig -q
  winrm set winrm/config/winrs @{MaxMemoryPerShellMB="512"}
  winrm set winrm/config @{MaxTimeoutms="1800000"}
  winrm set winrm/config/service @{AllowUnencrypted="true"}
  winrm set winrm/config/service/auth @{Basic="true"}
  sc config WinRM start=auto
)

echo Disable windows auto login
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v AutoAdminLogon /t REG_SZ /d 1 /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultUserName /t REG_SZ /d vagrant /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v DefaultPassword /t REG_SZ /d vagrant /f

echo Disable automatic updates
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU" /v NoAutoUpdate /t REG_DWORD /d 1 /f

echo Disable security
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows Defender" /v DisableAntiSpyware /t REG_DWORD /d 1 /f
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows Defender" /v DisableAntiTamper /t REG_DWORD /d 1 /f

echo Enable test signing mode
bcdedit /set testsigning on

echo Enable kernel debugging
bcdedit /debug on

echo All needed registries are changed.
echo Remember to run Virtual Boxs Guest Tools!

echo.
echo A restart is required for the changes to take effect.
choice /M "Do you want to restart now?"

if %ERRORLEVEL% == 1 (
  shutdown /r /t 0
) else (
  echo Restart aborted. Please restart your computer manually to apply the changes.
)

pause
