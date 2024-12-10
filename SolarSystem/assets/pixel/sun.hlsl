Texture2D srv : register(t0);
SamplerState samplerState : register(s0);

struct PS_Input
{
    float4 pos : SV_POSITION;
    float3 nor : NORMAL;
    float2 uv : TEXCOORD0;
    float3 viewDir : TEXCOORD1;
    float3 fragPos : TEXCOORD2;
};

struct PointLight
{
    float3 position_;
    float constant_;
    float3 ambient_;
    float linear_;
    float3 diffuse_;
    float quadratic_;
    float3 specular_;
    float pad0_;
};

cbuffer LightConstBuffer : register(b2)
{
    PointLight lights[4];
    int lightCount;
    float3 viewPos;
};

cbuffer Common : register(b3)
{
    float2 resolution;
    float time;
    float pad0;
    float2 screenPos;
    float2 pad1;
}

float random(in float2 st)
{
    return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in float2 st)
{
    float2 i = floor(st);
    float2 f = frac(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + float2(1.0, 0.0));
    float c = random(i + float2(0.0, 1.0));
    float d = random(i + float2(1.0, 1.0));

    float2 u = f * f * (3.0 - 2.0 * f);

    return lerp(a, b, u.x) +
            (c - a) * u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float2 mod289(float2 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float3 mod289(float3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float3 permute(float3 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

float snoise(float2 v)
{

    // Precompute values for skewed triangular grid
    const float4 C = float4(0.211324865405187,
                        // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,
                        // 0.5*(sqrt(3.0)-1.0)
                        -0.577350269189626,
                        // -1.0 + 2.0 * C.x
                        0.024390243902439);
                        // 1.0 / 41.0

    // First corner (x0)
    float2 i = floor(v + dot(v, C.yy));
    float2 x0 = v - i + dot(i, C.xx);

    // Other two corners (x1, x2)
    float2 i1 = float2(0.0, 0.0);
    i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
    float2 x1 = x0.xy + C.xx - i1;
    float2 x2 = x0.xy + C.zz;

    // Do some permutations to avoid
    // truncation effects in permutation
    i = mod289(i);
    float3 p = permute(
            permute(i.y + float3(0.0, i1.y, 1.0))
                + i.x + float3(0.0, i1.x, 1.0));

    float3 m = max(0.5 - float3(
                        dot(x0, x0),
                        dot(x1, x1),
                        dot(x2, x2)
                        ), 0.0);

    m = m * m;
    m = m * m;

    // Gradients:
    //  41 pts uniformly over a line, mapped onto a diamond
    //  The ring size 17*17 = 289 is close to a multiple
    //      of 41 (41*7 = 287)

    float3 x = 2.0 * frac(p * C.www) - 1.0;
    float3 h = abs(x) - 0.5;
    float3 ox = floor(x + 0.5);
    float3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);

    // Compute final noise value at P
    float3 g = float3(1,1, 1);
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * float2(x1.x, x2.x) + h.yz * float2(x1.y, x2.y);
    return 20.0 * dot(m, g);
}


float ridge(float h, float offset)
{
    h = abs(h); // create creases
    h = offset - h; // invert so creases are at top
    h = h * h; // sharpen creases
    return h;
}

float ridgedMF(float2 p, int octaves)
{
    float lacunarity = 3.0;
    float gain = 0.5;
    float offset = 0.6;

    float sum = 0.0;
    float freq = 8.0, amp = 0.7;
    float prev = 1.0;
    for (int i = 0; i < octaves; i++)
    {
        float n = ridge(snoise(p * freq), offset);
        sum += n * amp;
        sum += n * amp * prev; // scale by previous octave
        prev = n;
        freq *= lacunarity;
        amp *= gain;
    }
    return sum;
}

#define PI 3.14159265359
#define TAU 6.28318530718

float4 fs_main(PS_Input i) : SV_TARGET
{
    float2 st = i.uv;
    
    st *= 1.0f;

    st.x += (sin(st.y * TAU * 8 + time*2.0f) * 0.5f + 0.5f)*0.01;
    st.y += (sin(st.x * TAU * 8 + time * 2.0f) * 0.5f + 0.5f) * 0.01;

    float e = ridgedMF(st, 6);
    e *= e*e*e;
    float3 color = lerp(float3(0.5, 0.2, 0.1),
                        float3(1.0f, 1.0f, 0.2f), e) * 3.0f;
    
    float3 normal = normalize(i.nor);
    float3 viewDir = normalize(viewPos - i.fragPos);
    float3 fresnel = smoothstep(1.0f, 0.01f, dot(viewDir, normal));
    fresnel = pow(fresnel, 7.0);
    
    color = lerp(color, float3(100.0, 1.0, 0.0), fresnel);
    
    return float4(color, 1.0f);
}