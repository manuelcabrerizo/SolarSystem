#include "Window.h"
#include <stdexcept>

#include "Engine.h"


namespace mc
{
    LRESULT CALLBACK Wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    Window::Window(const std::string& title, int width, int height, std::size_t userData)
    {
        HINSTANCE hInstance = GetModuleHandleA(0);
        
        width_ = width;
        height_ = height;

        WNDCLASSEX wndClass{};
        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = Wndproc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = sizeof(std::size_t);
        wndClass.hInstance = hInstance;
        wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszMenuName = nullptr;
        wndClass.lpszClassName = "MCEngine";
        wndClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

        if (!RegisterClassEx(&wndClass))
        {
            throw std::runtime_error("Error registering the window class.");
        }

        // TODO: when we have input check that the size of the window is correct.
        RECT desiredClientArea = { 0, 0, width_, height_ };
        AdjustWindowRectEx(&desiredClientArea, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, false, WS_EX_OVERLAPPEDWINDOW);
        int windowWidth = desiredClientArea.right - desiredClientArea.left;
        int windowHeight = desiredClientArea.bottom - desiredClientArea.top;
        hwnd_ = CreateWindowEx(
            WS_EX_OVERLAPPEDWINDOW, "MCEngine", title.c_str(), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);
        if (!hwnd_)
        {
            throw std::runtime_error("Error creating the window.");
        }

        SetWindowLongPtrA(hwnd_, GWLP_USERDATA, (LONG_PTR)userData);

        ShowWindow(hwnd_, true);
    }

    Window::~Window()
    {
        if (hwnd_)
        {
            DestroyWindow(hwnd_);
        }
    }

    void Window::ProcessEvents()
    {
        InputManager* im = reinterpret_cast<InputManager *>(GetWindowLongPtrA(hwnd_, GWLP_USERDATA));
        if (im)
        {
            im->Process();
        }

        MSG msg{};
        while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE) != 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    LRESULT CALLBACK Wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        InputManager* im = reinterpret_cast<InputManager*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

        LRESULT result = 0;
        switch (msg)
        {
        case WM_CLOSE: Engine::isRunning = false; break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            DWORD keyCode = (DWORD)wParam;
            bool isDown = ((lParam & (1 << 31)) == 0);
            if (im)
            {
                im->SetKey(keyCode, isDown);
            }
        } break;

        default: result = DefWindowProc(hwnd, msg, wParam, lParam); break;
        }
        return result;
    }
}

