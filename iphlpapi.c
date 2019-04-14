#include "iphlpapi.h"

void DLLHijackAttach(bool isSucceed) {
  // definition for pointer to MessageBoxW, drop this when unnecessary
  typedef int (WINAPI *pfMsgBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
  // Use of native api is recommended, but not enforced,
  // you can still use windows api if early load is not required.
  static pfMsgBoxW pMsgBoxW = NULL;
  if (pMsgBoxW == NULL) {
    void *user32 = LdrLoadLibraryW(L"user32.dll");
    LdrGetProcAddressA(user32, "MessageBoxW");
  }

  if (isSucceed) {
    pMsgBoxW(NULL, TEXT("DLL Hijack Attach Succeed!"), TEXT(DLL_NAME " DLL Hijack Attach"), 0);
  }
}

