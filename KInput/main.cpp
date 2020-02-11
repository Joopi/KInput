/*
    Copyright (C) <2018>  <Kasi Reddy>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <iostream>
#include "Hooks.hpp"
#include "KInput.hpp"
#include "KInputControllerListener.h"

KInput* Input = nullptr;
KInputControllerListener *ControllerListener = nullptr;

bool __stdcall DllMain(HMODULE DLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(DLL);
#ifdef DEBUG
            AllocConsole();
            freopen("CONOUT$", "wt", stdout);
            freopen("CONOUT$", "wt", stderr);
#endif
            ApplyHooks();
            Input = new KInput();
            ControllerListener = new KInputControllerListener(Input);
            ControllerListener->Start();
            break;
        case DLL_PROCESS_DETACH:
            delete ControllerListener;
            ControllerListener = nullptr;
            delete Input;
            Input = nullptr;
            RemoveHooks();
#ifdef DEBUG
            FreeConsole();
#endif
            break;
        default:
            break;
    }
    return TRUE;
}
