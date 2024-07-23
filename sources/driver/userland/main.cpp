#include <Windows.h>
#include <stdio.h>

#define MOUSE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define KEYBOARD_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x777, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define KEY_MAKE 0x00  // Key down
#define KEY_BREAK 0x01 // Key up

typedef struct _KMOUSE_REQUEST {
    long x;
    long y;
    unsigned short button_flags;
} KMOUSE_REQUEST, * PKMOUSE_REQUEST;

typedef struct _KKEYBOARD_REQUEST {
    unsigned short make_code;
    unsigned short flags; // Add flags to specify key down or key up
} KKEYBOARD_REQUEST, * PKKEYBOARD_REQUEST;

int main() {
    HANDLE hDevice = CreateFileW(L"\\\\.\\infestation", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device: %d\n", GetLastError());
        return 1;
    }

    // Example for sending mouse input
    KMOUSE_REQUEST mouse_request;
    mouse_request.x = 100;
    mouse_request.y = 100;
    mouse_request.button_flags = MOUSE_MOVE_ABSOLUTE;  // Absolute movement

    DWORD bytesReturned;
    if (!DeviceIoControl(hDevice, MOUSE_REQUEST, &mouse_request, sizeof(mouse_request), NULL, 0, &bytesReturned, NULL)) {
        printf("DeviceIoControl failed: %d\n", GetLastError());
        CloseHandle(hDevice);
        return 1;
    }

    Sleep(5000);
    printf("Wait 5 seconds\n");

    // Example for sending keyboard input
    KKEYBOARD_REQUEST keyboard_request;
    keyboard_request.make_code = 0x1E;  // 'A' key
    keyboard_request.flags = KEY_MAKE;  // Key down

    printf("Sending key down for 'A'\n");
    if (!DeviceIoControl(hDevice, KEYBOARD_REQUEST, &keyboard_request, sizeof(keyboard_request), NULL, 0, &bytesReturned, NULL)) {
        printf("DeviceIoControl failed: %d\n", GetLastError());
        CloseHandle(hDevice);
        return 1;
    }

    Sleep(1000);

    // Key up event
    keyboard_request.flags = KEY_BREAK;  // Key up

    printf("Sending key up for 'A'\n");
    if (!DeviceIoControl(hDevice, KEYBOARD_REQUEST, &keyboard_request, sizeof(keyboard_request), NULL, 0, &bytesReturned, NULL)) {
        printf("DeviceIoControl failed: %d\n", GetLastError());
        CloseHandle(hDevice);
        return 1;
    }

    printf("Input sent successfully.\n");
    CloseHandle(hDevice);
    return 0;
}
