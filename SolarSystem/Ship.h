#pragma once

#include <DirectXMath.h>
#include "InputManager.h"

using namespace DirectX;

namespace mc
{
    struct CollisionData;

    class Ship
    {
    public:
        Ship(const XMFLOAT3& position, float mass, float radio);
        void Update(
            const InputManager& im, float dt,
            CollisionData* collisionDataArray[],
            unsigned int collisionDataCount);
        XMFLOAT3 GetPosition() const;
        XMVECTOR GetOrientation() const;
        XMVECTOR GetForward() const { return forward_; }
        XMVECTOR GetFront() const { return front_; }
        XMVECTOR GetUp() const { return up_; }
        XMVECTOR GetRight() const { return right_; }
        XMVECTOR GetVelocity() const { return vel_; }
        float GetThrust() const { return thrustMagnitude_; }
        float GetThrustMax() const { return thrustMax_; }

    private:
        void ProcessInput(const InputManager& im, float dt);
        void ProcessVelocities(float dt);
        void ProcessCollision(CollisionData* collisionData, float dt);

        XMVECTOR pos_{};
        XMVECTOR vel_{};
        XMVECTOR acc_{};

        XMVECTOR thrust_{};
        float damping_{ 0.05f };

        XMVECTOR right_{};
        XMVECTOR up_{};
        XMVECTOR front_{};

        XMVECTOR forward_{};
        XMVECTOR worldUp_{};

        float mass_{};
        float radio_{};
        float thrustMagnitude_{ 0.0f };
        float thrustMax_{ 6.4f };

        float yawVel_{};
        float rollVel_{};
    };
}

