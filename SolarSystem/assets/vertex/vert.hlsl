cbuffer Matrices : register(b0) {
    matrix model;
    matrix view;
    matrix proj;
};

struct VS_Input {
    float3 pos : POSITION;
    float3 nor : NORMAL;
    float3 tan : TEXCOORD0;
    float2 uv  : TEXCOORD1;
};

struct PS_Input {
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
};

PS_Input vs_main(VS_Input i) {
    PS_Input o = (PS_Input)0;
    
    float4 wPos = mul(model, float4(i.pos, 1.0f));
    wPos = mul(view, wPos);
    wPos = mul(proj, wPos);
    
    float3 wNor = mul((float3x3) model, i.nor);
    wNor = normalize(wNor);
    
    o.pos = wPos;
    o.nor = wNor;
    o.uv = i.uv;
    
    return o;
}