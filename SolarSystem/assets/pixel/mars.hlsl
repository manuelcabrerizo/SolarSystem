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

float3 CalcPointLight(float3 color, PointLight light, float3 normal, float3 viewDir, float3 fragPos)
{

    float3 lightDir = normalize(light.position_ - fragPos);
    float diff = max(dot(normal, lightDir), 0.0f);

    float3 ambient = light.ambient_ * color;
    float3 diffuse = (light.diffuse_*0.99) * diff * color;

    float attenuation = 1.0f / length(light.position_ - fragPos);
    diffuse *= attenuation;

    return ambient + diffuse;
}


float3 hash3(float3 p)
{
    p = float3(
    dot(p, float3(127.1, 331.7, 74.7)),
    dot(p, float3(269.5, 183.3, 246.1)),
    dot(p, float3(113.5, 271.9, 124.6)));
    return -1.0f + 2.0f * frac(sin(p) * 43758.5453123);
}

float noise(in float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    float3 u = f * f * (3.0 - 2.0 * f);

    return lerp(lerp(lerp(dot(hash3(i + float3(0.0, 0.0, 0.0)), f - float3(0.0, 0.0, 0.0)),
                          dot(hash3(i + float3(1.0, 0.0, 0.0)), f - float3(1.0, 0.0, 0.0)), u.x),
                     lerp(dot(hash3(i + float3(0.0, 1.0, 0.0)), f - float3(0.0, 1.0, 0.0)),
                          dot(hash3(i + float3(1.0, 1.0, 0.0)), f - float3(1.0, 1.0, 0.0)), u.x), u.y),
                lerp(lerp(dot(hash3(i + float3(0.0, 0.0, 1.0)), f - float3(0.0, 0.0, 1.0)),
                          dot(hash3(i + float3(1.0, 0.0, 1.0)), f - float3(1.0, 0.0, 1.0)), u.x),
                     lerp(dot(hash3(i + float3(0.0, 1.0, 1.0)), f - float3(0.0, 1.0, 1.0)),
                          dot(hash3(i + float3(1.0, 1.0, 1.0)), f - float3(1.0, 1.0, 1.0)), u.x), u.y), u.z);
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

float fbm(in float3 p, int octaves, float persistence, float lacunarity)
{
    float amplitude = 0.5f;
    float frequency = 0.25f;
    float total = 0.0f;
    float normalization = 0.0f;
    for (int i = 0; i < octaves; i++)
    {
        float noiseValue = noise(p * frequency);
        total += noiseValue * amplitude;
        normalization += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    total /= normalization;
    return total;
}


float4 fs_main(PS_Input i) : SV_TARGET
{
    float2 pixelCoords = i.uv;
    float3 color = float3(1.0f, 1.0f, 1.0f);
        
    float3 viewNormal = i.fragPos;
    float3 wsPosition = viewNormal;
    float3 wsNormal = normalize(wsPosition);
    float3 wsViewDir = float3(0.0f, 1.0f, 0.0f);
    
    float3 noiseCoord = wsPosition * 1.0f;
    float noiseSample = fbm(noiseCoord, 6, 0.5f, 2.0f);

    float waterMask = smoothstep(0.001, 0.04, noiseSample);
    float landMask = smoothstep(0.05, 0.1, noiseSample);
    
    float3 marsColor = float3(218.0 / 255.0, 79.0 / 255.0, 7.0 / 255.0)*0.8;
    
    float3 waterColor = lerp(float3(marsColor.r, marsColor.g, marsColor.b),
                             float3(marsColor.r*0.65, marsColor.g*0.65, marsColor.b*0.65),
                             waterMask);
    float3 landColor = lerp(float3(marsColor.r * 0.75, marsColor.g * 0.75, marsColor.b * 0.75),
                            float3(marsColor.r * 0.8, marsColor.g * 0.7, marsColor.b * 0.7),
                            landMask);
    

    landColor = lerp(landColor,
                     float3(marsColor.r * 0.9, marsColor.g * 0.8, marsColor.b * 0.8), smoothstep(0.08, 0.18, noiseSample));
    landColor = lerp(landColor,
                     float3(marsColor.r*1.1, marsColor.g*1.1, marsColor.b*1.1), smoothstep(0.18, 0.25, noiseSample));
    
    float mask = smoothstep(0.05, 0.1, noiseSample);
    
    color = lerp(waterColor, landColor, mask);
    
    float3 normal = normalize(i.nor);
    float3 viewDir = normalize(viewPos - i.fragPos);
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for (int index = 0; index < lightCount; index++)
    {
        PointLight light = lights[0];
        result += CalcPointLight(color, light, normal, viewDir, i.fragPos);
    }
    
    color = result;
    
    float3 fresnel = smoothstep(1.5f, 0.01f, dot(viewDir, normal));
    fresnel = pow(fresnel, 8.0);
    
    color = lerp(color, float3(0.9, 0.6, 0.4), fresnel);

    return float4(color, 1.0f);
}