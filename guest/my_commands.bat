sc stop yourdriver
sc delete yourdriver
sc create yourdriver binPath= C:\kernel-debugging\yourdriver.sys type= kernel
copy C:\kernel-debugging\yourdriver.sys C:\Windows\System32\drivers\
sc start yourdriver
sc qc yourdriver

timeout /t 10
