struct Particle
{
    float3 pos : POSITION;
    float3 vel : TEXCOORD0;
    float2 sizeW : TEXCOORD1;
    float age : TEXCOORD2;
    unsigned int type : TEXCOORD3;
};

Particle vs_main(Particle vin)
{
    Particle vout = vin;
    return vout;
}