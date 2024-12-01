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

#define PI 3.14159265359
#define TAU 6.28318530718

float2 DirToRectilinear(float3 dir)
{
    float x = atan2(dir.z, dir.x) / TAU + 0.5;
    float y = dir.y * 0.5 + 0.5;
    return float2(x, y);
}

float4 fs_main(PS_Input i) : SV_TARGET
{
    float3 viewDir = normalize(i.viewDir);
    
    float d = cos(viewDir.y * TAU * 1.0f * sin(viewDir.x * TAU * 1.0f) + sin(viewDir.z * TAU * 1.0f));
    float3 color = lerp(float3(0.01f, 0.01f, 0.03f), float3(0.02f, 0.01f, 0.08f), d);
    return float4(color, 1.0f);
    
    /*
    float2 skyUv = DirToRectilinear(viewDir);
    skyUv.y = 1.0f - skyUv.y;
    float4 textureColor = srv.SampleLevel(samplerState, skyUv, 0);
    return textureColor;
    */
}