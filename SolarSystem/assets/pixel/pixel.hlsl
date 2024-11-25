struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    return float4(i.uv.x, i.uv.y, 0.0f, 1.0f);
}