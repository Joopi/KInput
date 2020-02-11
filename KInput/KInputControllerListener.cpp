/*
    Copyright (C) <2018>  <Andre>

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

#include <iostream>
#include "KInputControllerListener.h"

bool KInputControllerListener::Start()
{
    // Init the pipe name
    auto PipeName = R"(\\.\pipe\)" + std::string(PIPE_PREFIX) + std::to_string(GetCurrentProcessId());

    // Create a named pipe
    this->Pipe = CreateNamedPipe(
            PipeName.c_str(),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,
            BUFF_SIZE,
            BUFF_SIZE,
            0,
            nullptr);

    if (this->Pipe == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to create a pipe" << std::endl;
        return false;
    }

    // Start the thread
    this->Running = true;
    this->CommunicationThread = std::thread(&KInputControllerListener::CommunicationListener, this);
    return true;
}

void KInputControllerListener::CommunicationListener()
{
    // Await connection, keep retrying if a client fails to connect
    auto Connected = false;
    while (!Connected && this->Running)
    {
        Connected = ConnectNamedPipe(this->Pipe, nullptr) ? true : (GetLastError() == ERROR_PIPE_CONNECTED);
    }

    // If was unable to connect, but ordered to stop, stop
    if (!Connected)
        return;

    // When connected and running await for commands
    void *CommandBuffer, *ReplyBuffer;
    CommandBuffer = malloc(BUFF_SIZE);
    ReplyBuffer = malloc(BUFF_SIZE);
    DWORD CommandSize, ReplySize, ReplyWritten;
    while (this->Running)
    {
        std::cout << "Wating for messages!" << std::endl;
        Connected = ReadFile(this->Pipe, (void *) CommandBuffer, BUFF_SIZE, &CommandSize, nullptr);
        std::cout << "Got one!" << std::endl;

        // Check if pipe is till up! If not stop communication!
        if (!Connected || CommandSize == 0)
        {
            std::cout << "Pipe broke!!!" << std::endl;
            break;
        }

        // Process the request
        this->ProcessCommand(CommandBuffer, CommandSize, ReplyBuffer, &ReplySize);

        // Reply (except for "void" calls = ReplySize = 0)
        if (ReplySize == 0)
        {
            continue;
        }

        // Send the reply
        Connected = WriteFile(this->Pipe, ReplyBuffer, ReplySize, &ReplyWritten, nullptr);
        if (!Connected || ReplySize != ReplyWritten)
        {
            std::cout << "Failed to reply!" << std::endl;
            continue;
        }
    }
    free(CommandBuffer);
    free(ReplyBuffer);
    std::cout << "Reached the end of the thread!!!" << std::endl;
}

void KInputControllerListener::ProcessCommand(void *CommandBuffer, DWORD CommandSize, void *ReplyBuffer, LPDWORD ReplySize)
{
    // Process the command
    char Command = *((char *) CommandBuffer);
    void *Arguments = CommandSize > sizeof(char) ? (((char *) CommandBuffer) + 1) : nullptr;
    switch(Command){
        case FOCUS_EVENT_COMMAND:
        {
            *ReplySize = sizeof(bool);
            if (!Input)
            {
                *((bool *) ReplyBuffer) = false;
                return;
            }
            auto Event = (FocusEvent *) Arguments;
            *((bool *) ReplyBuffer) = Input->FocusEvent(Event->ID);
            return;
        }

        case KEY_EVENT_COMMAND: {
            *ReplySize = sizeof(bool);
            if (!Input)
            {
                *((bool *) ReplyBuffer) = false;
                return;
            }
            std::cout << "Got a key event message from the pipe!" << std::endl;
            auto Event = (KeyEvent*) Arguments;
            *((bool *) ReplyBuffer) = Input->KeyEvent(Event->ID, Event->When, Event->Modifiers, Event->KeyCode,
                                   Event->KeyChar, Event->KeyLocation);
            return;
        }

        case MOUSE_EVENT_COMMAND: {
            *ReplySize = sizeof(bool);
            if (!Input)
            {
                *((bool *) ReplyBuffer) = false;
                return;
            }
            std::cout << "Got a key event message from the pipe!" << std::endl;
            auto Event = (MouseEvent*) Arguments;
            *((bool *) ReplyBuffer) = Input->MouseEvent(Event->ID, Event->When, Event->Modifiers, Event->X,
                                             Event->Y, Event->ClickCount, Event->PopupTrigger, Event->Button);
            return;
        }

        case MOUSE_WHEEL_EVENT_COMMAND: {
            *ReplySize = sizeof(bool);
            if (!Input)
            {
                *((bool *) ReplyBuffer) = false;
                return;
            }
            std::cout << "Got a key event message from the pipe!" << std::endl;
            auto Event = (MouseWheelEvent*) Arguments;
            *((bool *) ReplyBuffer) = Input->MouseWheelEvent(Event->ID, Event->When, Event->Modifiers, Event->X,
                                                  Event->Y, Event->ClickCount, Event->PopupTrigger, Event->ScrollType,
                                                  Event->ScrollAmount, Event->WheelRotation);
            return;
        }

        case GET_SURFACE_INFO_COMMAND: {
            *ReplySize = sizeof(ClientSurfaceInfo);
            if (!Input) {
                ClientSurfaceInfo SurfaceInfo{};
                memcpy(ReplyBuffer, &SurfaceInfo, sizeof(ClientSurfaceInfo));
            } else {
                memcpy(ReplyBuffer, Input->GetClientSurfaceInfo(), sizeof(ClientSurfaceInfo));
            }
            return;
        }

        case REQUEST_PIXEL_BUFFER_COMMAND: {
            *ReplySize = sizeof(bool);
            if (!Input) {
                *((bool *) ReplyBuffer) = false;
                return;
            }
            *((bool *) ReplyBuffer) = Input->RequestPixelBufferUpdate();
            return;
        }

        default:
            std::cout << "Unknown command" << std::endl;
            *ReplySize = 0;
            return;
    }
}

KInputControllerListener::~KInputControllerListener()
{
    // Signal the thread to stop
    this->Running = false;
    std::cout << "Flagged Communication thread to stop!" << std::endl;
    // Close the pipe
    if (this->Pipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(this->Pipe);
        std::cout << "Disconnected the pipe!" << std::endl;
        CloseHandle(this->Pipe);
        std::cout << "Closed the pipe!" << std::endl;
    }
    // Join the thread
    if (CommunicationThread.joinable())
    {
        std::cout << "Joining the thread!" << std::endl;
        // Join is hanging even when the thread last statement is reached (try a print!)
        // Detach seems to do the job when cleaning up the thread (ProcessExplorer counts a thread dying)
        CommunicationThread.detach();
    }
    std::cout << "Done!!!!" << std::endl;
}
