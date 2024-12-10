cbuffer Camera : register(b1)
{
    matrix view;
    matrix proj;
    float3 viewPos;
    float pad;
};


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
    unsigned int Type : TEXCOORD0;
};

struct GeoOut
{
    float4 PosH : SV_Position;
    float2 Tex : TEXCOORD0;
};

#define PT_EMITTER 0
#define PT_FLARE 1

static float3 gAccelW = { 0.0f, 0.0f, 0.0f };

[maxvertexcount(2)]
void gs_main(point GeoIn gin[1],
        inout LineStream<GeoOut> lineStream)
{
    // do not draw emitter particles
    if (gin[0].Type != PT_EMITTER)
    {
        // Slant line in acceleration direction
        float3 p0 = gin[0].PosW;
        float3 p1 = gin[0].PosW + float3(0, 0.2, 0); // * gAccelW;

        GeoOut v0;
        float4 wPos = float4(p0, 1.0f);
        wPos = mul(view, wPos);
        v0.PosH = mul(proj, wPos);
        v0.Tex = float2(0, 0);
        lineStream.Append(v0);

        GeoOut v1;
        wPos = float4(p1, 1.0f);
        wPos = mul(view, wPos);
        v1.PosH = mul(proj, wPos);
        v1.Tex = float2(1, 1);
        lineStream.Append(v1);
    }
}