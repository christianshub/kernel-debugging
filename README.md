# Loading a Driver via Command Prompt + WinDBG

> Typically when you test and debug a driver, the debugger and the driver run on
> separate computers. The computer that runs the debugger is called the host
> computer, and the computer that runs the driver is called the target computer.
> The target computer is also called the test computer.

1. Install a Windows VM onto the test computer
1. Use **VMware** to access the VM (had issues with VirtualBox)
1. Copy this repo into your test PC at `C:\kernel-debugging\`
1. Take a snapshot of the clean VM.
1. On the test PC, enable testsigning, debugging, etc. Add to a .bat script and
   run:

    ```batch
    @echo off

    echo Check if the WinRM service is running, otherwise set it up, needed for vagrant
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

    echo Disable automatic updates
    reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU" /v NoAutoUpdate /t REG_DWORD /d 1 /f

    echo Disable security
    reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows Defender" /v DisableAntiSpyware /t REG_DWORD /d 1 /f
    reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Policies\Microsoft\Windows Defender" /v DisableAntiTamper /t REG_DWORD /d 1 /f

    bcdedit /set nointegritychecks on
    bcdedit /set testsigning on
    bcdedit /debug on
    bcdedit /dbgsettings net hostip:192.168.0.151 port:49152 key:1.1.1.1

    echo All needed registries are changed.
    echo Remember to run Virtual Boxs Guest Tools!

    echo Disable firewall
    reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\SharedAccess\Parameters" /v Start /t REG_DWORD /d 4 /f

    echo Add kernel debug print
    reg add "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter" /v DEFAULT /t REG_DWORD /d 0xf /f

    echo Disable UAC
    reg add "HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Policies\System" /v EnableLUA /d 0 /t REG_DWORD /f /reg:64

    pause
    ```

    > Ensure you run it so everything ran successfully.

    > You will need to change the hostip to your private IPv4 IP-address (run
    > `ipconfig`)

1. Restart VM
1. Take a snapshot of your VMs state, this can serve as a base
1. Attach WinDbg to the test PC (VM).
    * Download the newest
      [WinDbg](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/#install-windbg-directly)
    * Open WinDbg -> File -> Attach to kernel -> Net -> Port: 49152 -> Key:
      1.1.1.1

    > You are here telling WinDbg what VM to connect to by specifying port and
    > key. This maps to the debug settings you sat in the steps prior: `bcdedit
    > /dbgsettings net hostip:192.168.0.151 port:49152 key:1.1.1.1`

    Verify that you are not just waiting to connect by seeing something similar
    to this:

    ```output
    Microsoft (R) Windows Debugger Version 10.0.27553.1004 AMD64
    Copyright (c) Microsoft Corporation. All rights reserved.

    Using NET for debugging
    Opened WinSock 2.0
    Waiting to reconnect...
    Connected to target 192.168.0.151 on port 49152 on local IP 192.168.0.151.
    You can get the target MAC address by running .kdtargetmac command.
    ```

1. On the test PC (VM), run (elevated)

```ps1
Set-ExecutionPolicy RemoteSigned -Scope LocalMachine
notepad $PROFILE.AllUsersAllHosts
```

Insert:

```ps1
function Install-Driver($name)
{
    $cleanName = $name -replace ".sys|.\\", ""

    sc.exe stop $cleanName
    sc.exe delete $cleanName

    cp $name c:\windows\system32\drivers\ -verbose -force
    sc.exe create $cleanName type= kernel start= demand error= normal binPath= c:\windows\System32\Drivers\$cleanName.sys DisplayName= $cleanName

    sc.exe start $cleanName
}
```

1. Use Windows Driver Model (WDM) and not Windows Driver Framework (WDF) Kernel
   Driver when creating driver projects

> WDF wont let you easily unload drivers, but is likely the better option since
> a lot of help is given.

When creating the driver, ensure you are using WDM in scenarios where you want
to unload the driver without restarting.

* Create WDM project
* Delete .inf file
* Create driver.c and insert

```c
#include <ntddk.h>

void DriverUnload(PDRIVER_OBJECT dob)
{
	UNREFERENCED_PARAMETER(dob);
	DbgPrint("Driver unloaded");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = DriverUnload;
	DbgPrint("Driver loaded");

	return STATUS_SUCCESS;
}
```
