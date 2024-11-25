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
    
    float4 wPos = mul(model, float4(i.pos, 1.0f));
    wPos = mul(view, wPos);
    wPos = mul(proj, wPos);
    
    float3 wNor = mul((float3x3) model, i.nor);
    wNor = normalize(wNor);
    
    o.pos = wPos;
    o.nor = wNor;
    o.col = i.col;

    return o;
}