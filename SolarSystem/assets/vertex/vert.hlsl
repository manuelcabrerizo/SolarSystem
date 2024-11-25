cbuffer Matrices : register(b0) {
    matrix model;
    matrix view;
    matrix proj;
};

struct VS_Input {
    float3 pos : POSITION;
    float3 nor : NORMAL;
    float4 col : COLOR;
};

struct PS_Input {
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float4 col : COLOR;
};

PS_Input vs_main(VS_Input i) {
    PS_Input o = (PS_Input)0;
    
    float4 wPos =  mul(float4(i.pos, 1.0f), model);
    wPos = mul(wPos, view);
    wPos = mul(wPos, proj);
    
    float3 wNor = mul(i.nor, (float3x3) model);
    wNor = normalize(wNor);
    
    o.pos = wPos;
    o.nor = wNor;
    o.col = i.col;

    return o;
}