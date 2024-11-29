#include "Camera.h"

namespace mc
{
    Camera::Camera(const XMFLOAT3& position)
        : position_(position), rot_(XMFLOAT3(0.0f, 0.f,0.0f))
    {
        worldUp_ = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        front_ = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        CalculateViewMat();
    }

    void Camera::Update(const InputManager& im, float dt)
    {
        // TODO: camera input
        XMVECTOR pos = XMLoadFloat3(&position_);
        if (im.KeyDown(mc::KEY_D))
        {
            pos += right_ * 0.1f;
        }
        if (im.KeyDown(mc::KEY_A))
        {
            pos -= right_ * 0.1f;
        }
        if (im.KeyDown(mc::KEY_W))
        {
            pos += front_ * 0.1f;
        }
        if (im.KeyDown(mc::KEY_S))
        {
            pos -= front_ * 0.1f;
        }
        XMStoreFloat3(&position_, pos);

        if (im.KeyDown(mc::KEY_RIGHT))
        {
            rot_.y += 0.025f;
        }
        if (im.KeyDown(mc::KEY_LEFT))
        {
            rot_.y -= 0.025f;
        }
        if (im.KeyDown(mc::KEY_UP))
        {
            rot_.x += 0.025f;
        }
        if (im.KeyDown(mc::KEY_DOWN))
        {
            rot_.x -= 0.025f;
        }
        if (rot_.x > (89.0f / 180.0f) * XM_PI)
            rot_.x = (89.0f / 180.0f) * XM_PI;
        if (rot_.x < -(89.0f / 180.0f) * XM_PI)
            rot_.x = -(89.0f / 180.0f) * XM_PI;

        CalculateViewMat();
    }

    const XMMATRIX& Camera::GetViewMat()
    {
        return view_;
    }

    const XMFLOAT3& Camera::GetPosition()
    {
        return position_;
    }

    XMFLOAT3 Camera::GetTarget()
    {        
        XMFLOAT3 target;
        XMStoreFloat3(&target, XMLoadFloat3(&position_) + front_);
        return target;
    }

    void Camera::SetPosition(const XMFLOAT3& position)
    {
        position_ = position;
    }


    void Camera::CalculateViewMat()
    {
        XMVECTOR pos = XMLoadFloat3(&position_);
        front_ = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        front_ = XMVector3TransformNormal(front_, XMMatrixRotationX(rot_.x));
        front_ = XMVector3Normalize(front_);
        front_ = XMVector3TransformNormal(front_, XMMatrixRotationY(rot_.y));
        front_ = XMVector3Normalize(front_);
        right_ = XMVector3Normalize(XMVector3Cross(worldUp_, front_));
        up_ = XMVector3Cross(front_, right_);
        view_ = XMMatrixLookAtLH(pos, pos + front_, worldUp_);
    }
}