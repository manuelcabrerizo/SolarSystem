cbuffer Matrices : register(b0) {
    matrix proj;
    matrix view;
    matrix world;
};

struct VS_Input {
    float3 pos       : POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
};

struct PS_Input {
    float4 pos       : SV_POSITION;
    float3 nor       : NORMAL;
    float2 uv        : TEXCOORD0;
    unsigned int tex : TEXCOORD1;
    float3 fragPos   : TEXCOORD2;
};

PS_Input vs_main(VS_Input i) {
    PS_Input o = (PS_Input)0;
    float4 wPos =  mul(float4(i.pos, 1.0f), world);
    float3 fragPos = float3(wPos.xyz);
    wPos = mul(wPos, view);
    wPos = mul(wPos, proj);
    float3 wNor = mul(i.nor, (float3x3)world);
    wNor = normalize(wNor);
    o.pos = wPos;
    o.nor = wNor;
    o.uv = i.uv;
    o.tex = i.tex;
    o.fragPos = fragPos;
    return o;
}