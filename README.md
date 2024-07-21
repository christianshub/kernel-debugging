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

1. Now use IOCTL to communicate between kernel and userspace:

**Userland**

main.cpp:

```c
#include <iostream>
#include <Windows.h>

#define IOCTL_SPOTLESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <message>" << std::endl;
        return 1;
    }

    HANDLE device = INVALID_HANDLE_VALUE;
    BOOL status = FALSE;
    DWORD bytesReturned = 0;
    CHAR inBuffer[128] = { 0 };
    CHAR outBuffer[128] = { 0 };

    strncpy_s(inBuffer, argv[1], sizeof(inBuffer) - 1);

    device = CreateFileW(L"\\\\.\\SpotlessDeviceLink", GENERIC_WRITE | GENERIC_READ | GENERIC_EXECUTE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);

    if (device == INVALID_HANDLE_VALUE)
    {
        std::cerr << "> Could not open device: 0x" << std::hex << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "> Issuing IOCTL_SPOTLESS 0x" << std::hex << IOCTL_SPOTLESS << std::endl;
    status = DeviceIoControl(device, IOCTL_SPOTLESS, inBuffer, sizeof(inBuffer), outBuffer, sizeof(outBuffer), &bytesReturned, NULL);
    if (!status)
    {
        std::cerr << "> DeviceIoControl failed: 0x" << std::hex << GetLastError() << std::endl;
        CloseHandle(device);
        return 1;
    }
    std::cout << "> IOCTL_SPOTLESS 0x" << std::hex << IOCTL_SPOTLESS << " issued" << std::endl;
    std::cout << "> Received from the kernel land: " << outBuffer << ". Received buffer size: " << bytesReturned << std::endl;

    CloseHandle(device);

    Sleep(5000);
    return 0;
}
```

**Kernel**

driver.c:

```c
#include <wdm.h>

DRIVER_DISPATCH HandleCustomIOCTL;
#define IOCTL_SPOTLESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\SpotlessDevice");
UNICODE_STRING DEVICE_SYMBOLIC_NAME = RTL_CONSTANT_STRING(L"\\??\\SpotlessDeviceLink");

void DriverUnload(PDRIVER_OBJECT dob)
{
    DbgPrint("Driver unloaded, deleting symbolic links and devices\n");
    IoDeleteDevice(dob->DeviceObject);
    IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_NAME);
}

NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stackLocation = NULL;
    CHAR* messageFromKernel = "ohai from them kernelz\n";

    stackLocation = IoGetCurrentIrpStackLocation(Irp);

    if (stackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_SPOTLESS)
    {
        DbgPrint("IOCTL_SPOTLESS (0x%x) issued\n", stackLocation->Parameters.DeviceIoControl.IoControlCode);
        DbgPrint("Input received from userland: %s\n", (char*)Irp->AssociatedIrp.SystemBuffer);
    }

    size_t messageLength = strlen(messageFromKernel) + 1; // Include the null terminator
    Irp->IoStatus.Information = messageLength;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    DbgPrint("Sending to userland: %s\n", messageFromKernel);
    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, messageFromKernel, messageLength);

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS MajorFunctions(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(Irp);

    switch (stackLocation->MajorFunction)
    {
    case IRP_MJ_CREATE:
        DbgPrint("Handle to symbolic link %wZ opened\n", &DEVICE_SYMBOLIC_NAME);
        break;
    case IRP_MJ_CLOSE:
        DbgPrint("Handle to symbolic link %wZ closed\n", &DEVICE_SYMBOLIC_NAME);
        break;
    default:
        break;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS status;

    // routine that will execute when our driver is unloaded/service is stopped
    DriverObject->DriverUnload = DriverUnload;

    // routine for handling IO requests from userland
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleCustomIOCTL;

    // routines that will execute once a handle to our device's symbolic link is opened/closed
    DriverObject->MajorFunction[IRP_MJ_CREATE] = MajorFunctions;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MajorFunctions;

    DbgPrint("Driver loaded\n");

    status = IoCreateDevice(DriverObject, 0, &DEVICE_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DriverObject->DeviceObject);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Could not create device %wZ\n", &DEVICE_NAME);
        return status;
    }
    else
    {
        DbgPrint("Device %wZ created\n", &DEVICE_NAME);
    }

    status = IoCreateSymbolicLink(&DEVICE_SYMBOLIC_NAME, &DEVICE_NAME);
    if (NT_SUCCESS(status))
    {
        DbgPrint("Symbolic link %wZ created\n", &DEVICE_SYMBOLIC_NAME);
    }
    else
    {
        DbgPrint("Error creating symbolic link %wZ\n", &DEVICE_SYMBOLIC_NAME);
        IoDeleteDevice(DriverObject->DeviceObject);
    }

    return status;
}
```

1. Use [DebugView](https://learn.microsoft.com/en-us/sysinternals/downloads/debugview) to pretty print debug messages
  * Use x64-version on the VM host (local), no admin privlieges needed

1. Use [WinObj](https://learn.microsoft.com/en-us/sysinternals/downloads/winobj) to ensure the symbolic link is correctly created.
  * Use x64-version on the VM host (local)
  * Open program -> Device -> Search for your device.





asad
