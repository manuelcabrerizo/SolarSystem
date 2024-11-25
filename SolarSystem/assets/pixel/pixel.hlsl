struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
    float3 fragPos : TEXCOORD2;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}