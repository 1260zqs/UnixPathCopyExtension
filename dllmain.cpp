#include "pch.h"
#include <Guiddef.h>
#include "ClassFactory.h"
#include "Reg.h"

// {F58148A4-FE8E-4EDD-8AFA-D6731885838F}
static const CLSID CLSID_ContextMenu =
{0xf58148a4, 0xfe8e, 0x4edd, {0x8a, 0xfa, 0xd6, 0x73, 0x18, 0x85, 0x83, 0x8f}};


HINSTANCE g_hInst   = NULL;
long g_cDllRef      = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(CLSID_ContextMenu, rclsid))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory* pClassFactory = new ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}

STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Register the component.
    hr = RegisterInprocServer(szModule, CLSID_ContextMenu, L_Friendly_Class_Name, L"Apartment");
    if (SUCCEEDED(hr))
    {
        // Register the context menu handler. The context menu handler is 
        // associated with the any file class.
        // Control the visibility in QueryContextMenu
        hr = RegisterShellExtContextMenuHandler(L"*", CLSID_ContextMenu, L_Friendly_Menu_Name);
    }

    return hr;
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Unregister the component.
    hr = UnregisterInprocServer(CLSID_ContextMenu);
    if (SUCCEEDED(hr))
    {
        // Unregister the context menu handler.
        hr = UnregisterShellExtContextMenuHandler(L"*", CLSID_ContextMenu);
    }
    
    return hr;
}
