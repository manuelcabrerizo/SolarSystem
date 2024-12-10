
Texture1D gRandomTex;
SamplerState samLinear;

cbuffer CBParticle : register(b5)
{
    float3 eyePosW;
    float gameTime;

    float3 emitPosW;
    float timeStep;
    
    float3 emitDirW;
}

struct Particle
{
    float3 pos : POSITION;
    float3 vel : TEXCOORD0;
    float2 sizeW : TEXCOORD1;
    float age : TEXCOORD2;
    unsigned int type : TEXCOORD3;
};

float3 RandomUnitVec3(float offset)
{
    float u = (gameTime + offset);
    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
    return normalize(v);
}

float3 RandomVec3(float offset)
{
    float u = (gameTime + offset);
    float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
    return v;

}

#define PT_EMITTER 0
#define PT_FLARE 1

[maxvertexcount(6)]
void gs_main(point Particle gin[1],
        inout PointStream<Particle> ptStream)
{

    gin[0].age += timeStep;
    if (gin[0].type == PT_EMITTER)
    {
        // time to emit a new particle?
        if (gin[0].age > 0.016f)
        {

            for (int i = 0; i < 5; ++i)
            {

                float3 vRandom = 4.0f * RandomVec3((float) i / 5.0f);
                vRandom.y = 2.0f;

                Particle p;
                p.pos = emitPosW + vRandom;
                p.vel = float3(0, 0, 0);
                p.sizeW = float2(1.0f, 1.0f);
                p.age = 0.0f;
                p.type = PT_FLARE;

                ptStream.Append(p);
            }

            // reset the time to emit
            gin[0].age = 0.0f;
        }
        // always keep emitters
        ptStream.Append(gin[0]);
    }
    else
    {
        // Specify condition to keep particle; this may vary from system to system.
        if (gin[0].age <= 2.0f)
            ptStream.Append(gin[0]);
    }
}