Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 fragPos : TEXCOORD2;
};

cbuffer Common : register(b3)
{
    float2 resolution;
    float time;
    float pad0;
}

float inverseLerp(float v, float minValue, float maxValue)
{
    return (v - minValue) / (maxValue - minValue);
}

float remap(float v, float inMin, float inMax, float outMin, float outMax)
{
    float t = inverseLerp(v, inMin, inMax);
    return lerp(outMin, outMax, t);

}

float3 mod289(float3 x)
{
    return x - floor(x / 289.0f) * 289.0f;
}

float4 mod289(float4 x)
{
    return x - floor(x / 289.0f) * 289.0f;
}

float4 permute(float4 x)
{
    return mod289((x * 34.0 + 1.0) * x);
}

float3 hash3(float3 p)
{
    p = float3(
    dot(p, float3(127.1, 331.7, 74.7)),
    dot(p, float3(269.5, 183.3, 246.1)),
    dot(p, float3(113.5, 271.9, 124.6)));
    return -1.0f + 2.0f * frac(sin(p) * 43758.5453123);
}

float noise(in float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    float3 u = f * f * (3.0 - 2.0 * f);

    return lerp(lerp(lerp(dot(hash3(i + float3(0.0, 0.0, 0.0)), f - float3(0.0, 0.0, 0.0)),
                          dot(hash3(i + float3(1.0, 0.0, 0.0)), f - float3(1.0, 0.0, 0.0)), u.x),
                     lerp(dot(hash3(i + float3(0.0, 1.0, 0.0)), f - float3(0.0, 1.0, 0.0)),
                          dot(hash3(i + float3(1.0, 1.0, 0.0)), f - float3(1.0, 1.0, 0.0)), u.x), u.y),
                lerp(lerp(dot(hash3(i + float3(0.0, 0.0, 1.0)), f - float3(0.0, 0.0, 1.0)),
                          dot(hash3(i + float3(1.0, 0.0, 1.0)), f - float3(1.0, 0.0, 1.0)), u.x),
                     lerp(dot(hash3(i + float3(0.0, 1.0, 1.0)), f - float3(0.0, 1.0, 1.0)),
                          dot(hash3(i + float3(1.0, 1.0, 1.0)), f - float3(1.0, 1.0, 1.0)), u.x), u.y), u.z);
}

float3 GenerateGridStar(float2 pixelCoords, float starRadius, float cellWidth, float seed)
{
    float2 cellCoords = (frac(pixelCoords / cellWidth) - 0.5f);
    
    float2 cellID = floor(pixelCoords / cellWidth) + seed / 100.0f;
    float3 cellHashValue = hash3(float3(cellID, 0.0f));
    
    float starBrightness = saturate(cellHashValue.z) * 2.0f;
    float2 starPosition = float2(0.0f, 0.0f);
    starPosition += cellHashValue.xy * (cellWidth * 0.5f) - starRadius * 4.0f;
    float distStar = length(cellCoords - starPosition);
    
    float glow = exp(-2.0 * distStar / starRadius);
    
    return float3(glow, glow, glow) * starBrightness;
}

float3 GenerateStars(float2 pixelCoords)
{    
    float3 stars = float3(0.0f, 0.0f, 0.0f);
    float size = 0.02f;
    float cellWidth = 1.0f;
    for (float i = 0.0f; i < 10; i++)
    {
        stars += GenerateGridStar(pixelCoords, size, cellWidth, i);
        size *= 0.7f;
    }
    return stars;
}

float4 fs_main(PS_Input i) : SV_TARGET
{       
    float3 p = normalize(i.viewDir) * 7.0f;
    float3 color0 = GenerateStars(p.xy);
    float3 color1 = GenerateStars(p.yz);
    return float4(color0 + color1, 1.0f); 
}