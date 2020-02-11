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
    public:
        explicit Injector(DWORD PID);
        void* Load(const std::string& DLLPath);
        bool Free(const std::string& DLLPath);
        ~Injector();
};

#endif // INJECTOR_HPP_INCLUDED
