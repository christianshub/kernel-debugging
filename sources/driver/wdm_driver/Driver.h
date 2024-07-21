#pragma once

#include <ntddk.h>
#include <ntddmou.h>  // Include this for mouse-related definitions

#define IOCTL_SPOTLESS CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUSE_INPUT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x204A, METHOD_BUFFERED, FILE_ANY_ACCESS)

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
NTSTATUS HandleCustomIOCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS MajorFunctions(PDEVICE_OBJECT DeviceObject, PIRP Irp);
VOID MoveMouse(LONG x, LONG y, ULONG buttons, BOOLEAN doubleClick);
VOID InjectMouseInput(LONG x, LONG y, ULONG buttons, BOOLEAN buttonDown);

typedef struct _MY_MOUSE_INPUT_DATA {
    LONG LastX;
    LONG LastY;
    ULONG ButtonFlags;
    BOOLEAN doubleClick;
} MY_MOUSE_INPUT_DATA, * PMY_MOUSE_INPUT_DATA;
