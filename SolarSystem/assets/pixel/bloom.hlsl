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
    float horizontal;
    float2 screenPos;
    float2 flerActive;
}


float4 fs_main(PS_Input i) : SV_TARGET
{
    float2 uv = float2(i.uv.x, 1.0f - i.uv.y);
    float weight[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

    
    uint width, height, levels;
    srv.GetDimensions(0, width, height, levels);
    float2 texOffset = 1.0f / float2((float) width, (float) height);
    float3 result = srv.Sample(samplerState, uv).rgb * weight[0];
    if (horizontal > 0.0)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += srv.Sample(samplerState, uv + float2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += srv.Sample(samplerState, uv - float2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += srv.Sample(samplerState, uv + float2(0.0, texOffset.x * i)).rgb * weight[i];
            result += srv.Sample(samplerState, uv - float2(0.0, texOffset.x * i)).rgb * weight[i];
        }
    }

    return float4(result, 1.0f);
}