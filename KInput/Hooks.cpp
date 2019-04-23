
#include "Hooks.hpp"

ptr_SwapBuffers SwapBuffers_Original = nullptr;
LPVOID *SwapBuffers_Address = nullptr;

void __stdcall SwapBuffers_Hook(HDC hdc)
{
    // Try to read the pixel data
    // Check: https://lektiondestages.blogspot.com/2013/01/reading-opengl-backbuffer-to-system.html
    SwapBuffers_Original(hdc);
}

bool HookSwapBuffers()
{
    HMODULE kernel = GetModuleHandle("gdi32.dll");
    SwapBuffers_Address = (LPVOID *) GetProcAddress(kernel, "SwapBuffers");

    if (!SwapBuffers_Address)
    {
        return false;
    }
    std::cout << "Found SwapBuffers at " << ((void *) SwapBuffers_Address) << std::endl;

    // Create a hook for SwapBuffers, in disabled state.
    if (MH_CreateHook(SwapBuffers_Address, ((LPVOID *) &SwapBuffers_Hook),
                      ((LPVOID *) &SwapBuffers_Original)) != MH_OK)
    {
        return false;
    }
    std::cout << "Hooked it!" << std::endl;

    // Enable the hook for SwapBuffers.
    return MH_EnableHook(SwapBuffers_Address) == MH_OK;
}

bool UnHookSwapBuffers()
{
    // Disable the hook.
    return !(MH_DisableHook(SwapBuffers_Address) != MH_OK);
}

ptr_D3DBL_CopyImageToIntXrgbSurface CopyImageToIntXrgbSurface_Original = nullptr;
LPVOID *CopyImageToIntXrgbSurface_Address = nullptr;

#define CANVAS_SRC_TYPE 3

long __cdecl CopyImageToIntXrgbSurface_Hook(SurfaceDataRasInfo *pSrcInfo, int srctype, D3DResource *pDstSurfaceRes, jint srcx, jint srcy, jint srcWidth, jint srcHeight, jint dstx, jint dsty)
{
    long Temp = CopyImageToIntXrgbSurface_Original(pSrcInfo, srctype, pDstSurfaceRes, srcx, srcy, srcWidth, srcHeight, dstx, dsty);
    // Filter those who are not from a canvas! (Like buttons)
    if (srctype != CANVAS_SRC_TYPE) {
        return Temp;
    }
    if (Input)
    {
        Input->UpdateSurfaceInfo(pSrcInfo->bounds.x2 - pSrcInfo->bounds.x1, pSrcInfo->bounds.y2 - pSrcInfo->bounds.y1, pSrcInfo->rasBase);
    }
    return Temp;
}

bool HookCopyImageToIntXrgbSurface()
{
    HMODULE kernel = GetModuleHandle("awt.dll");
    CopyImageToIntXrgbSurface_Address = (LPVOID *) GetProcAddress(kernel, "?D3DBL_CopyImageToIntXrgbSurface@@YAJPAUSurfaceDataRasInfo@@HPAVD3DResource@@JJJJJJ@Z");

    if (!CopyImageToIntXrgbSurface_Address)
    {
        return false;
    }
    std::cout << "Found D3DBL_CopyImageToIntXrgbSurface at " << ((void *) CopyImageToIntXrgbSurface_Address) << std::endl;

    // Create a hook for CopyImageToIntXrgbSurface, in disabled state.
    if (MH_CreateHook(CopyImageToIntXrgbSurface_Address, ((LPVOID *) &CopyImageToIntXrgbSurface_Hook),
                      ((LPVOID *) &CopyImageToIntXrgbSurface_Original)) != MH_OK)
    {
        return false;
    }
    std::cout << "Hooked it!" << std::endl;

    // Enable the hook for CopyImageToIntXrgbSurface.
    return MH_EnableHook(CopyImageToIntXrgbSurface_Address) == MH_OK;
}

bool UnHookCopyImageToIntXrgbSurface()
{
    // Disable the hook.
    return !(MH_DisableHook(CopyImageToIntXrgbSurface_Address) != MH_OK);
}

ptr_SetDIBitsToDevice SetDIBitsToDevice_Original = nullptr;
LPVOID *SetDIBitsToDevice_Address = nullptr;

int __stdcall SetDIBitsToDevice_Hook(HDC hdc, int xDest, int yDest, DWORD w, DWORD h, int xSrc, int ySrc, UINT StartScan, UINT cLines, const VOID *lpvBits, const BITMAPINFO *lpbmi, UINT ColorUse)
{
    auto temp = SetDIBitsToDevice_Original(hdc, xDest, yDest, w, h, xSrc, ySrc, StartScan, cLines, lpvBits, lpbmi, ColorUse);
    if (Input)
    {
        Input->UpdateSurfaceInfo(w, h, lpvBits);
    }
    return temp;
}

bool HookSetDIBitsToDevice()
{
    HMODULE kernel = GetModuleHandle("gdi32.dll");
    SetDIBitsToDevice_Address = (LPVOID *) GetProcAddress(kernel, "SetDIBitsToDevice");

    if (!SetDIBitsToDevice_Address)
    {
        return false;
    }
    std::cout << "Found SetDIBitsToDevice at " << ((void *) SetDIBitsToDevice_Address) << std::endl;

    // Create a hook for SetDIBitsToDevice, in disabled state.
    if (MH_CreateHook(SetDIBitsToDevice_Address, ((LPVOID *) &SetDIBitsToDevice_Hook),
                      ((LPVOID *) &SetDIBitsToDevice_Original)) != MH_OK)
    {
        return false;
    }
    std::cout << "Hooked it!" << std::endl;

    // Enable the hook for SetDIBitsToDevice.
    return MH_EnableHook(SetDIBitsToDevice_Address) == MH_OK;
}

bool UnHookSetDIBitsToDevice()
{
    // Disable the hook.
    return !(MH_DisableHook(SetDIBitsToDevice_Address) != MH_OK);
}

ptr_CreateWindowExW CreateWindowExW_Original = nullptr;
LPVOID *CreateWindowExW_Address = nullptr;

HWND __stdcall CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    HWND Temp = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    if (Input)
    {
        std::wstring WStr(lpClassName);
        std::string ClassNameString(WStr.begin(), WStr.end());
        if (ClassNameString == "SunAwtCanvas")
        {
            Input->NotifyCanvasUpdate(Temp);
        }
    }
    return Temp;
}

bool HookCreateWindowW()
{
    HMODULE kernel = GetModuleHandle("user32.dll");
    CreateWindowExW_Address = (LPVOID *) GetProcAddress(kernel, "CreateWindowExW");
    if (!CreateWindowExW_Address)
    {
        return false;
    }

    // Create a hook for CreateWindowExW, in disabled state.
    if (MH_CreateHook(CreateWindowExW_Address, ((LPVOID *) &CreateWindowExW_Hook),
                      ((LPVOID *) &CreateWindowExW_Original)) != MH_OK)
    {
        return false;
    }

    // Enable the hook for CreateWindowExW.
    return MH_EnableHook(CreateWindowExW_Address) == MH_OK;
}

bool UnHookCreateWindow()
{
    // Disable the hook.
    return MH_DisableHook(CreateWindowExW_Address) == MH_OK;
}

bool InitializeMinHook()
{
    // Initialize MinHook.
    return MH_Initialize() == MH_OK;
}

bool UninitializeMinHook()
{
    // Uninitialize MinHook.
    return MH_Uninitialize() == MH_OK;
}

void ApplyHooks()
{
    InitializeMinHook();
    HookCreateWindowW();
    HookSwapBuffers();
    HookCopyImageToIntXrgbSurface();
    HookSetDIBitsToDevice();
}

void RemoveHooks()
{
    UnHookSetDIBitsToDevice();
    UnHookCopyImageToIntXrgbSurface();
    UnHookSwapBuffers();
    UnHookCreateWindow();
    UninitializeMinHook();
}