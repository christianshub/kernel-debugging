#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

#define IOCTL_MOUSE_MOVE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

void MoveMouse(HANDLE device, LONG x, LONG y) {
    LONG input[2] = { x, y };
    DWORD bytesReturned;
    BOOL result = DeviceIoControl(
        device,
        IOCTL_MOUSE_MOVE,
        input,
        sizeof(input),
        NULL,
        0,
        &bytesReturned,
        NULL
    );
    if (!result) {
        printf("DeviceIoControl failed: %d\n", GetLastError());
    }
}

int main() {
    HANDLE device = CreateFile(
        L"\\\\.\\MouseMover",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (device == INVALID_HANDLE_VALUE) {
        printf("Failed to open device: %d\n", GetLastError());
        return 1;
    }

    MoveMouse(device, 100, 100);

    CloseHandle(device);
    return 0;
}
