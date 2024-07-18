sc stop input
sc delete input
sc create input binPath= C:\kernel-debugging\input.sys type= kernel
copy C:\kernel-debugging\input.sys C:\Windows\System32\drivers\
sc start input
sc qc input

timeout /t 10
