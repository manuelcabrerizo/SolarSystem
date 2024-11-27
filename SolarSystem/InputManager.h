#pragma once

namespace mc
{
    constexpr unsigned int KEY_COUNT = 256;
    constexpr unsigned int MOUSE_BUTTON_COUNT = 3;

    class InputManager
    {
    public:
        InputManager();
        ~InputManager();
        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        bool KeyDown(unsigned int key) const;
        bool KeyJustDown(unsigned int key) const;
        bool KeyJustUp(unsigned int key) const;
        bool MouseButtonDown(unsigned int button) const;
        bool MouseButtonJustDown(unsigned int button) const;
        bool MouseButtonJustUp(unsigned int button) const;

        void Process();
        void SetKey(unsigned int key, bool value);
        void SetMouseButton(unsigned int button, bool value);
    private:
        bool keys_[2][KEY_COUNT];
        bool mouseButtons_[2][MOUSE_BUTTON_COUNT];
        int mousePosX;
        int mousePosY;
    };

    constexpr unsigned int KEY_ESCAPE = 0x1B;
    constexpr unsigned int KEY_0 = 0x30;
    constexpr unsigned int KEY_1 = 0x31;
    constexpr unsigned int KEY_2 = 0x32;
    constexpr unsigned int KEY_3 = 0x33;
    constexpr unsigned int KEY_4 = 0x34;
    constexpr unsigned int KEY_5 = 0x35;
    constexpr unsigned int KEY_6 = 0x36;
    constexpr unsigned int KEY_7 = 0x37;
    constexpr unsigned int KEY_8 = 0x38;
    constexpr unsigned int KEY_9 = 0x39;
    constexpr unsigned int KEY_A = 0x41;
    constexpr unsigned int KEY_B = 0x42;
    constexpr unsigned int KEY_C = 0x43;
    constexpr unsigned int KEY_D = 0x44;
    constexpr unsigned int KEY_E = 0x45;
    constexpr unsigned int KEY_F = 0x46;
    constexpr unsigned int KEY_G = 0x47;
    constexpr unsigned int KEY_H = 0x48;
    constexpr unsigned int KEY_I = 0x49;
    constexpr unsigned int KEY_J = 0x4A;
    constexpr unsigned int KEY_K = 0x4B;
    constexpr unsigned int KEY_L = 0x4C;
    constexpr unsigned int KEY_M = 0x4D;
    constexpr unsigned int KEY_N = 0x4E;
    constexpr unsigned int KEY_O = 0x4F;
    constexpr unsigned int KEY_P = 0x50;
    constexpr unsigned int KEY_Q = 0x51;
    constexpr unsigned int KEY_R = 0x52;
    constexpr unsigned int KEY_S = 0x53;
    constexpr unsigned int KEY_T = 0x54;
    constexpr unsigned int KEY_U = 0x55;
    constexpr unsigned int KEY_V = 0x56;
    constexpr unsigned int KEY_W = 0x57;
    constexpr unsigned int KEY_X = 0x58;
    constexpr unsigned int KEY_Y = 0x59;
    constexpr unsigned int KEY_Z = 0x5A;
    constexpr unsigned int KEY_NUMPAD0 = 0x60;
    constexpr unsigned int KEY_NUMPAD1 = 0x61;
    constexpr unsigned int KEY_NUMPAD2 = 0x62;
    constexpr unsigned int KEY_NUMPAD3 = 0x63;
    constexpr unsigned int KEY_NUMPAD4 = 0x64;
    constexpr unsigned int KEY_NUMPAD5 = 0x65;
    constexpr unsigned int KEY_NUMPAD6 = 0x66;
    constexpr unsigned int KEY_NUMPAD7 = 0x67;
    constexpr unsigned int KEY_NUMPAD8 = 0x68;
    constexpr unsigned int KEY_NUMPAD9 = 0x69;
    constexpr unsigned int KEY_RETURN = 0x0D;
    constexpr unsigned int KEY_SPACE = 0x20;
    constexpr unsigned int KEY_TAB = 0x09;
    constexpr unsigned int KEY_CONTROL = 0x11;
    constexpr unsigned int KEY_SHIFT = 0x10;
    constexpr unsigned int KEY_ALT = 0x12;
    constexpr unsigned int KEY_CAPS = 0x14;
    constexpr unsigned int KEY_LEFT = 0x25;
    constexpr unsigned int KEY_UP = 0x26;
    constexpr unsigned int KEY_RIGHT = 0x27;
    constexpr unsigned int KEY_DOWN = 0x28;

    constexpr unsigned int MOUSE_BUTTON_LEFT = 0;
    constexpr unsigned int MOUSE_BUTTON_MIDDLE = 1;
    constexpr unsigned int MOUSE_BUTTON_RIGHT = 2;
}

