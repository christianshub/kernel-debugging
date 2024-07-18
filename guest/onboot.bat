@echo off

echo Creating a scheduled task to run my_commands.bat with elevated privileges
schtasks /create /tn RunMyCommands /tr "cmd.exe /c C:\kernel-debugging\guest\my_commands.bat" /sc once /st 00:00 /rl highest /f

echo Running the scheduled task
schtasks /run /tn RunMyCommands

echo Deleting the scheduled task
schtasks /delete /tn RunMyCommands /f
