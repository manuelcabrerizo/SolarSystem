#include "InputManager.h"

#include <memory>

namespace mc
{

    InputManager::InputManager()
        : keys_{}, mouseButtons_{}
    {
    }

    InputManager::~InputManager()
    {
    }

    bool InputManager::KeyDown(unsigned int key) const
    {
        return keys_[0][key];
    }

    bool InputManager::KeyJustDown(unsigned int key) const
    {
        return keys_[0][key] && !keys_[1][key];
    }

    bool InputManager::KeyJustUp(unsigned int key) const
    {
        return !keys_[0][key] && keys_[1][key];
    }

    bool InputManager::MouseButtonDown(unsigned int button) const
    {
        return mouseButtons_[0][button];
    }

    bool InputManager::MouseButtonJustDown(unsigned int button) const
    {
        return mouseButtons_[0][button] && !mouseButtons_[1][button];
    }

    bool InputManager::MouseButtonJustUp(unsigned int button) const
    {
        return !mouseButtons_[0][button] && mouseButtons_[1][button];
    }

    void InputManager::Process()
    {
        std::memcpy(keys_[1], keys_[0], KEY_COUNT * sizeof(bool));
        std::memcpy(mouseButtons_[1], mouseButtons_[0], MOUSE_BUTTON_COUNT * sizeof(bool));
    }

    void InputManager::SetKey(unsigned int key, bool value)
    {
        keys_[0][key] = value;
    }

    void InputManager::SetMouseButton(unsigned int button, bool value)
    {
        mouseButtons_[0][button] = value;
    }
}