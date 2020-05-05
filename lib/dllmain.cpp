// dllmain.cpp - defines the entry point for the DLL application.
#include "pch.h"

#define WIN32_LEAN_AND_MEAN             // exclude rarely-used stuff from Windows headers
#include <windows.h>

BOOL APIENTRY DllMain(
	_in HMODULE hModule,
	_in DWORD ReasonForCall,
	_in LPVOID pReserved
) {
    switch (ReasonForCall) {
    case DLL_PROCESS_ATTACH:
		Winapi::DisableThreadLibraryCalls(hModule);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

