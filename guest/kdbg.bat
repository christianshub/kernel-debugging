bcdedit /debug on
bcdedit /dbgsettings net hostip:192.168.56.1 port:49152 key:1.1.1.1
copy C:\kernel-debugging\guest\onboot.bat C:\onboot.bat
copy C:\kernel-debugging\guest\my_commands.bat C:\my_commands.bat
schtasks /create /sc onlogon /tr "C:\onboot.bat" /tn vagrantonboot /RU vagrant /f
shutdown /r /t 0
