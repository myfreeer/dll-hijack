#!/bin/bash
set -e

if [[ "$1" = "" ]]; then
    echo Fatal: must specify a dll.
    exit
elif [[ "$1" = "--help" ]] || [[ "$1" = "-h" ]]; then
    echo Usage: ./gen_code.sh dll_name.dll
    echo dll_name.dll should be in "${WINDIR}/System32/"
    echo Requirements: gendef
    exit
fi
dll_path="$(which "$1")"
dll_file="$(basename "${dll_path}")"
dll_name="$(echo "${dll_file}" | cut -d'.' -f1)"
if [[ "${dll_name}" = "" ]]; then
    echo Fatal: dll "$1" not found. Try specifying full dll name with extension.
    exit
fi

gen_header() {
    local count=1
    local dll_exports=( $(gendef -f - "${dll_path}" |
        sed -r -e '/^[;]/d' -e 's/^([^;].+)( .+)$/\1/g' -e '/^(LIBRARY|EXPORTS)/d') )
    echo "#define DLL_NAME \"${dll_name}\""
    echo "#include \"hijack.h\""
    echo "LIBRARY \"${dll_name}\"" > "${dll_name}.def"
    echo "EXPORTS" >> "${dll_name}.def"
    for i in ${dll_exports[@]}; do
        local func_name="__place_holder_$((count++))"
        echo "PLACEHOLDER ${func_name}() NOP_FUNC(__LINE__);"
        echo "${i} = ${func_name}" >> "${dll_name}.def"
    done
}

gen_header > "./${dll_name}.h"
echo "Generated ${dll_name}.h"

cat <<EOF > "./${dll_name}.c"
#include "${dll_name}.h"

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

EOF

echo "Generated ${dll_name}.c"

sed -i "/add_library(${dll_name} SHARED/d" CMakeLists.txt
echo "add_library(${dll_name} SHARED ${dll_name}.c ${dll_name}.h ${dll_name}.def \${SRC})" >> CMakeLists.txt
echo Added "${dll_name}" entry to CMakeLists.txt