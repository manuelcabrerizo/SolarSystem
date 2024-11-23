#pragma once

#include <windows.h>
#include <string>

namespace mc
{
    class InputManager;

    class Window
    {
    public:
        Window(const std::string& title, int width, int height, std::size_t userData);
        ~Window();
        void ProcessEvents();

        HWND Get() const { return hwnd_; }
        int Width() const { return width_; }
        int Height() const { return height_; }

    private:
        HWND hwnd_;
        int width_;
        int height_;
    };
}

