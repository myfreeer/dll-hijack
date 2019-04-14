// linker flags: -lshell32 -lversion -luuid -lole32
#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif // !_UNICODE

#include <windows.h>
#include <stdio.h>

void mainCRTStartup() {
    WCHAR versionDllW[MAX_PATH] = {0};
    GetSystemDirectoryW(versionDllW, MAX_PATH);
    wcscat_s(versionDllW, MAX_PATH, L"\\version.dll");
    LPDWORD lpdwHandle = NULL;
    DWORD size = GetFileVersionInfoSizeW(versionDllW, lpdwHandle);
    printf("GetFileVersionInfoSizeW: %lu\n", size);

    ExitProcess(0);
}