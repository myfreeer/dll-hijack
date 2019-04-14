#pragma once
#ifndef __DLL_HIJACK_MAIN_HEADER__
#define __DLL_HIJACK_MAIN_HEADER__

#include <stdbool.h>
#include <stdint.h>

// define this macro to hijack library out of system dir
// and change value of DLL_NAME macro preventing conflicts with this dll
#ifndef DLL_HIJACK_NON_SYSTEM
#define HIJACK_ORIG_DLL TEXT("\\" DLL_NAME ".dll")
#else
#define HIJACK_ORIG_DLL TEXT(DLL_NAME ".dll");
#endif

extern void DLLHijackAttach(bool isSucceed);

// define this macro to use detach callback
#ifdef DLL_HIJACK_DETACH_CALLBACK
// we can probably not call windows api funcs
// since its library can probably be unloaded before this dll
extern void DLLHijackDetach(bool isSucceed);
#endif

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif // !_UNICODE

// Exclude rarely-used stuff from Windows headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "native_api.h"

#ifdef _WIN64
typedef uint64_t MWORD;
#else //_WIN64
typedef uint32_t MWORD;
#endif //_WIN64

#ifdef __cplusplus
#define EXTERNC extern "C"
#else //__cplusplus
#define EXTERNC
#endif //__cplusplus

#define PLACEHOLDER EXTERNC int __cdecl

#ifndef __GNUC__
#define DllMainCRTStartup DllMain
#define UNUSED
#else // __GNUC__
#define UNUSED __attribute__((unused))
#define __nop() asm ("nop")
#endif // __GNUC__

HINSTANCE hInstance = NULL;

#define NOP_FUNC(seq) { \
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    __nop();\
    return (seq);\
}
// return different value,
// preventing visual studio from merging placeholder functions

static inline const wchar_t *wcsrchr_s(
    const wchar_t *s,
    const size_t len,
    const wchar_t c
) {
  for (register const wchar_t *ptr = s + len; ptr >= s; --ptr) {
    if (c == *ptr) {
      return ptr;
    }
  }
  return NULL;
}

/**
 * Retrieves the path of the system directory.
 * @param buffer
 *      A pointer to the buffer to receive the path.
 *      This path does not end with a backslash unless the system directory is the root directory.
 *      For example, if the system directory is named Windows\System32 on drive C,
 *      the path of the system directory retrieved by this function is C:\Windows\System32.
 * @param bufferSize
 *      A pointer to the maximum size of the buffer, in wchar_ts.
 *      If the function succeeds, the return value is the length, in wchar_ts,
 *      of the string copied to the buffer, not including the terminating null character.
 *      If buffer is NULL, the value is the size of the buffer required to hold the path,
 *      in wchar_ts, including the terminating null character.
 * @return {@code STATUS_SUCCESS} for success, anything else for failure
 */
static NTSTATUS LdrGetSystemDirW(_Out_opt_ wchar_t *const buffer,
                                 _Inout_ size_t *const bufferSize) {
  if (bufferSize == NULL) {
    return STATUS_FAIL_CHECK;
  }
  // require loader lock
  void *magic = NULL;
  NTSTATUS st = LdrLockLoaderLock(0, NULL, &magic);
  if (st != STATUS_SUCCESS) {
    return st;
  }

  // get pointer to entry of ntdll
  PLDR_DATA_TABLE_ENTRY pEntry;
  st = LdrFindEntryForAddress(LdrFindEntryForAddress, &pEntry);
  if (st != STATUS_SUCCESS || pEntry == NULL) {
    goto end;
  }

  // get dir of ntdll
  // assuming that dir of ntdll is system dir
  const wchar_t *end = wcsrchr_s(pEntry->FullDllName.Buffer,
                                 pEntry->FullDllName.Length / sizeof(wchar_t), L'\\');
  if (end == NULL) {
    goto end;
  }

  const size_t len = end - pEntry->FullDllName.Buffer;
  if (buffer == NULL) {
    *bufferSize = len + 1;
    goto end;
  }

  // set out size and copy path to buffer
  const size_t targetSize = min(len, *bufferSize - 1);
  if (targetSize < len) {
    st = ERROR_INSUFFICIENT_BUFFER;
  }
  RtlCopyMemory(buffer, pEntry->FullDllName.Buffer, (targetSize) * sizeof(wchar_t));
  buffer[targetSize] = 0;
  *bufferSize = targetSize;

  end:
  LdrUnlockLoaderLock(0, magic);
  return st;
}

// wrapper of native ldr api which performs like LoadLibraryW
static inline void *LdrLoadLibraryW(const wchar_t *const szDllPath) {

  void *hModule = NULL;
  UNICODE_STRING uDllPath;
  RtlInitUnicodeString(&uDllPath, szDllPath);

  NTSTATUS st = LdrGetDllHandle(NULL, NULL, &uDllPath, &hModule);
  if (st == STATUS_SUCCESS && hModule != NULL) {
    return hModule;
  }

  st = LdrLoadDll(NULL, NULL, &uDllPath, &hModule);
  if (st == STATUS_SUCCESS && hModule != NULL) {
    return hModule;
  }
  return NULL;
}

// wrapper of native ldr api which performs like GetProcAddress
static inline void *LdrGetProcAddressA(PVOID hModule, const char *const name) {
  void *procedureAddress = NULL;
  ANSI_STRING procedureName;
  RtlInitAnsiString(&procedureName, name);
  if (LdrGetProcedureAddress(hModule, &procedureName, 0, &procedureAddress) == STATUS_SUCCESS) {
    return procedureAddress;
  }
  return NULL;
}

static bool WriteMemory(void *BaseAddress, const void *Buffer, SIZE_T nSize) {
  DWORD ProtectFlag = 0;
  // note: NtProtectVirtualMemory will align BaseAddress and nSize to 4096
  // note: be sure to keep the original value
  size_t copySize = nSize;
  void *copyAddress = BaseAddress;
  NTSTATUS st = NtProtectVirtualMemory(
      NtCurrentProcess(), &BaseAddress, &nSize, PAGE_EXECUTE_READWRITE, &ProtectFlag);
  if (STATUS_SUCCESS == st) {
    RtlCopyMemory(copyAddress, Buffer, copySize);
    NtFlushInstructionCache(NtCurrentProcess(), BaseAddress, nSize);
    st = NtProtectVirtualMemory(NtCurrentProcess(), &BaseAddress, &nSize, ProtectFlag, &ProtectFlag);
    return st == STATUS_SUCCESS;
  }
  return false;
}

// jmp to original func
static void InstallJMP(PBYTE BaseAddress, MWORD Function) {
#ifdef _WIN64
  BYTE move[] = {0x48, 0xB8};//move rax,xxL);
  BYTE jump[] = {0xFF, 0xE0};//jmp rax

  WriteMemory(BaseAddress, move, sizeof(move));
  BaseAddress += sizeof(move);

  WriteMemory(BaseAddress, (PBYTE) &Function, sizeof(MWORD));
  BaseAddress += sizeof(MWORD);

  WriteMemory(BaseAddress, jump, sizeof(jump));
#else // _WIN64
  BYTE jump[] = {0xE9};
  WriteMemory(BaseAddress, jump, sizeof(jump));
  BaseAddress += sizeof(jump);

  MWORD offset = Function - (MWORD)BaseAddress - 4;
  WriteMemory(BaseAddress, (PBYTE)&offset, sizeof(offset));
#endif // _WIN64
}

// load the original dll
static bool LoadSysDll(HINSTANCE hModule) {
  PBYTE pImageBase = (PBYTE) hModule;
  PIMAGE_DOS_HEADER pimDH = (PIMAGE_DOS_HEADER) pImageBase;
  if (pimDH->e_magic != IMAGE_DOS_SIGNATURE) {
    return false;
  }
  PIMAGE_NT_HEADERS pimNH = (PIMAGE_NT_HEADERS) (pImageBase + pimDH->e_lfanew);
  if (pimNH->Signature != IMAGE_NT_SIGNATURE) {
    return false;
  }
  PIMAGE_EXPORT_DIRECTORY pimExD = (PIMAGE_EXPORT_DIRECTORY)
      (pImageBase + pimNH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
  DWORD *pName = (DWORD *) (pImageBase + pimExD->AddressOfNames);
  DWORD *pFunction = (DWORD *) (pImageBase + pimExD->AddressOfFunctions);
  WORD *pNameOrdinals = (WORD*) (pImageBase + pimExD->AddressOfNameOrdinals);
#ifndef DLL_HIJACK_NON_SYSTEM
  wchar_t szDllPath[MAX_PATH + 1];
  size_t bufferSize = MAX_PATH;
  if (STATUS_SUCCESS != LdrGetSystemDirW(szDllPath, &bufferSize)) {
    return false;
  }
  RtlCopyMemory(szDllPath + bufferSize, HIJACK_ORIG_DLL, sizeof(HIJACK_ORIG_DLL));
#else
  wchar_t *szDllPath = TEXT(DLL_NAME ".dll");
#endif
  HINSTANCE hDllModule = LdrLoadLibraryW(szDllPath);
  hInstance = hDllModule;
  for (size_t i = 0; i < pimExD->NumberOfNames; i++) {
    MWORD Original = (MWORD) LdrGetProcAddressA(hDllModule, (char *) (pImageBase + pName[i]));
    if (Original) {
      InstallJMP(pImageBase + pFunction[pNameOrdinals[i]], Original);
    }
  }
  return true;
}

EXTERNC BOOL WINAPI DllMainCRTStartup(HINSTANCE hModule, DWORD dwReason, LPVOID UNUSED pv) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    // keeping function of original dll
    DLLHijackAttach(LoadSysDll(hModule));
  } else if (dwReason == DLL_PROCESS_DETACH) {
#ifdef DLL_HIJACK_DETACH_CALLBACK
    if (hInstance != NULL) {
      DLLHijackDetach(STATUS_SUCCESS == LdrUnloadDll(hInstance));
    } else {
      DLLHijackDetach(false);
    }
#else
    if (hInstance != NULL) {
      LdrUnloadDll(hInstance);
    }
#endif
  }
  return TRUE;
}

#endif // __DLL_HIJACK_MAIN_HEADER__
