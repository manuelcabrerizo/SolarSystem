#include "Camera.h"
#include "Ship.h"
namespace mc
{
    Camera::Camera(const XMFLOAT3& position,
                   float nearPlane, float farPlane,
                   float fovMin, float fovMax,
                   float aspectRation)
        : position_(position), rot_(XMFLOAT3(0.0f, 0.f,0.0f)),
          nearPlane_(nearPlane),  farPlane_(farPlane), fovMin_(fovMin), fovMax_(fovMax),
          aspectRation_(aspectRation)
    {
        worldUp_ = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        front_ = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        CalculateViewMat();
    }

    void Camera::Update(const InputManager& im, float dt)
    {
        // TODO: camera input
        float speed = 0.5f;
        XMVECTOR pos = XMLoadFloat3(&position_);
        if (im.KeyDown(mc::KEY_D))
        {
            pos += right_ * speed * dt;
        }
        if (im.KeyDown(mc::KEY_A))
        {
            pos -= right_ * speed * dt;
        }
        if (im.KeyDown(mc::KEY_W))
        {
            pos += front_ * speed * dt;
        }
        if (im.KeyDown(mc::KEY_S))
        {
            pos -= front_ * speed * dt;
        }
        XMStoreFloat3(&position_, pos);

        float rotationSpeed = 1.0f;
        if (im.KeyDown(mc::KEY_RIGHT))
        {
            rot_.y += rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_LEFT))
        {
            rot_.y -= rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_UP))
        {
            rot_.x -= rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_DOWN))
        {
            rot_.x += rotationSpeed * dt;
        }
        if (rot_.x > (89.0f / 180.0f) * XM_PI)
            rot_.x = (89.0f / 180.0f) * XM_PI;
        if (rot_.x < -(89.0f / 180.0f) * XM_PI)
            rot_.x = -(89.0f / 180.0f) * XM_PI;

        CalculateViewMat();
    }

    void Camera::FollowShip(const Ship& ship)
    {
        // TODO: fix this
        XMFLOAT3 position = ship.GetPosition();
        XMVECTOR shipPos = XMLoadFloat3(&position);
        XMVECTOR offset = worldUp_ * 0.125f;

        XMVECTOR pos = shipPos - (ship.GetForward() * 0.25f) + offset;
        XMStoreFloat3(&position_, pos);

        front_ = XMVector3Normalize(shipPos - pos);
        right_ = XMVector3Normalize(XMVector3Cross(worldUp_, front_));
        up_ = XMVector3Normalize(XMVector3Cross(front_, right_));
        view_ = XMMatrixLookAtLH(pos, pos + front_, up_);
    }

    void Camera::TargteShip(const Ship& ship)
    {
        XMFLOAT3 shipPosition = ship.GetPosition();
        XMVECTOR shipPos = XMLoadFloat3(&shipPosition);
        XMVECTOR position = XMLoadFloat3(&position_);
        front_ = XMVector3Normalize(shipPos - position);
        right_ = XMVector3Normalize(XMVector3Cross(worldUp_, front_));
        up_ = XMVector3Normalize(XMVector3Cross(front_, right_));
        view_ = XMMatrixLookAtLH(position, shipPos, worldUp_);
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