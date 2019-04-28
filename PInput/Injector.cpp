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

#include "Injector.hpp"
#include <iostream>

Injector::Injector(DWORD PID) : PID(PID), ProcessHandle(nullptr)
{
    this->ProcessHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, PID);
}

void* Injector::Load(const std::string& DLLPath)
{
    void* Result = nullptr;
    if (!this->ProcessHandle)
        return Result;
    if (Modules.count(DLLPath))
        return Modules[DLLPath];
    void* LoadLib = (void*)GetProcAddress(GetModuleHandle("Kernel32.dll"), "LoadLibraryA");
    if (LoadLib)
    {
        void* DLLEntry = VirtualAllocEx(this->ProcessHandle, nullptr, sizeof(char) * (DLLPath.size() + 1), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (DLLEntry)
        {
            if (WriteProcessMemory(this->ProcessHandle, DLLEntry, DLLPath.c_str(), sizeof(char) * (DLLPath.size() + 1), nullptr))
            {
                HANDLE RemoteThread = CreateRemoteThread(this->ProcessHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLib, DLLEntry, 0, nullptr);
                if (RemoteThread)
                {
                    WaitForSingleObject(RemoteThread, INFINITE);
                    void* DLL = nullptr;
                    if (GetExitCodeThread(RemoteThread, (LPDWORD)&DLL))
                    {
                        if (DLL)
                        {
                            Modules[DLLPath] = DLL;
                            Result = DLL;
                        }
                    }
                    CloseHandle(RemoteThread);
                }
            }
            VirtualFreeEx(this->ProcessHandle, DLLEntry, 0, MEM_RELEASE);
        }
    }
    return Result;
}

bool Injector::Free(const std::string& DLLPath)
{
    bool Result = false;
    if (!this->ProcessHandle)
        return Result;
    if (!Modules.count(DLLPath))
        return true;
    void* DLL = Modules[DLLPath];
    void* FreeLib = (void*)GetProcAddress(GetModuleHandle("Kernel32.dll"), "FreeLibrary");
    if (FreeLib)
    {
        HANDLE RemoteThread = CreateRemoteThread(this->ProcessHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)FreeLib, DLL, 0, nullptr);
        if (RemoteThread)
        {
            WaitForSingleObject(RemoteThread, INFINITE);
            BOOL Freed = FALSE;
            if (GetExitCodeThread(RemoteThread, (LPDWORD)&Freed))
            {
                if (Freed)
                {
                    this->Modules.erase(DLLPath);
                    Result = true;
                }
            }
            CloseHandle(RemoteThread);
        }
    }
    return Result;
}

Injector::~Injector()
{
    if (this->ProcessHandle)
    {
        CloseHandle(this->ProcessHandle);
        this->ProcessHandle = nullptr;
    }
}
