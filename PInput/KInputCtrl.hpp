#ifndef KINPUTCTRL_HPP_INCLUDED
#define KINPUTCTRL_HPP_INCLUDED

#include "../Shared/Definitions.h"
#include "Injector.hpp"

class KInputCtrl : private Injector
{
private:
    void *DLL;
    std::string FileName;
    ClientSurfaceInfo *SurfaceInfo;
    size_t CopiedPixelBufferSize;

    // Communication related structures
    HANDLE Pipe;
public:
    KInputCtrl(DWORD PID, const std::string& Path);

    bool PerformRequest(char CommandId, void *Arguments, size_t ArgumentsSize, void *Return, size_t ReturnSize);

    bool FocusEvent(std::int32_t ID);

    bool KeyEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t KeyCode,
                  std::uint16_t KeyChar, std::int32_t KeyLocation);

    bool MouseEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                    std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t Button);

    bool MouseWheelEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                         std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t ScrollType,
                         std::int32_t ScrollAmount, std::int32_t WheelRotation);

    void UpdateClientSurfaceInfo(bool CopyPixelBuffer);

    ClientSurfaceInfo *GetClientSurfaceInfo();

    ~KInputCtrl();
};

#endif // KINPUTCTRL_HPP_INCLUDED
