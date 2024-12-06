#include "Ship.h"
#include "GeometryGenerator.h"

#include <utility>
#include <iostream>
#include <algorithm>
namespace mc
{
    Ship::Ship(const XMFLOAT3& position, float mass, float radio)
        : pos_(XMLoadFloat3(&position)), mass_(mass), radio_(radio)
    {
        right_ = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
        up_ = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        front_ = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        forward_ = front_;
        worldUp_ = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    }

    void Ship::ProcessInput(const InputManager& im, float dt)
    {
        float rotationSpeed = 6.0f;
        if (im.KeyDown(mc::KEY_A))
        {
            yawVel_ -= rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_D))
        {
            yawVel_ += rotationSpeed * dt;
        }
        if (im.KeyDown(mc::KEY_W))
        {
            thrustMagnitude_ = std::min(thrustMagnitude_ + (100.0f*0.016f*dt), thrustMax_);
        }
        else
        {
            thrustMagnitude_ = std::max(thrustMagnitude_ - (200.0f*0.016f*dt), 0.0f);
        }
    }

    void Ship::ProcessVelocities(float dt)
    {
        rollVel_ = yawVel_*-0.25f;
        forward_ = XMVector3Normalize(XMVector3Rotate(forward_, XMQuaternionRotationAxis(worldUp_, yawVel_ * dt)));
        XMVECTOR worldRight = XMVector3Cross(worldUp_, forward_);
        front_ = XMVector3Rotate(front_, XMQuaternionRotationAxis(worldUp_, yawVel_ * dt));
        right_ = XMVector3Rotate(worldRight, XMQuaternionRotationAxis(forward_, rollVel_));
        up_ = XMVector3Normalize(XMVector3Cross(front_, right_));

        thrust_ = forward_ * thrustMagnitude_;

        XMVECTOR noseVel = forward_ * XMVector3Length(vel_);

        XMVECTOR force = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        force += thrust_;

        acc_ = noseVel - vel_;
        acc_ = force / mass_;

        vel_ += acc_ * dt;
        pos_ += vel_ * dt;

        float damping = std::powf(damping_, dt);
        vel_ *= damping;
        yawVel_ *= damping;
        rollVel_ *= damping;
    }

    void Ship::ProcessCollision(CollisionData* collisionData, float dt)
    {
        for (size_t i = 0; i < collisionData->quads.size(); i++)
        { 
            const CollisionQuad& quad = collisionData->quads[i];
            // TODO: store the collision quad directly using XMVECTOR
            XMVECTOR a = XMLoadFloat3(&quad.vertices[0]);
            XMVECTOR b = XMLoadFloat3(&quad.vertices[1]);
            XMVECTOR c = XMLoadFloat3(&quad.vertices[2]);
            XMVECTOR d = XMLoadFloat3(&quad.vertices[3]);
            XMVECTOR n = XMLoadFloat3(&quad.normal);

            float width = XMVectorGetX(XMVector3Length(b - a));
            float height = XMVectorGetX(XMVector3Length(d - a));

            // first we have to get a basis for this quad
            XMVECTOR origin = XMVectorSet(quad.vertices[0].x, quad.vertices[0].y, quad.vertices[0].z, 1.0f) + (n * radio_);
            XMVECTOR right = XMVector3Normalize(b - a);
            XMVECTOR up = n;
            XMVECTOR front = XMVector3Normalize(d - a);
            XMMATRIX basisMatrix = XMMATRIX(right, up, front, origin);

            XMVECTOR relPos_ = XMVector3Transform(pos_, XMMatrixInverse(nullptr, basisMatrix));
            XMFLOAT3 relPos;
            XMStoreFloat3(&relPos, relPos_);
            float penetration = relPos.y;
            if (penetration <= 0) // posible collision
            {
                penetration = std::fabsf(penetration);
                if (penetration > 1.0f)
                {
                    continue;
                } 
                XMVECTOR contactPoint_ = relPos_ + (XMVectorSet(0.0f, 1.0f, 0.0f, .0f) * penetration);;
                XMFLOAT3 contactPoint;
                XMStoreFloat3(&contactPoint, contactPoint_);
                if (contactPoint.x >= 0 && contactPoint.x <= width)
                {
                    XMVECTOR worldContactPoint = XMVector3Transform(contactPoint_, basisMatrix);
                    pos_ = worldContactPoint + n * 0.001f;
                    vel_ = vel_ - n * XMVector3Dot(vel_, n);
                }
            }
        }
    }

    void Ship::Update(
        const InputManager& im, float dt,
        CollisionData* collisionDataArray[],
        unsigned int collisionDataCount)
    {
        ProcessInput(im, dt);
        ProcessVelocities(dt);
        for (unsigned int i = 0; i < collisionDataCount; i++)
        {
            ProcessCollision(collisionDataArray[i], dt);
        }
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