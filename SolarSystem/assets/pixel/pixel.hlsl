struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float4 col : COLOR;
};

float4 fs_main(PS_Input i) : SV_TARGET
{
    return i.col;
}