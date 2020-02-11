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

#ifndef KINPUT_DEFINITIONS_H
#define KINPUT_DEFINITIONS_H

#define BUFF_SIZE 512
#define PIPE_PREFIX "KInput_"

#define FOCUS_EVENT_COMMAND 1
#define KEY_EVENT_COMMAND 2
#define MOUSE_EVENT_COMMAND 3
#define MOUSE_WHEEL_EVENT_COMMAND 4
#define GET_SURFACE_INFO_COMMAND 5
#define REQUEST_PIXEL_BUFFER_COMMAND 6

union BGRA
{ // Contains pixel info in BGRA format
    struct
    {
        char B, G, R, A;
    };
    unsigned int Color;
};

struct ClientSurfaceInfo
{
    int Width = -1;
    int Height = -1;
    const BGRA *PixelBuffer = nullptr; // Contains pixel info in BGRA format

public:
    size_t GetPixelBufferSize()
    {
        return Width * Height * sizeof(BGRA);
    }
};

struct FocusEvent
{
    std::int32_t ID;
};

struct KeyEvent
{
    std::int32_t ID;
    std::int64_t When;
    std::int32_t Modifiers;
    std::int32_t KeyCode;
    std::uint16_t KeyChar;
    std::int32_t KeyLocation;
};

struct MouseEvent
{
    std::int32_t ID;
    std::int64_t When;
    std::int32_t Modifiers;
    std::int32_t X;
    std::int32_t Y;
    std::int32_t ClickCount;
    bool PopupTrigger;
    std::int32_t Button;
};

struct MouseWheelEvent
{
    std::int32_t ID;
    std::int64_t When;
    std::int32_t Modifiers;
    std::int32_t X;
    std::int32_t Y;
    std::int32_t ClickCount;
    bool PopupTrigger;
    std::int32_t ScrollType;
    std::int32_t ScrollAmount;
    std::int32_t WheelRotation;
};

#endif //KINPUT_DEFINITIONS_H
