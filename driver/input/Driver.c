#include <ntddk.h>
#include <wdf.h>

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD EvtDriverDeviceAdd;
EVT_WDF_DRIVER_UNLOAD UnloadDriver;
EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtDriverContextCleanup;
EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtDeviceContextCleanup;

// Context structure for driver-wide data
typedef struct _DRIVER_CONTEXT {
    WDFDEVICE Device;
} DRIVER_CONTEXT, * PDRIVER_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DRIVER_CONTEXT, DriverGetContext)

// Unload driver callback function
_Use_decl_annotations_
void UnloadDriver(IN WDFDRIVER driver)
{
    UNREFERENCED_PARAMETER(driver);
    DbgPrint("Driver unloaded\n");
}

// Cleanup callback function for driver context
_Use_decl_annotations_
void EvtDriverContextCleanup(WDFOBJECT DriverObject)
{
    PDRIVER_CONTEXT driverContext = DriverGetContext(DriverObject);
    if (driverContext->Device != NULL) {
        DbgPrint("Deleting device in driver context cleanup\n");
        WdfObjectDelete(driverContext->Device);
    }
    DbgPrint("Driver context cleanup\n");
}

// Device context cleanup function
_Use_decl_annotations_
void EvtDeviceContextCleanup(WDFOBJECT DeviceObject)
{
    DbgPrint("Device context cleanup\n");
    PDEVICE_OBJECT pDeviceObject = WdfDeviceWdmGetDeviceObject(DeviceObject);
    IoDeleteDevice(pDeviceObject);
}

// Driver Entry function
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    WDF_DRIVER_CONFIG config;
    WDF_OBJECT_ATTRIBUTES attributes;
    NTSTATUS status;

    DbgPrint("DriverEntry called\n");

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DRIVER_CONTEXT);
    attributes.EvtCleanupCallback = EvtDriverContextCleanup;

    WDF_DRIVER_CONFIG_INIT(&config, EvtDriverDeviceAdd);
    config.EvtDriverUnload = UnloadDriver;

    status = WdfDriverCreate(DriverObject, RegistryPath, &attributes, &config, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        DbgPrint("WdfDriverCreate failed with status 0x%x\n", status);
        return status;
    }

    DbgPrint("Driver loaded\n");

    return status;
}

// EvtDriverDeviceAdd callback function
NTSTATUS EvtDriverDeviceAdd(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit)
{
    UNREFERENCED_PARAMETER(Driver);
    WDFDEVICE device;
    PDRIVER_CONTEXT driverContext;
    NTSTATUS status;
    UNICODE_STRING deviceName;
    UNICODE_STRING symbolicLinkName;

    DbgPrint("EvtDriverDeviceAdd called\n");

    RtlInitUnicodeString(&deviceName, L"\\Device\\MyDevice");
    RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\MyDevice");

    WdfDeviceInitAssignName(DeviceInit, &deviceName);

    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);
    deviceAttributes.EvtCleanupCallback = EvtDeviceContextCleanup;

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
    if (!NT_SUCCESS(status)) {
        DbgPrint("WdfDeviceCreate failed with status 0x%x\n", status);
        return status;
    }

    status = WdfDeviceCreateSymbolicLink(device, &symbolicLinkName);
    if (!NT_SUCCESS(status)) {
        DbgPrint("WdfDeviceCreateSymbolicLink failed with status 0x%x\n", status);
        return status;
    }

    driverContext = DriverGetContext(WdfGetDriver());
    driverContext->Device = device;

    DbgPrint("Device created successfully: %wZ\n", &deviceName);

    return status;
}
