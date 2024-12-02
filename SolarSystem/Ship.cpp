#include "Ship.h"
#include <utility>
#include <iostream>
namespace mc
{
    Ship::Ship(const XMFLOAT3& position, float mass, float radio)
        : pos_(XMLoadFloat3(&position)), mass_(mass), radio_(radio)
    {
        right_ = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        up_ = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        front_ = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }

    void Ship::ProcessInput(const InputManager& im, float dt)
    {
        float rotationSpeed = 6.0f;
        if (im.KeyDown(mc::KEY_Q))
        {
            yawVel_ -= rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_E))
        {
            yawVel_ += rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_W))
        {
            pitchVel_ += rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_S))
        {
            pitchVel_ -= rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_A))
        {
            rollVel_ += rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_D))
        {
            rollVel_ -= rotationSpeed * dt;
        }

        if (im.KeyDown(mc::KEY_SPACE))
        {
            thrustMagnitude_ = std::min(thrustMagnitude_ + (100.0f * dt), thrustMax_);
        }
        else
        {
            thrustMagnitude_ = std::max(thrustMagnitude_ - (200.0f * dt), 0.0f);
        }
    }


    void Ship::Update(const InputManager& im, float dt)
    {
        ProcessInput(im, dt);

        if (pitchVel_ != 0.0f)
        {
            front_ = XMVector3Rotate(front_, XMQuaternionRotationAxis(right_, pitchVel_ * dt));
            up_ = XMVector3Normalize(XMVector3Cross(front_, right_));
        }
        if (yawVel_ != 0.0f)
        {
            front_ = XMVector3Rotate(front_, XMQuaternionRotationAxis(up_, yawVel_ * dt));
            right_ = XMVector3Normalize(XMVector3Cross(up_, front_));
        }
        if (rollVel_ != 0.0f)
        {
            right_ = XMVector3Rotate(right_, XMQuaternionRotationAxis(front_, rollVel_ * dt));
            up_ = XMVector3Normalize(XMVector3Cross(front_, right_));
        }
        
        thrust_ = front_ * (thrustMagnitude_ * dt);

        XMVECTOR noseVel = front_ * XMVector3Length(vel_);
        
        XMVECTOR force = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        force += thrust_;

        acc_ = noseVel - vel_;

        acc_ = force / mass_;

        vel_ += acc_ * dt;
        pos_ += vel_ * dt;

        float damping = std::powf(damping_, dt);
        vel_ *= damping;
        pitchVel_ *= damping;
        yawVel_ *= damping;
        rollVel_ *= damping;

    }

    XMFLOAT3 Ship::GetPosition() const
    {
        XMFLOAT3 position;
        XMStoreFloat3(&position, pos_);
        return position;
    }

    XMVECTOR Ship::GetOrientation() const
    {
        XMFLOAT3 x, y, z;
        XMStoreFloat3(&x, right_);
        XMStoreFloat3(&y, up_);
        XMStoreFloat3(&z, front_);
        XMMATRIX rotMat = XMMatrixSet(
            x.x,  x.y,  x.z, 0.0f,
            y.x,  y.y,  y.z, 0.0f,
            z.x,  z.y,  z.z, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
        return XMQuaternionMultiply(XMQuaternionRotationRollPitchYaw(0.0f, XM_PI, 0.0f),
            XMQuaternionRotationMatrix(rotMat));
    }

}