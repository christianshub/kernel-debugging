#include "Driver.h"

UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\SpotlessDevice");
UNICODE_STRING DEVICE_SYMBOLIC_NAME = RTL_CONSTANT_STRING(L"\\??\\SpotlessDeviceLink");

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
    DbgPrint("Driver unloaded, deleting symbolic links and devices\n");
    IoDeleteDevice(DriverObject->DeviceObject);
    IoDeleteSymbolicLink(&DEVICE_SYMBOLIC_NAME);
}

NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(Irp);
    CHAR* messageFromKernel = "ohai from them kernelz\n";
    NTSTATUS status = STATUS_SUCCESS;

    if (stackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_SPOTLESS)
    {
        DbgPrint("IOCTL_SPOTLESS (0x%x) issued\n", stackLocation->Parameters.DeviceIoControl.IoControlCode);
        DbgPrint("Input received from userland: %s\n", (char*)Irp->AssociatedIrp.SystemBuffer);

        size_t messageLength = strlen(messageFromKernel) + 1; // Include the null terminator
        Irp->IoStatus.Information = messageLength;
        RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, messageFromKernel, messageLength);
    }
    else if (stackLocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_MOUSE_INPUT)
    {
        if (stackLocation->Parameters.DeviceIoControl.InputBufferLength < sizeof(MY_MOUSE_INPUT_DATA))
        {
            status = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            PMY_MOUSE_INPUT_DATA mouseData = (PMY_MOUSE_INPUT_DATA)Irp->AssociatedIrp.SystemBuffer;
            DbgPrint("Mouse input received: x=%d, y=%d, buttons=%lu, doubleClick=%d\n", mouseData->LastX, mouseData->LastY, mouseData->ButtonFlags, mouseData->doubleClick);

            // Simulate mouse input
            MoveMouse(mouseData->LastX, mouseData->LastY, mouseData->ButtonFlags, mouseData->doubleClick);
        }
    }
    else
    {
        status = STATUS_INVALID_DEVICE_REQUEST;
        DbgPrint("Invalid device request\n");
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;
}

VOID MoveMouse(LONG x, LONG y, ULONG buttons, BOOLEAN doubleClick)
{
    DbgPrint("MoveMouse function entered with x=%d, y=%d, buttons=%lu, doubleClick=%d\n", x, y, buttons, doubleClick);

    // Simulate single or double click
    if (doubleClick)
    {
        // First click
        InjectMouseInput(x, y, buttons, TRUE);
        InjectMouseInput(x, y, buttons, FALSE);

        // Second click
        InjectMouseInput(x, y, buttons, TRUE);
        InjectMouseInput(x, y, buttons, FALSE);
    }
    else
    {
        // Single click
        InjectMouseInput(x, y, buttons, TRUE);
        InjectMouseInput(x, y, buttons, FALSE);
    }
}

VOID InjectMouseInput(LONG x, LONG y, ULONG buttons, BOOLEAN buttonDown)
{
    MOUSE_INPUT_DATA inputData = { 0 };
    inputData.LastX = x;
    inputData.LastY = y;

    if (buttonDown)
    {
        if (buttons & MOUSE_LEFT_BUTTON_DOWN) inputData.ButtonFlags |= MOUSE_LEFT_BUTTON_DOWN;
        if (buttons & MOUSE_RIGHT_BUTTON_DOWN) inputData.ButtonFlags |= MOUSE_RIGHT_BUTTON_DOWN;
    }
    else
    {
        if (buttons & MOUSE_LEFT_BUTTON_UP) inputData.ButtonFlags |= MOUSE_LEFT_BUTTON_UP;
        if (buttons & MOUSE_RIGHT_BUTTON_UP) inputData.ButtonFlags |= MOUSE_RIGHT_BUTTON_UP;
    }

    // Get the device object for the mouse class driver
    UNICODE_STRING mouseDeviceName = RTL_CONSTANT_STRING(L"\\Device\\PointerClass0");
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status = IoGetDeviceObjectPointer(&mouseDeviceName, FILE_READ_DATA, &fileObject, &deviceObject);
    if (!NT_SUCCESS(status))
    {
        DbgPrint("Failed to get device object pointer for mouse class driver: 0x%x\n", status);
        return;
    }

    KEVENT event;
    IO_STATUS_BLOCK ioStatusBlock;
    PIRP irp;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUSE_INSERT_DATA, deviceObject, &inputData, sizeof(inputData), NULL, 0, TRUE, &event, &ioStatusBlock);
    if (irp == NULL)
    {
        DbgPrint("Failed to build IRP\n");
        return;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING)
    {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatusBlock.Status;
    }

    if (!NT_SUCCESS(status))
    {
        DbgPrint("IoCallDriver failed: 0x%x\n", status);
    }

    ObDereferenceObject(fileObject);
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

    // Routine that will execute when our driver is unloaded/service is stopped
    DriverObject->DriverUnload = DriverUnload;

    // Routine for handling IO requests from userland
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleCustomIOCTL;

    // Routines that will execute once a handle to our device's symbolic link is opened/closed
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
