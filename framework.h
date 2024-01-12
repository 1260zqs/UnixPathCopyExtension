#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>

#define L_Menu_Text L"Copy Path (Unix Format)"

#define Verb_Name "Copy Path (Unix Format)"
#define L_Verb_Name TEXT(Verb_Name)

#define Verb_Canonical_Name Verb_Name
#define L_Verb_Canonical_Name TEXT(Verb_Canonical_Name)

#define Verb_Help_Text "Copy Path As Unix Format \"/\""
#define L_Verb_Help_Text TEXT(Verb_Help_Text)

#define L_Friendly_Menu_Name L"CopyPathAsUnixFormat"
#define L_Friendly_Class_Name L"ContextMenuHandler.CopyPathAsUnixFormat"


