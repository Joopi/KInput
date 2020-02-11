#ifndef HOOKS_HPP
#define HOOKS_HPP

#include <MinHook.h>
#include <iostream>
#include <windows.h>
#include <jni.h>
#include "KInput.hpp"

extern KInput *Input;

/**
* Direct draw render
*/

#define SD_RASINFO_PRIVATE_SIZE 64

struct SurfaceDataBounds
{
    jint x1;
    jint y1;
    jint x2;
    jint y2;
};

struct SurfaceDataRasInfo
{
    SurfaceDataBounds bounds;                 /* bounds of raster array */
    void *rasBase;               /* Pointer to (0, 0) pixel */
    jint pixelBitOffset;         /* bit offset to (0, *) pixel */
    jint pixelStride;            /* bytes to next X pixel */
    jint scanStride;             /* bytes to next Y pixel */
    unsigned int lutSize;                /* # colors in colormap */
    jint *lutBase;               /* Pointer to colormap[0] */
    unsigned char *invColorTable;         /* Inverse color table */
    char *redErrTable;           /* Red ordered dither table */
    char *grnErrTable;           /* Green ordered dither table */
    char *bluErrTable;           /* Blue ordered dither table */
    int *invGrayTable;          /* Inverse gray table */
    union
    {
        void *align;                 /* ensures strict alignment */
        char data[SD_RASINFO_PRIVATE_SIZE];
    } priv;
};

/**
 * OpenGL render (For RuneLite)
 */

typedef void(__stdcall *ptr_SwapBuffers) (HDC);

void __stdcall SwapBuffers_Hook(HDC hdc);

bool HookSwapBuffers();

bool UnHookSwapBuffers();

/**
 * XrgbSurface render
 */

typedef long(__cdecl *ptr_D3DBL_CopyImageToIntXrgbSurface)(SurfaceDataRasInfo *, int, class D3DResource *, long, long, long, long, long, long);

long __cdecl CopyImageToIntXrgbSurface_Hook(SurfaceDataRasInfo *pSrcInfo, int srctype, D3DResource *pDstSurfaceRes, jint srcx, jint srcy, jint srcWidth, jint srcHeight, jint dstx, jint dsty);

bool HookCopyImageToIntXrgbSurface();

bool UnHookCopyImageToIntXrgbSurface();

/**
 * GDI Render
 */

typedef int(__stdcall *ptr_SetDIBitsToDevice)(HDC, int, int, DWORD, DWORD, int, int, UINT, UINT, const VOID *, const BITMAPINFO *, UINT);

int __stdcall SetDIBitsToDevice_Hook(HDC hdc, int xDest, int yDest, DWORD w, DWORD h, int xSrc, int ySrc, UINT StartScan, UINT cLines, const VOID *lpvBits, const BITMAPINFO *lpbmi, UINT ColorUse);

bool HookSetDIBitsToDevice();

bool UnHookSetDIBitsToDevice();

/**
 * Canvas update
 */

typedef HWND(__stdcall *ptr_CreateWindowExW)(DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);

HWND __stdcall CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

bool HookCreateWindowW();

bool UnHookCreateWindow();

/**
 * Hooks initialization
 */

bool InitializeMinHook();

bool UninitializeMinHook();

void ApplyHooks();

void RemoveHooks();

#endif