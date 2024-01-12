#pragma once
#include <shlobj.h>     // For IShellExtInit and IContextMenu
#include <string>
#include <vector>

class UnixPathCopyExtension : public IShellExtInit, public IContextMenu
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IShellExtInit
    IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

    // IContextMenu
    IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax);

    UnixPathCopyExtension(void);
protected:
    ~UnixPathCopyExtension(void);
private:
    // Reference count of component.
    long m_cRef;

    // The method that handles the "display" verb.
    void OnVerbDisplayFileName(LPCMINVOKECOMMANDINFO pici);

    HANDLE m_hMenuBmp;
    LPDATAOBJECT pDataObj;
};
