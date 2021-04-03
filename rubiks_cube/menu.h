#pragma once
#ifndef MENU_H
#define MENU_H

#include "root.h"

namespace cube
{
	void setMenu()
	{
        // Menu
        hwndGL = GetActiveWindow();
        ShowWindow(hwndGL, SW_MAXIMIZE);
        HMENU hMenu = CreateMenu();
        HMENU hMenu_material = CreatePopupMenu();
        HMENU hMenu_light = CreatePopupMenu();

        OldProc = (WNDPROC)SetWindowLong(hwndGL, GWL_WNDPROC, (LONG)WndProc);

        AppendMenu(hMenu, MF_STRING, 1, TEXT("Random State"));
        AppendMenu(hMenu, MF_STRING, 2, TEXT("Read State"));
        AppendMenu(hMenu, MF_STRING, 3, TEXT("Reset"));
        AppendMenu(hMenu, MF_POPUP, (UINT)hMenu_material, TEXT("Material"));
        AppendMenu(hMenu, MF_POPUP, (UINT)hMenu_light, TEXT("Light"));
        AppendMenu(hMenu_material, MF_STRING, 11, TEXT("Pearl"));
        AppendMenu(hMenu_material, MF_STRING, 12, TEXT("White plastic"));
        AppendMenu(hMenu_material, MF_STRING, 13, TEXT("White rubber"));
        AppendMenu(hMenu_material, MF_STRING, 14, TEXT("Emerald"));
        AppendMenu(hMenu_material, MF_STRING, 15, TEXT("Jade"));
        AppendMenu(hMenu_material, MF_STRING, 16, TEXT("Obsidian"));
        AppendMenu(hMenu_material, MF_STRING, 17, TEXT("Ruby"));
        AppendMenu(hMenu_material, MF_STRING, 18, TEXT("Turquoise"));
        AppendMenu(hMenu_material, MF_STRING, 19, TEXT("Silver"));
        AppendMenu(hMenu_light, MF_STRING, 21, TEXT("On/Off"));
        AppendMenu(hMenu_light, MF_STRING, 22, TEXT("Gradient"));
        AppendMenu(hMenu_light, MF_STRING, 23, TEXT("Light Move/Stop"));


        SetMenu(hwndGL, hMenu);
	}
}

#endif // !MENU_H
