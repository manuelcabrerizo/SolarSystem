#pragma once


#include <DirectXMath.h>
using namespace DirectX;

namespace mc
{
    struct ObjectConstBuffer
    {
        XMMATRIX model;

    };
    struct CameraConstBuffer
    {
        XMMATRIX view;
        XMMATRIX proj;
        XMFLOAT3 viewPos;
        float pad;
    };

    struct PointLight
    {
        XMFLOAT3 position;
        float constant;
        XMFLOAT3 ambient;
        float linear;
        XMFLOAT3 diffuse;
        float quadratic;
        XMFLOAT3 specular;
        float pad0;
    };

    struct LightConstBuffer
    {
        PointLight lights[4];
        int count;
        XMFLOAT3 viewPos;
    };

    struct CommonConstBuffer
    {
        XMFLOAT2 resolution;
        float time;
        float pad0;
        XMFLOAT2 screenPos;
        XMFLOAT2 flerActive;
    };
}
