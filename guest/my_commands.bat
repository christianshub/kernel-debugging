@echo off

echo Stopping input service
sc stop input

echo Deleting input service
sc delete input

timeout /t 5

echo Creating input service
sc create input binPath= C:\kernel-debugging\input.sys type= kernel start= demand

echo Copying driver to system directory
copy C:\kernel-debugging\input.sys C:\Windows\System32\drivers\

echo Starting input service
sc start input

echo Querying input service configuration
sc qc input

timeout /t 10
