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

#ifndef KINPUT_KINPUTCONTROLLERLISTENER_H
#define KINPUT_KINPUTCONTROLLERLISTENER_H

#include <thread>
#include "../Shared/Definitions.h"
#include "KInput.hpp"

class KInputControllerListener
{
private:
    KInput *Input;

    // Communication related
    bool Running;
    std::thread CommunicationThread;
    HANDLE Pipe;

public:
    explicit KInputControllerListener(KInput *Input) : Input(Input), Running(false), Pipe(INVALID_HANDLE_VALUE)
    {}

    bool Start();

    void CommunicationListener();

    void ProcessCommand(void *CommandBuffer, DWORD CommandSize, void *ReplyBuffer, LPDWORD ReplySize);

    ~KInputControllerListener();
};


#endif //KINPUT_KINPUTCONTROLLERLISTENER_H
