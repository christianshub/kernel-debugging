@REM start vagrant up
start "" "windbgx" -k "net:port=49152,key=1.1.1.1" -y "srv*c:\kernel-debugging\symbols*https://msdl.microsoft.com/download/symbols"
@REM vagrant halt -f
