#include <Windows.h>
#include <stdio.h>

#define MOUSE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _KMOUSE_REQUEST {
	long x;
	long y;
	unsigned short button_flags;
} KMOUSE_REQUEST, * PKMOUSE_REQUEST;

int main() {
	HANDLE hDevice = CreateFileW(L"\\\\.\\infestation", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Failed to open device: %d\n", GetLastError());
		return 1;
	}

	KMOUSE_REQUEST mouse_request;
	mouse_request.x = 100;
	mouse_request.y = 100;
	mouse_request.button_flags = 0;  // No movement, only clicking

	DWORD bytesReturned;
	if (!DeviceIoControl(hDevice, MOUSE_REQUEST, &mouse_request, sizeof(mouse_request), NULL, 0, &bytesReturned, NULL)) {
		printf("DeviceIoControl failed: %d\n", GetLastError());
		CloseHandle(hDevice);
		return 1;
	}

	printf("Double click at (100, 100) performed successfully.\n");
	CloseHandle(hDevice);
	return 0;
}
