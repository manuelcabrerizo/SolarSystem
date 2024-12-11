Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

cbuffer CBParticle : register(b5)
{
    float3 eyePosW;
    float gameTime;

    float3 emitPosW;
    float timeStep;
    
    float3 emitDirW;
    float thrust;
    float3 starVelocity;
    float pad1;
}

struct PS_Input
{
    float4 PosH : SV_Position;
    float2 Tex : TEXCOORD0;
    float age : TEXCOORD1;
};

float inverseLerp(float v, float minValue, float maxValue)
{
    return (v - minValue) / (maxValue - minValue);
}

float remap(float v, float inMin, float inMax, float outMin, float outMax)
{
    float t = inverseLerp(v, inMin, inMax);
    return lerp(outMin, outMax, t);

}

float4 fs_main(PS_Input i) : SV_TARGET
{
    float alpha = saturate(remap(thrust, 0.0f, 0.6f, 0.0f, 1.0f));
    
    float age = (0.25 - i.age);
    float4 text = srv.Sample(samplerState, i.Tex);
    return float4(text.rgb * float3(0.3, 0.8, 0.1), (text.a * age) * (alpha*alpha));
}