#include "Driver.h"

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG config;
    NTSTATUS status;

    KdPrint(("DriverEntry called\n"));

    WDF_DRIVER_CONFIG_INIT(&config, MouseMoverEvtDeviceAdd);

    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("WdfDriverCreate failed: %!STATUS!\n", status));
    }

    return status;
}

NTSTATUS MouseMoverEvtDeviceAdd(
    _In_ WDFDRIVER Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDFDEVICE device;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;

    KdPrint(("MouseMoverEvtDeviceAdd called\n"));

    WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);
    deviceAttributes.SynchronizationScope = WdfSynchronizationScopeDevice;

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        KdPrint(("WdfDeviceCreate failed: %!STATUS!\n", status));
        return status;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);
    queueConfig.EvtIoDeviceControl = MouseMoverEvtIoDeviceControl;

    status = WdfIoQueueCreate(device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("WdfIoQueueCreate failed: %!STATUS!\n", status));
        return status;
    }

    // Create a symbolic link
    UNICODE_STRING symLinkName = RTL_CONSTANT_STRING(L"\\??\\NewDriverLink");
    status = WdfDeviceCreateSymbolicLink(device, &symLinkName);
    if (!NT_SUCCESS(status)) {
        KdPrint(("WdfDeviceCreateSymbolicLink failed: %!STATUS!\n", status));
        WdfObjectDelete(device);
        return status;
    }

    KdPrint(("Symbolic link created successfully: %wZ\n", &symLinkName));
    return status;
}

VOID MouseMoverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);

    NTSTATUS status = STATUS_SUCCESS;
    PVOID buffer;
    size_t bufSize;

    KdPrint(("MouseMoverEvtIoDeviceControl called with IoControlCode: %d\n", IoControlCode));

    switch (IoControlCode) {
    case IOCTL_MOUSE_MOVE:
        if (InputBufferLength < sizeof(LONG) * 2) {
            status = STATUS_BUFFER_TOO_SMALL;
            KdPrint(("Input buffer too small\n"));
            break;
        }

        status = WdfRequestRetrieveInputBuffer(Request, sizeof(LONG) * 2, &buffer, &bufSize);
        if (NT_SUCCESS(status)) {
            LONG* input = (LONG*)buffer;
            KdPrint(("MoveMouse called with x=%d, y=%d\n", input[0], input[1]));
            MoveMouse(input[0], input[1]);
        }
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        KdPrint(("Invalid device request\n"));
        break;
    }

    WdfRequestComplete(Request, status);
}

VOID MoveMouse(LONG x, LONG y)
{
    KdPrint(("MoveMouse function entered with x=%d, y=%d\n", x, y));

    // Use mouse movement simulation code
    // This approach uses the registry-based method for mouse movement

    UNICODE_STRING mouseRegistryPath;
    RtlInitUnicodeString(&mouseRegistryPath, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Mouse");

    HANDLE hKey;
    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, &mouseRegistryPath, OBJ_CASE_INSENSITIVE, NULL, NULL);

    NTSTATUS status = ZwOpenKey(&hKey, KEY_SET_VALUE, &objAttr);
    if (NT_SUCCESS(status)) {
        LONG mouseData[2] = { x, y };

        UNICODE_STRING valueName;
        RtlInitUnicodeString(&valueName, L"DefaultMouseSpeed");

        status = ZwSetValueKey(hKey, &valueName, 0, REG_BINARY, mouseData, sizeof(mouseData));
        if (NT_SUCCESS(status)) {
            KdPrint(("ZwSetValueKey succeeded\n"));
        }
        else {
            KdPrint(("ZwSetValueKey failed: %!STATUS!\n", status));
        }
        ZwClose(hKey);
    }
    else {
        KdPrint(("ZwOpenKey failed: %!STATUS!\n", status));
    }
}
