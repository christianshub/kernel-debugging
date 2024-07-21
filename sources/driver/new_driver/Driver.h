#pragma once

#include <ntddk.h>
#include <wdf.h>

#define IOCTL_MOUSE_MOVE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD MouseMoverEvtDeviceAdd;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL MouseMoverEvtIoDeviceControl;

VOID MoveMouse(LONG x, LONG y);
