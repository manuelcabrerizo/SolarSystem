struct VS_Input
{
    float3 pos : POSITION;
    float3 vel : TEXCOORD0;
    float2 sizeW : TEXCOORD1;
    float age : TEXCOORD2;
    unsigned int type : TEXCOORD3;
};

struct PS_Input
{
    float3 PosW : POSITION;
    float2 SizeW : TEXCOORD0;
    float age : TEXCOORD1;
    unsigned int Type : TEXCOORD2;
};

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

PS_Input vs_main(VS_Input vin)
{
    PS_Input vout;
    
    float t = vin.age;
    // Check if this is correct    
    vout.PosW = vin.pos + vin.vel * t;
    vout.SizeW = vin.sizeW;
    vout.Type = vin.type;
    vout.age = vin.age;

    return vout;
}