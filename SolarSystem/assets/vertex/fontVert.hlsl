cbuffer Camera : register(b1)
{
    matrix view;
    matrix proj;
    float3 viewPos;
    float pad;
};

struct VS_Input {
    float3 pos : POSITION;
    float3 nor : NORMAL;
    float3 tan : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

struct PS_Input {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_Input vs_main(VS_Input i) {
    PS_Input o = (PS_Input)0;
    float4 wPos = float4(i.pos, 1.0f);
    wPos = mul(view, wPos);
    wPos = mul(proj, wPos);
    o.pos = wPos;
    o.uv = i.uv;    
    return o;
}