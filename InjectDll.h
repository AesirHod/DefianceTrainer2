#pragma comment(lib,"shlwapi.lib")

#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <shlwapi.h>

DWORD Inject(char* PROC_NAME, char* DLL_NAME, void* loaderAddress, DWORD* bytesWritten);