static float3 gAccelW = { 0.0f, -2.0f, 0.0f };

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
    unsigned int Type : TEXCOORD0;
};

PS_Input vs_main(VS_Input vin)
{
    PS_Input vout;
    
    float t = vin.age;
    
    // constant accelertion equation
    vout.PosW = 0.5f * t * t * gAccelW + t * vin.vel + vin.pos;
    vout.Type = vin.type;

    return vout;
}