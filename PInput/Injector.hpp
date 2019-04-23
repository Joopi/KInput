#ifndef INJECTOR_HPP_INCLUDED
#define INJECTOR_HPP_INCLUDED

#include <windows.h>
#include <map>

class Injector
{
    protected:
        DWORD PID;
        std::map<std::string, void*> Modules;
        HANDLE ProcessHandle;

        void* GetRemoteProcAddress(void* DLL, std::string ProcName);
    public:
        explicit Injector(DWORD PID);
        void* Load(std::string DLLPath);
        bool CallExport(void* DLL, std::string ProcName, void* Data, std::uint32_t Size);
        bool CallExport(std::string DLL, std::string ProcName, void* Data, std::uint32_t Size);
        DWORD CallExport(void* DLL, std::string ProcName);

        bool Free(std::string DLLPath);
        ~Injector();
};

#endif // INJECTOR_HPP_INCLUDED
