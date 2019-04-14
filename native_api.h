//
// Created by Administrator on 2019/4/13.
//

#ifndef DLL_HIJACK_NATIVE_API_H
#define DLL_HIJACK_NATIVE_API_H

#include <winternl.h>

#ifdef __cplusplus
extern "C" {
#endif

NTSYSAPI
ULONG
DbgPrintEx(
    ULONG ComponentId,
    ULONG Level,
    PCSTR Format,
    ...
);

#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)

NTSYSCALLAPI
NTSTATUS
NTAPI
NtTerminateProcess(
    _In_opt_ HANDLE ProcessHandle,
    _In_ NTSTATUS ExitStatus
);

#define NtExitProcess(status) NtTerminateProcess(NtCurrentProcess(), (NTSTATUS)(status))

NTSYSCALLAPI
NTSTATUS
NTAPI
NtProtectVirtualMemory(
    _In_ HANDLE ProcessHandle,
    _Inout_ PVOID *BaseAddress,
    _Inout_ PSIZE_T RegionSize,
    _In_ ULONG NewProtect,
    _Out_ PULONG OldProtect
);


NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteVirtualMemory(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress,
    _In_reads_bytes_(BufferSize) PVOID Buffer,
    _In_ SIZE_T BufferSize,
    _Out_opt_ PSIZE_T NumberOfBytesWritten
);


NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushInstructionCache(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress,
    _In_ SIZE_T Length
);

NTSYSAPI
NTSTATUS
NTAPI
LdrFindEntryForAddress(
    _In_ PVOID DllHandle,
    _Out_ PLDR_DATA_TABLE_ENTRY *Entry
);

NTSYSAPI
NTSTATUS
NTAPI
LdrGetDllHandle(
    _In_opt_ PWSTR DllPath,
    _In_opt_ PULONG DllCharacteristics,
    _In_ PUNICODE_STRING DllName,
    _Out_ PVOID *DllHandle
);

NTSYSAPI
NTSTATUS
NTAPI
LdrLoadDll(
    _In_opt_  PWSTR SearchPath,
    _In_opt_  PULONG LoadFlags,
    _In_    PUNICODE_STRING Name,
    _Out_opt_  PVOID *BaseAddress
);

NTSYSAPI
NTSTATUS
NTAPI
LdrUnloadDll(
    _In_ PVOID DllHandle
);

NTSYSAPI
NTSTATUS
NTAPI
LdrGetProcedureAddress(
    _In_  PVOID BaseAddress,
    _In_  PANSI_STRING Name,
    _In_  ULONG Ordinal,
    _Out_  PVOID *ProcedureAddress
);

NTSYSAPI
NTSTATUS
NTAPI
LdrLockLoaderLock(
    _In_ ULONG Flags,
    _Out_opt_ ULONG *Disposition,
    _Out_ PVOID *Cookie
);
NTSYSAPI
NTSTATUS
NTAPI
LdrUnlockLoaderLock(
    _In_ ULONG Flags,
    _Inout_ PVOID Cookie
);


#ifndef __cplusplus
#define RTL_CONSTANT_STRING(s) { sizeof(s) - sizeof((s)[0]), sizeof(s), s }
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#endif
#ifndef STATUS_FAIL_CHECK
#define STATUS_FAIL_CHECK                ((NTSTATUS)0xC0000229L)
#endif

#ifndef ERROR_INSUFFICIENT_BUFFER
#define ERROR_INSUFFICIENT_BUFFER 0x7A
#endif

#ifdef __cplusplus
}
#endif

#endif //DLL_HIJACK_NATIVE_API_H
