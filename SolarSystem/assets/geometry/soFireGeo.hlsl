
Texture1D gRandomTex;
SamplerState samLinear;

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

[maxvertexcount(11)]
void gs_main(point Particle gin[1], inout PointStream<Particle> ptStream)
{
    gin[0].age += timeStep;
    if (gin[0].type == PT_EMITTER)
    {
        // time to emit a new particle?
        if (gin[0].age > 0.016f)
        {
            float3 front = normalize(emitDirW);
            float3 up = float3(0, 1, 0);
            float3 right = cross(up, front);
            
            float3 velocit = front * 1.0f;
            float3 relVel = starVelocity + (velocit * (thrust*thrust));
                       
            for (int i = 0; i < 10; ++i)
            {
                float3 vRandom = 0.01f * RandomVec3((float) i / 30.0f);

                Particle p;
                p.pos = emitPosW + (front*0.02) + vRandom;
                p.vel = relVel;
                p.sizeW = float2(0.03f, 0.025f);
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
        if (gin[0].age <= 0.25f)
            ptStream.Append(gin[0]);
    }
}