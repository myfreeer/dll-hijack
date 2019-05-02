cmake_minimum_required(VERSION 3.1)
project(dll-hijack C)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif (NOT CMAKE_BUILD_TYPE)

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT /wd4996 /wd4068")
    set(CompilerFlags
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE)
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()
    link_libraries(ntdll)
endif ()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "-pipe -Wall -Wextra -nostartfiles")
    set(CMAKE_C_FLAGS_DEBUG "-g")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -s")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_C_FLAGS}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/test_mingw_ntdll_crt.c"
            "size_t __cdecl strlen(const char *_Str);int main(void) {return strlen(\"str\");}")
    try_compile(HAS_NTDLLCRT ${CMAKE_CURRENT_BINARY_DIR}
            "${CMAKE_CURRENT_BINARY_DIR}/test_mingw_ntdll_crt.c"
            LINK_LIBRARIES ntdllcrt)
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/test_mingw_ntdll_crt.c")
    if(HAS_NTDLLCRT)
        link_libraries(ntdll ntdllcrt)
    else()
        link_libraries(ntdll)
    endif()
endif ()

set (CMAKE_SHARED_LIBRARY_PREFIX "")
set (CMAKE_SHARED_MODULE_PREFIX "")
set (SRC hijack.h native_api.h)

find_library(sys_version NAMES version)
add_executable(version_test version_test.c)
target_link_libraries(version_test "${sys_version}")

add_library(winmm SHARED winmm.c winmm.h winmm.def ${SRC})
add_library(version SHARED version.c version.h version.def ${SRC})
add_library(iphlpapi SHARED iphlpapi.c iphlpapi.h iphlpapi.def ${SRC})
add_library(winsta SHARED winsta.c winsta.h winsta.def ${SRC})
add_library(winhttp SHARED winhttp.c winhttp.h winhttp.def ${SRC})
add_library(wtsapi32 SHARED wtsapi32.c wtsapi32.h wtsapi32.def ${SRC})
add_library(cryptsp SHARED cryptsp.c cryptsp.h cryptsp.def ${SRC})
add_library(msimg32 SHARED msimg32.c msimg32.h msimg32.def ${SRC})
