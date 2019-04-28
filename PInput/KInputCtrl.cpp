/*
    Copyright (C) <2018>  <Kasi Reddy> <Andre>

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
#include "KInputCtrl.hpp"

KInputCtrl::KInputCtrl(DWORD PID, const std::string& Path) : Injector(PID)
{
    this->FileName = Path + "\\KInput.dll";
    this->DLL = this->Load(FileName);
    this->SurfaceInfo = new ClientSurfaceInfo();
    this->CopiedPixelBufferSize = 0;

    // Init the pipe name
    auto PipeName = R"(\\.\pipe\)" + std::string(PIPE_PREFIX) + std::to_string(PID);

    // Open the name pipe! Wait if necessary!
    while (true)
    {
        this->Pipe = CreateFile(
                PipeName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr);

        // Break if the pipe handle is valid.
        if (this->Pipe != INVALID_HANDLE_VALUE)
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            std::cerr << "Could not open pipe" << std::endl;
        }

        // All pipe instances are busy, so wait for 20 seconds.
        if (!WaitNamedPipe(PipeName.c_str(), 20000))
        {
            std::cerr << "Could not open pipe: 20 second wait timed out." << std::endl;
            return;
        }
    }

    // The pipe is connected, change the read mode to message
    DWORD PipeMode = PIPE_READMODE_MESSAGE;
    bool Success = SetNamedPipeHandleState(this->Pipe, &PipeMode, nullptr, nullptr);
    if (!Success)
    {
        std::cerr << "Failed to set pipe state." << std::endl;
        return;
    }
    std::cout << "Connected to the PIPE!!!!" << std::endl;
}

bool KInputCtrl::PerformRequest(char CommandId, void *Arguments, size_t ArgumentsSize, void *Return, size_t ReturnSize)
{
    // Send the event!
    DWORD BytesWritten;
    DWORD MessageSize = ArgumentsSize + 1;
    void *MessageBytes = malloc(MessageSize);
    *((char*) MessageBytes) = CommandId;
    if (Arguments != nullptr && ArgumentsSize > 0) {
        memcpy((void *) (((char*) MessageBytes) + 1), Arguments, ArgumentsSize);
    }
    bool Success = WriteFile(this->Pipe, (void *) MessageBytes, MessageSize, &BytesWritten, nullptr);
    free(MessageBytes);
    if (!Success || BytesWritten != MessageSize) {
        std::cerr << "Failed to send message to pipe!" << std::endl;
        return false;
    }

    // If it is a void we don't need to read for any reply
    if (ReturnSize == 0 || Return == nullptr)
        return true;

    // Read the reply
    DWORD BytesRead;
    auto Connected = ReadFile(this->Pipe, (void *) Return, ReturnSize, &BytesRead, nullptr);
    return Connected && BytesRead == ReturnSize;
}

bool KInputCtrl::FocusEvent(std::int32_t ID)
{
    struct FocusEvent Event{};
    Event.ID = ID;

    // Perform the request
    bool Reply;
    return PerformRequest(FOCUS_EVENT_COMMAND, &Event, sizeof(Event), &Reply, sizeof(bool)) && Reply;
}

bool KInputCtrl::KeyEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t KeyCode,
                          std::uint16_t KeyChar, std::int32_t KeyLocation)
{
    struct KeyEvent Event{};
    Event.ID = ID;
    Event.When = When;
    Event.Modifiers = Modifiers;
    Event.KeyCode = KeyCode;
    Event.KeyChar = KeyChar;
    Event.KeyLocation = KeyLocation;

    // Perform the request
    bool Reply;
    return PerformRequest(KEY_EVENT_COMMAND, &Event, sizeof(Event), &Reply, sizeof(bool)) && Reply;
}

bool KInputCtrl::MouseEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                            std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t Button)
{
    struct MouseEvent Event{};
    Event.ID = ID;
    Event.When = When;
    Event.Modifiers = Modifiers;
    Event.X = X;
    Event.Y = Y;
    Event.ClickCount = ClickCount;
    Event.PopupTrigger = PopupTrigger;
    Event.Button = Button;

    // Perform the request
    bool Reply;
    return PerformRequest(MOUSE_EVENT_COMMAND, &Event, sizeof(Event), &Reply, sizeof(bool)) && Reply;
}

bool KInputCtrl::MouseWheelEvent(std::int32_t ID, std::int64_t When, std::int32_t Modifiers, std::int32_t X,
                                 std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger, std::int32_t ScrollType,
                                 std::int32_t ScrollAmount, std::int32_t WheelRotation)
{
    struct MouseWheelEvent Event{};
    Event.ID = ID;
    Event.When = When;
    Event.Modifiers = Modifiers;
    Event.X = X;
    Event.Y = Y;
    Event.ClickCount = ClickCount;
    Event.PopupTrigger = PopupTrigger;
    Event.ScrollType = ScrollType;
    Event.ScrollAmount = ScrollAmount;
    Event.WheelRotation = WheelRotation;

    // Perform the request
    bool Reply;
    return PerformRequest(MOUSE_WHEEL_EVENT_COMMAND, &Event, sizeof(Event), &Reply, sizeof(bool)) && Reply;
}

void KInputCtrl::UpdateClientSurfaceInfo(bool CopyPixelBuffer)
{
    // Request for an update and wait for the next frame (If copying from it)!
    if (CopyPixelBuffer) {
        bool Requested;
        auto Success = PerformRequest(REQUEST_PIXEL_BUFFER_COMMAND, nullptr, 0, &Requested, sizeof(bool));
        if (!Success || !Requested) {
            return;
        }
    }

    // Read the SurfaceInfo from KInput
    ClientSurfaceInfo RemoteClientSurfaceInfo;
    auto Success = PerformRequest(GET_SURFACE_INFO_COMMAND, nullptr, 0, &RemoteClientSurfaceInfo, sizeof(ClientSurfaceInfo));
    if (!Success) {
        return;
    }

    // Update the dimensions
    this->SurfaceInfo->Width = RemoteClientSurfaceInfo.Width;
    this->SurfaceInfo->Height = RemoteClientSurfaceInfo.Height;

    if (CopyPixelBuffer) {
        // Allocate new memory or resize the existing one in-case the pixel buffer is bigger!
        if (this->SurfaceInfo->PixelBuffer == nullptr) {
            this->SurfaceInfo->PixelBuffer = static_cast<const BGRA *>(malloc(RemoteClientSurfaceInfo.GetPixelBufferSize()));
        } else if (CopiedPixelBufferSize != RemoteClientSurfaceInfo.GetPixelBufferSize())
        {
            this->SurfaceInfo->PixelBuffer = static_cast<const BGRA *>(realloc((void*) this->SurfaceInfo->PixelBuffer, RemoteClientSurfaceInfo.GetPixelBufferSize()));
        }
        // Update the copied buffer size
        CopiedPixelBufferSize = RemoteClientSurfaceInfo.GetPixelBufferSize();

        // Read the remote pixel buffer into the local pixel buffer
        if (RemoteClientSurfaceInfo.PixelBuffer) {
            ReadProcessMemory(this->ProcessHandle, RemoteClientSurfaceInfo.PixelBuffer, (void*) this->SurfaceInfo->PixelBuffer, RemoteClientSurfaceInfo.GetPixelBufferSize(), nullptr);
        }
    }
}

ClientSurfaceInfo *KInputCtrl::GetClientSurfaceInfo()
{
    return this->SurfaceInfo;
}

KInputCtrl::~KInputCtrl()
{
    delete this->SurfaceInfo->PixelBuffer;
    delete this->SurfaceInfo;
    CloseHandle(this->Pipe);
    this->Free(this->FileName);
}
