/****************************** Module Header ******************************\
Module Name:  FileContextMenuExt.cpp
Project:      CppShellExtContextMenuHandler
Copyright (c) Microsoft Corporation.

The code sample demonstrates creating a Shell context menu handler with C++.

A context menu handler is a shell extension handler that adds commands to an
existing context menu. Context menu handlers are associated with a particular
file class and are called any time a context menu is displayed for a member
of the class. While you can add items to a file class context menu with the
registry, the items will be the same for all members of the class. By
implementing and registering such a handler, you can dynamically add items to
an object's context menu, customized for the particular object.

The example context menu handler adds the menu item "Display File Name (C++)"
to the context menu when you right-click a .cpp file in the Windows Explorer.
Clicking the menu item brings up a message box that displays the full path
of the .cpp file.

This source is subject to the Microsoft Public License.
See http://www.microsoft.com/opensource/licenses.mspx#Ms-PL.
All other rights reserved.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include "pch.h"
#include "framework.h"
#include "UnixPathCopyExtension.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include <shellapi.h>
#include <sstream>
#include "framework.h"
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Msimg32.lib")

extern HINSTANCE g_hInst;
extern long g_cDllRef;
HBITMAP GetBitmapFromModuleResource(HMODULE hModule, int resourceId);
HBITMAP CreateBitmapFromIcon(HICON hIcon);
HRESULT CopyToClipboard(HGLOBAL hGlobal);
void ReplaceChar(LPWSTR input, WCHAR oldChar, WCHAR newChar);
void ReplaceChar(LPSTR input, CHAR oldChar, CHAR newChar);

#define IDM_DISPLAY             0  // The command's identifier offset

UnixPathCopyExtension::UnixPathCopyExtension(void): m_cRef(1)
{
    InterlockedIncrement(&g_cDllRef);
    // Load the bitmap for the menu item. 
    // If you want the menu item bitmap to be transparent, the color depth of 
    // the bitmap must not be greater than 8bpp.
    // m_hMenuBmp = LoadImage(g_hInst, MAKEINTRESOURCE(IDB_OK), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
}

UnixPathCopyExtension::~UnixPathCopyExtension(void)
{
    // if (m_hMenuBmp)
    // {
    //     DeleteObject(m_hMenuBmp);
    //     m_hMenuBmp = NULL;
    // }
    InterlockedDecrement(&g_cDllRef);
}


void UnixPathCopyExtension::OnVerbDisplayFileName(LPCMINVOKECOMMANDINFO pici)
{
    if (pDataObj)
    {
        STGMEDIUM stm{};
        FORMATETC fmt = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        if (SUCCEEDED(pDataObj->GetData(&fmt, &stm)))
        {
            HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
            if (hDrop)
            {
                UINT charCount = 0;
                UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
                for (UINT i = 0; i < fileCount; ++i)
                {
                    UINT size = DragQueryFile(hDrop, i, NULL, 0);
                    if (size) charCount += size + 1;
                }
                if (charCount)
                {
                    charCount += 1;
                    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, charCount * sizeof(WCHAR));
                    if (hGlobal)
                    {
                        LPWSTR pszData = static_cast<LPWSTR>(GlobalLock(hGlobal));
                        ZeroMemory(pszData, charCount * sizeof(WCHAR));
                        LPWSTR buffer = pszData;
                        for (UINT i = 0; i < fileCount; ++i)
                        {
                            UINT size = DragQueryFile(hDrop, i, nullptr, 0);
                            DragQueryFile(hDrop, i, buffer, size + 1);
                            buffer += size;
                            if (i < fileCount - 1)
                            {
                                *buffer = '\n';
                                buffer++;
                            }
                        }
                        ReplaceChar(pszData, '\\', '/');
                        GlobalUnlock(hGlobal);
                        CopyToClipboard(hGlobal);
                    }
                }
                else
                {
                    OutputDebugString(L"no file selected");
                }
                GlobalUnlock(stm.hGlobal);
            }
            ReleaseStgMedium(&stm);
        }
    }
    else if (pici && pici->lpDirectory)
    {
        int charCount = MultiByteToWideChar(CP_ACP, 0, pici->lpDirectory, -1, NULL, 0);
        if (charCount > 0)
        {
            size_t bufferSize = charCount * sizeof(WCHAR);
            HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, bufferSize);
            if (hGlobal)
            {
                LPWSTR pszData = static_cast<LPWSTR>(GlobalLock(hGlobal));
                ZeroMemory(pszData, bufferSize);
                if (MultiByteToWideChar(CP_ACP, 0, pici->lpDirectory, -1, pszData, charCount))
                {
                    ReplaceChar(pszData, '\\', '/');
                    GlobalUnlock(hGlobal);
                    CopyToClipboard(hGlobal);
                }
                else
                {
                    GlobalUnlock(hGlobal);
                }
            }
        }
        // size_t bufferSize = strlen(pici->lpDirectory) * sizeof(CHAR) + 1;
        // if (hGlobal)
        // {
        //     LPWSTR pszData = static_cast<LPWSTR>(GlobalLock(hGlobal));
        //     ZeroMemory(pszData, bufferSize * sizeof(CHAR));
        //     memcpy(pszData, pici->lpDirectory, bufferSize);
        //     ReplaceChar(pszData, '\\', '/');
        //     GlobalUnlock(hGlobal);
        //     CopyToClipboard(hGlobal);
        // }
        OutputDebugStringA("use lpDirectory");
        OutputDebugStringA(pici->lpDirectory);
    }
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP UnixPathCopyExtension::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(UnixPathCopyExtension, IContextMenu),
        QITABENT(UnixPathCopyExtension, IShellExtInit),
        {0},
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) UnixPathCopyExtension::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) UnixPathCopyExtension::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

#pragma endregion


#pragma region IShellExtInit

// Initialize the context menu handler.
IFACEMETHODIMP UnixPathCopyExtension::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{
    this->pDataObj = pDataObj;
    return S_OK;
}

#pragma endregion


#pragma region IContextMenu

//
//   FUNCTION: FileContextMenuExt::QueryContextMenu
//
//   PURPOSE: The Shell calls IContextMenu::QueryContextMenu to allow the 
//            context menu handler to add its menu items to the menu. It 
//            passes in the HMENU handle in the hmenu parameter. The 
//            indexMenu parameter is set to the index to be used for the 
//            first menu item that is to be added.
//
IFACEMETHODIMP UnixPathCopyExtension::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    // If uFlags include CMF_DEFAULTONLY then we should not do anything.
    if (CMF_DEFAULTONLY & uFlags)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
    }


    // Use either InsertMenu or InsertMenuItem to add menu items.
    // Learn how to add sub-menu from:
    // http://www.codeproject.com/KB/shell/ctxextsubmenu.aspx

    MENUITEMINFOW mii{sizeof(MENUITEMINFOW)};
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mii.wID = idCmdFirst + IDM_DISPLAY;
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;
    constexpr size_t txt_size = ARRAYSIZE(L_Menu_Text) * sizeof(WCHAR);
    WCHAR pszMenuText[ARRAYSIZE(L_Menu_Text) + 1];
    ZeroMemory(pszMenuText, txt_size);
    memcpy(pszMenuText,L_Menu_Text, txt_size);
    mii.dwTypeData = pszMenuText;
    // HMODULE imageres = LoadLibrary(TEXT("imageres.dll"));
    // HBITMAP hbitmap = GetBitmapFromModuleResource(imageres, 5302);
    // mii.hbmpItem = hbitmap;
    // FreeLibrary(imageres);
    // HMODULE imageres = LoadLibrary(L"imageres.dll");
    // HICON hSystemIcon = LoadIcon(imageres, MAKEINTRESOURCE(5302));
    // if (hSystemIcon)
    // {
    //     mii.hbmpItem = CreateBitmapFromIcon(hSystemIcon);
    //     DestroyIcon(hSystemIcon);
    // }
    // if (imageres) FreeLibrary(imageres);
    if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Add a separator.
    // MENUITEMINFOW sep = {sizeof(sep)};
    // sep.fMask = MIIM_TYPE;
    // sep.fType = MFT_SEPARATOR;
    // if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
    // {
    //     return HRESULT_FROM_WIN32(GetLastError());
    // }

    // Return an HRESULT value with the severity set to SEVERITY_SUCCESS. 
    // Set the code value to the offset of the largest command identifier 
    // that was assigned, plus one (1).
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, USHORT(IDM_DISPLAY + 1));
}


//
//   FUNCTION: FileContextMenuExt::InvokeCommand
//
//   PURPOSE: This method is called when a user clicks a menu item to tell 
//            the handler to run the associated command. The lpcmi parameter 
//            points to a structure that contains the needed information.
//
IFACEMETHODIMP UnixPathCopyExtension::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    BOOL fUnicode = FALSE;

    // Determine which structure is being passed in, CMINVOKECOMMANDINFO or 
    // CMINVOKECOMMANDINFOEX based on the cbSize member of lpcmi. Although 
    // the lpcmi parameter is declared in Shlobj.h as a CMINVOKECOMMANDINFO 
    // structure, in practice it often points to a CMINVOKECOMMANDINFOEX 
    // structure. This struct is an extended version of CMINVOKECOMMANDINFO 
    // and has additional members that allow Unicode strings to be passed.
    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        if (pici->fMask & CMIC_MASK_UNICODE)
        {
            fUnicode = TRUE;
        }
    }

    // Determines whether the command is identified by its offset or verb.
    // There are two ways to identify commands:
    // 
    //   1) The command's verb string 
    //   2) The command's identifier offset
    // 
    // If the high-order word of lpcmi->lpVerb (for the ANSI case) or 
    // lpcmi->lpVerbW (for the Unicode case) is nonzero, lpVerb or lpVerbW 
    // holds a verb string. If the high-order word is zero, the command 
    // offset is in the low-order word of lpcmi->lpVerb.

    // For the ANSI case, if the high-order word is not zero, the command's 
    // verb string is in lpcmi->lpVerb. 
    if (!fUnicode && HIWORD(pici->lpVerb))
    {
        // Is the verb supported by this context menu extension?
        if (StrCmpIA(pici->lpVerb, Verb_Name) == 0)
        {
            OnVerbDisplayFileName(pici);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }
    // For the Unicode case, if the high-order word is not zero, the 
    // command's verb string is in lpcmi->lpVerbW. 
    else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
    {
        // Is the verb supported by this context menu extension?
        if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, L_Verb_Name) == 0)
        {
            OnVerbDisplayFileName(pici);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

    // If the command cannot be identified through the verb string, then 
    // check the identifier offset.
    else
    {
        // Is the command identifier offset supported by this context menu 
        // extension?
        if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
        {
            OnVerbDisplayFileName(pici);
        }
        else
        {
            // If the verb is not recognized by the context menu handler, it 
            // must return E_FAIL to allow it to be passed on to the other 
            // context menu handlers that might implement that verb.
            return E_FAIL;
        }
    }

    return S_OK;
}


//
//   FUNCTION: CFileContextMenuExt::GetCommandString
//
//   PURPOSE: If a user highlights one of the items added by a context menu 
//            handler, the handler's IContextMenu::GetCommandString method is 
//            called to request a Help text string that will be displayed on 
//            the Windows Explorer status bar. This method can also be called 
//            to request the verb string that is assigned to a command. 
//            Either ANSI or Unicode verb strings can be requested. This 
//            example only implements support for the Unicode values of 
//            uFlags, because only those have been used in Windows Explorer 
//            since Windows 2000.
//
IFACEMETHODIMP UnixPathCopyExtension::GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_INVALIDARG;

    if (idCommand == IDM_DISPLAY)
    {
        switch (uFlags)
        {
        case GCS_HELPTEXTW:
            // Only useful for pre-Vista versions of Windows that have a 
            // Status bar.
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, L_Verb_Help_Text);
            break;

        case GCS_VERBW:
            // GCS_VERBW is an optional feature that enables a caller to 
            // discover the canonical name for the verb passed in through 
            // idCommand.
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, L_Verb_Canonical_Name);
            break;

        default:
            hr = S_OK;
        }
    }

    // If the command (idCommand) is not supported by this context menu 
    // extension handler, return E_INVALIDARG.

    return hr;
}

#pragma endregion

HBITMAP CreateBitmapFromIcon(HICON hIcon)
{
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);

    BITMAP bm;
    GetObject(iconInfo.hbmColor, sizeof(bm), &bm);

    HDC hdc = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 16, 16);  // Adjust the size as needed
    SelectObject(hdcMem, hBitmap);

    // Draw the icon into the smaller bitmap with transparency
    DrawIconEx(hdcMem, 0, 0, hIcon, 16, 16, 0, nullptr, DI_NORMAL);

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdc);

    return hBitmap;
}

HBITMAP GetBitmapFromModuleResource(HMODULE hModule, int resourceId)
{
    // Load an icon resource
    HICON hIcon = LoadIcon(hModule, MAKEINTRESOURCE(resourceId));
    if (hIcon == NULL)
    {
        return NULL;
    }

    // Convert the icon to a bitmap
    ICONINFO iconInfo;
    if (GetIconInfo(hIcon, &iconInfo) == 0)
    {
        DestroyIcon(hIcon);
        return NULL;
    }

    BITMAP bm;
    if (GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bm) == 0)
    {
        // Failed to get object information
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        DestroyIcon(hIcon);
        return NULL;
    }

    // Create a new DIB section bitmap
    HBITMAP hBitmap = CreateDIBSection(NULL, (BITMAPINFO*)&bm, DIB_RGB_COLORS, NULL, NULL, 0);
    if (hBitmap != NULL)
    {
        // Draw the icon onto the bitmap
        HDC hdcBitmap = CreateCompatibleDC(NULL);
        if (hdcBitmap != NULL)
        {
            SelectObject(hdcBitmap, hBitmap);
            DrawIconEx(hdcBitmap, 0, 0, hIcon, bm.bmWidth, bm.bmHeight, 0, NULL, DI_NORMAL);
            DeleteDC(hdcBitmap);
        }
    }
    else
    {
        OutputDebugString(TEXT("CreateDIBSection null"));
    }

    // Clean up resources
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    DestroyIcon(hIcon);

    return hBitmap;
}


HRESULT CopyToClipboard(HGLOBAL hGlobal)
{
    if (!OpenClipboard(NULL))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    EmptyClipboard();
    if (hGlobal)
    {
        SetClipboardData(CF_UNICODETEXT, hGlobal);
    }
    CloseClipboard();
    return S_OK;
}


void ReplaceChar(LPWSTR input, WCHAR oldChar, WCHAR newChar)
{
    while (*input)
    {
        if (*input == oldChar)
        {
            *input = newChar;
        }
        input++;
    }
}

void ReplaceChar(LPSTR input, CHAR oldChar, CHAR newChar)
{
    while (*input)
    {
        if (*input == oldChar)
        {
            *input = newChar;
        }
        input++;
    }
}