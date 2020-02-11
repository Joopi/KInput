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

#ifndef PINPUT_HPP_INCLUDED
#define PINPUT_HPP_INCLUDED

#include <queue>
#include <set>
#include <thread>
#include <mutex>
#include "KInputCtrl.hpp"

/**
 * Java constants
 */

enum Events
{
    KEY_TYPED = 400,
    KEY_PRESSED = 401,
    KEY_RELEASED = 402
};

enum KeyEvents
{
    CHAR_UNDEFINED = 65535,
    KEY_LOCATION_LEFT = 2,
    KEY_LOCATION_NUMPAD = 4,
    KEY_LOCATION_RIGHT = 3,
    KEY_LOCATION_STANDARD = 1,
    KEY_LOCATION_UNKNOWN = 0,
    VK_ALT = 18,
};

enum InputEvents
{
    ALT_MASK = 8,
    SHIFT_DOWN_MASK = 64,
    CTRL_DOWN_MASK = 128,
    ALT_DOWN_MASK = 512,
    SHIFT_MASK = 1,
    BUTTON1_DOWN_MASK = 1024,
    BUTTON2_DOWN_MASK = 2048,
    BUTTON3_DOWN_MASK = 4096,
    META_DOWN_MASK = 256,
    CTRL_MASK = 2,
};

enum FocusEvents
{
    FOCUS_GAINED = 1004,
    FOCUS_LOST = 1005,
};

enum MouseEvents
{
    BUTTON1 = 1,
    BUTTON2 = 2,
    BUTTON3 = 3,
    MOUSE_PRESSED = 501,
    MOUSE_DRAGGED = 506,
    MOUSE_CLICKED = 500,
    MOUSE_MOVEDD = 503,
    MOUSE_RELEASED = 502,
    MOUSE_WHEEL = 507,
};

enum MouseWheelEvents
{
    WHEEL_UNIT_SCROLL = 0,
};

class HeldKey
{
private:
    std::int32_t KeyCode;
    std::int64_t SentTime;
public:
    HeldKey(int32_t KeyCode, int64_t SentTime) : KeyCode(KeyCode), SentTime(SentTime)
    {}

    explicit HeldKey(int32_t KeyCode) : HeldKey(KeyCode, 0)
    {}

    bool operator<(const HeldKey &rhs) const
    {
        return getKeyCode() < rhs.getKeyCode();
    }

    bool operator>(const HeldKey &rhs) const
    {
        return rhs < *this;
    }

    bool operator<=(const HeldKey &rhs) const
    {
        return !(rhs < *this);
    }

    bool operator>=(const HeldKey &rhs) const
    {
        return !(*this < rhs);
    }

    int32_t getKeyCode() const
    {
        return KeyCode;
    }

    int64_t getSentTime() const
    {
        return SentTime;
    }
};

class PInput
{
private:
    // Keyboard
    std::set<HeldKey> KeysHeld;
    std::mutex KeysHeldLock;
    std::thread KeySenderThread;
    bool RunningKeySender;

    // Focus
    bool Focused;

    // Mouse
    bool LeftDown;
    bool MidDown;
    bool RightDown;
    std::int32_t X, Y;
private:
    void SetKeyDown(HeldKey Key, bool Held);

    void KeySenderRun();

public:
    KInputCtrl *KInput;

    PInput(DWORD PID, std::string &Path);

    // Keyboard
    bool PressKey(std::int32_t KeyCode);

    bool IsKeyDown(std::int32_t KeyCode);

    bool KeyUp(std::int32_t KeyCode);

    bool KeyDown(std::int32_t KeyCode);

    bool SendKeys(std::string Text, std::int32_t KeyWait, std::int32_t KeyModWait);

    std::int32_t GetKeysModifiers();

    // Focus
    bool IsFocused();

    bool GainFocus();

    bool LoseFocus(bool AltTab);

    // Mouse
    bool GetMousePos(std::int32_t *X, std::int32_t *Y);

    bool IsMouseButtonDown(std::int32_t ClickType);

    bool IsDragging();

    bool MoveMouse(std::int32_t X, std::int32_t Y);

    bool HoldMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType);

    bool ReleaseMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType);

    bool ClickMouse(std::int32_t X, std::int32_t Y, std::int32_t ClickType);

    bool ScrollMouse(std::int32_t X, std::int32_t Y, std::int32_t Lines);

    std::int32_t GetMouseModifiers(std::int32_t Button = -1);

    ~PInput();
};

#endif //PINPUT_HPP_INCLUDED