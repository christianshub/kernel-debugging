#include <ntddk.h>
#include <ntddmou.h>

#define IOCTL_MOUSE_MOVE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

void DriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void MoveMouse(LONG x, LONG y);

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("DriverEntry called\n");

    DriverObject->DriverUnload = DriverUnload;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoctl;

    UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\MouseMover");
    UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\MouseMover");
    PDEVICE_OBJECT deviceObject = NULL;

    NTSTATUS status = IoCreateDevice(
        DriverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject
    );

    if (NT_SUCCESS(status)) {
        status = IoCreateSymbolicLink(&symLinkName, &deviceName);
    }

    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
    }

    DbgPrint("Driver loaded\n");
    return status;
}

void DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\MouseMover");
    IoDeleteSymbolicLink(&symLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);
    DbgPrint("Driver unloaded\n");
}

NTSTATUS DispatchCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    DbgPrint("DispatchCreateClose called\n");

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    DbgPrint("DispatchIoctl called\n");

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG_PTR information = 0;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_MOUSE_MOVE: {
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(LONG) * 2) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        LONG* inputBuffer = (LONG*)Irp->AssociatedIrp.SystemBuffer;
        DbgPrint("MoveMouse called with x=%d, y=%d\n", inputBuffer[0], inputBuffer[1]);
        MoveMouse(inputBuffer[0], inputBuffer[1]);
        information = sizeof(LONG) * 2;
        break;
    }
    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = information;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DbgPrint("DispatchIoctl completed\n");

    return status;
}

void MoveMouse(LONG x, LONG y) {
    DbgPrint("MoveMouse function entered\n");

    MOUSE_INPUT_DATA mid = { 0 };
    mid.LastX = x;
    mid.LastY = y;
    mid.Flags = MOUSE_MOVE_ABSOLUTE;

    UNICODE_STRING mouseDeviceName = RTL_CONSTANT_STRING(L"\\Device\\PointerClass0");
    PFILE_OBJECT mouseFileObject;
    PDEVICE_OBJECT mouseDeviceObject;

    NTSTATUS status = IoGetDeviceObjectPointer(&mouseDeviceName, FILE_READ_DATA, &mouseFileObject, &mouseDeviceObject);
    if (NT_SUCCESS(status)) {
        PVOID mouseServiceCallback = mouseDeviceObject->DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL];
        if (mouseServiceCallback) {
            DbgPrint("Calling mouseServiceCallback\n");
            (*(void(*)(PVOID, PVOID))mouseServiceCallback)(mouseDeviceObject->DeviceExtension, &mid);
        }
        ObDereferenceObject(mouseFileObject);
    }

    DbgPrint("MoveMouse function completed\n");
}
