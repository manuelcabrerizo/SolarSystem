#pragma once

#include <DirectXMath.h>
#include "InputManager.h"

using namespace DirectX;

namespace mc
{
    class Ship
    {
    public:
        Ship(const XMFLOAT3& position, float mass, float radio);
        void Update(const InputManager& im, float dt);
        XMFLOAT3 GetPosition() const;
        XMVECTOR GetOrientation() const;
        XMVECTOR GetFront() const { return front_; }
        XMVECTOR GetUp() const { return up_; }
        XMVECTOR GetRight() const { return right_; }
        XMVECTOR GetVelocity() const { return vel_; }
    private:
        void ProcessInput(const InputManager& im, float dt);

        XMVECTOR pos_{};
        XMVECTOR vel_{};
        XMVECTOR acc_{};

        XMVECTOR thrust_{};
        float damping_{ 0.05f };

        XMVECTOR right_{};
        XMVECTOR up_{};
        XMVECTOR front_{};

        float mass_{};
        float radio_{};
        float thrustMagnitude_{ 0.0f };
        float thrustMax_{ 400.0f };

        float pitchVel_{};
        float yawVel_{};
        float rollVel_{};
    };
}

