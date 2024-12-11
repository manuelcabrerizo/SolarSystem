cbuffer Camera : register(b1)
{
    matrix view;
    matrix proj;
    float3 viewPos;
    float pad;
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


static float2 gQuadTexC[4] =
{
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f)
};

struct GeoIn
{
    float3 PosW : POSITION;
    float2 SizeW : TEXCOORD0;
    float age : TEXCOORD1;
    unsigned int Type : TEXCOORD2;
};

struct GeoOut
{
    float4 PosH : SV_Position;
    float2 Tex : TEXCOORD0;
    float age : TEXCOORD1;
};

#define PT_EMITTER 0
#define PT_FLARE 1

[maxvertexcount(4)]
void gs_main(point GeoIn gin[1], inout TriangleStream<GeoOut> triStream)
{
    // do not draw emitter particles
    if (gin[0].Type != PT_EMITTER)
    {
        // Compute world matrix so that billboard faces the camera
        float3 look = normalize(eyePosW.xyz - gin[0].PosW);
        float3 right = normalize(cross(float3(0, 1, 0), look));
        float3 up = cross(look, right);

        // Compute triangle strip vertices (quad) in world space
        float halfWidth = 0.5f * gin[0].SizeW.x;
        float halfHeight = 0.5f * gin[0].SizeW.y;

        float4 v[4];
        v[0] = float4(gin[0].PosW + halfWidth * right - halfHeight * up, 1.0f);
        v[1] = float4(gin[0].PosW + halfWidth * right + halfHeight * up, 1.0f);
        v[2] = float4(gin[0].PosW - halfWidth * right - halfHeight * up, 1.0f);
        v[3] = float4(gin[0].PosW - halfWidth * right + halfHeight * up, 1.0f);

        // Transform quad vertices to world space and output
        // them as a triangle strip

        GeoOut gout;
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            
            float4 wPos = v[i];
            wPos = mul(view, wPos);
            wPos = mul(proj, wPos);

            gout.PosH = wPos;
            gout.Tex = gQuadTexC[i];
            gout.age = gin[0].age;
            triStream.Append(gout);
        }
    }
}