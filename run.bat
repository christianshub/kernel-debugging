@echo off

echo Stopping the existing driver if it's running...
sc stop yourdriver

echo Deleting the existing driver service...
sc delete yourdriver

echo Creating the new driver service...
sc create yourdriver binPath= "C:\Windows\System32\drivers\input.sys" type= kernel start= demand error= normal DisplayName= $cleanName

echo Copying the driver file to the System32\drivers directory...
copy C:\kernel-debugging\input.sys C:\Windows\System32\drivers\

echo Starting the new driver...
sc start yourdriver

echo Querying the driver configuration...
sc qc yourdriver

pause
