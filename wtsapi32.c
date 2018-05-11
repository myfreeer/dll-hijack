#include "wtsapi32.h"
#include <winuser.h>

void DLLHijackAttach(bool isSucceed) {
    if (isSucceed)
        MessageBox(NULL, TEXT("DLL Hijack Attach Succeed!"), TEXT(DLL_NAME " DLL Hijack Attach"), MB_OK);
}

void DLLHijackDetach(bool isSucceed) {
    if (isSucceed)
        MessageBox(NULL, TEXT("DLL Hijack Detach Succeed!"), TEXT(DLL_NAME " DLL Hijack Detach"), MB_OK);
}
