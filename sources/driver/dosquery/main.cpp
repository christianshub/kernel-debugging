#include <windows.h>
#include <stdio.h>

int main() {
    TCHAR targetPath[MAX_PATH];

    // Check if the symbolic link exists
    if (QueryDosDevice(L"NewDriverLink", targetPath, MAX_PATH)) {
        wprintf(L"Symbolic link exists: \\\\.\\NewDriverLink -> %s\n", targetPath);
    }
    else {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND) {
            printf("Symbolic link \\\\.\\NewDriverLink does not exist.\n");
        }
        else {
            printf("QueryDosDevice failed with error: %d\n", error);
        }
    }

    Sleep(5000);

    return 0;
}
