#pragma once

#include <DirectXMath.h>
#include "InputManager.h"

using namespace DirectX;

namespace mc
{
    class Ship;

    class Camera
    {
    public:
        Camera(const XMFLOAT3& position, float nearPlane,
               float farPlane,
               float fovMin, float fovMax,
               float aspectRation);
        void Update(const InputManager& im, float dt);
        void FollowShip(const Ship& ship);
        void TargteShip(const Ship& ship);

        const XMMATRIX& GetViewMat();
        const XMFLOAT3& GetPosition();
        XMFLOAT3 GetTarget();
        void SetPosition(const XMFLOAT3& position);
        XMVECTOR GetRight() const { return right_; }
        XMVECTOR GetFront() const { return front_; }
        XMVECTOR GetUp() const { return up_; }

        float GetNearPlane() { return nearPlane_; }
        float GetFarPlane() { return farPlane_; }
        float GetFovMin() { return fovMin_; }
        float GetFovMax() { return fovMax_; }
        float GetAspectRatio() { return aspectRation_; }

    private:

        void CalculateViewMat();

        XMFLOAT3 position_;
        XMFLOAT3 rot_;

        XMVECTOR right_;
        XMVECTOR up_;
        XMVECTOR front_;
        XMVECTOR worldUp_;
        XMMATRIX view_;

        float nearPlane_;
        float farPlane_;
        float fovMin_;
        float fovMax_;
        float aspectRation_;

    };
}

