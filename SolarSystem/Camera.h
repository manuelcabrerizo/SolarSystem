#pragma once

#include <DirectXMath.h>
#include "InputManager.h"

using namespace DirectX;

namespace mc
{
    class Camera
    {
    public:
        Camera(const XMFLOAT3& position);
        void Update(const InputManager& im,float dt);
        const XMMATRIX& GetViewMat();
        const XMFLOAT3& GetPosition();
        XMFLOAT3 GetTarget();
        void SetPosition(const XMFLOAT3& position);
    private:

        void CalculateViewMat();

        XMFLOAT3 position_;
        XMFLOAT3 rot_;

        XMVECTOR right_;
        XMVECTOR up_;
        XMVECTOR front_;
        XMVECTOR worldUp_;
        XMMATRIX view_;
    };
}

