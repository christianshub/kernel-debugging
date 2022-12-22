start vagrant up & vagrant powershell --command "schtasks /run /tn vagrantonboot"
"C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\windbg.exe" -k "net:port=49152,key=1.1.1.1" -y "srv*c:\kernel-debugging\symbols*https://msdl.microsoft.com/download/symbols"
vagrant halt -f
