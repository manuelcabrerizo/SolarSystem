Texture2D srv : register(t0);
Texture2D bloom : register(t1);
SamplerState samplerState : register(s0);

#define PI 3.14159265359
#define TAU 6.28318530718

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 fragPos : TEXCOORD2;

};

struct PointLight
{
    float3 position_;
    float constant_;
    float3 ambient_;
    float linear_;
    float3 diffuse_;
    float quadratic_;
    float3 specular_;
    float pad0_;
};

cbuffer LightConstBuffer : register(b2)
{
    PointLight lights[4];
    int lightCount;
    float3 viewPos;
};

cbuffer Common : register(b3)
{
    float2 resolution;
    float time;
    float pad0;
    float2 screenPos;
    float2 flerActive;
}

float random(in float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}
float noise(in float2 st)
{
    float2 i = floor(st);
    float2 f = frac(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return lerp(a, b, u.x) +
            (c - a) * u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}
float3 lensflare(float2 uv, float2 pos)
{
    float2 main = uv - pos;
    float2 uvd = uv * (length(uv));
	
    float ang = atan2(main.x, main.y);
    float dist = length(main);
    dist = pow(dist, 0.1);
	
    float n = noise(float2(ang * 16.0, dist * 32.0));
		
    float f1 = max(0.01 - pow(length(uv + 1.2 * pos), 1.9), .0) * 7.0;

    float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), .0) * 0.25;
    float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), .0) * 0.23;
    float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), .0) * 0.21;
	
    float2 uvx = lerp(uv, uvd, -0.5);
	
    float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), .0) * 6.0;
    float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), .0) * 5.0;
    float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), .0) * 3.0;
	
    uvx = lerp(uv, uvd, -.4);
	
    float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), .0) * 2.0;
    float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), .0) * 2.0;
    float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), .0) * 2.0;
	
    uvx = lerp(uv, uvd, -0.5);
	
    float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), .0) * 6.0;
    float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), .0) * 3.0;
    float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), .0) * 5.0;
	
    float3 c = float3(0.0, 0.0, 0.0);
	
    c.r += f2 + f4 + f5 + f6;
    c.g += f22 + f42 + f52 + f62;
    c.b += f23 + f43 + f53 + f63;
    float len = length(uvd) * .05;
    c = c * 1.3 - float3(len, len, len);

    return c;
}
float3 cc(float3 color, float factor, float factor2) // color modifier
{
    float w = color.x + color.y + color.z;
    return lerp(color, float3(w,w,w) * factor, w * factor2);
}

float4 fs_main(PS_Input i) : SV_TARGET
{
    const float exposure = 1.0f;
    const float gamma = 2.2;
    float3 hdrColor = pow(abs(srv.Sample(samplerState, float2(i.uv.x, 1.0f - i.uv.y)).rgb), float3(gamma, gamma, gamma));
    float3 bloomColor = bloom.Sample(samplerState, float2(i.uv.x, 1.0f - i.uv.y)).rgb;
    hdrColor += bloomColor;
    // Tone mapping
    float3 mapped = float3(1.0, 1.0, 1.0) - exp(-hdrColor * exposure);
    //float3 mapped = hdrColor / (hdrColor + float3(1.0, 1.0, 1.0));
    // gamma correction
    float invGamma = 1.0f / gamma;
    mapped = pow(abs(mapped), float3(invGamma, invGamma, invGamma));
    

    float3 tint = float3(2.8, 2.4, 2.0);
    float2 uv = i.uv - 0.5f;
    uv.x *= resolution.x / resolution.y;
    float2 sPos = (screenPos / resolution.xy) - float2(0.5, 0.5);
    sPos.x *= resolution.x / resolution.y;
    float3 color = mapped;
    color += tint * lensflare(uv, sPos) * flerActive.x;
    color -= noise(i.fragPos.xy) * .015;
    color = cc(color, .5, .1);
    return float4(color, 1.0f);
}