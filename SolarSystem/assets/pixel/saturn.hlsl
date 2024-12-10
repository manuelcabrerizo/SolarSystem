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

float3 CalcPointLight(float3 color, PointLight light, float3 normal, float3 viewDir, float3 fragPos)
{

    float3 lightDir = normalize(light.position_ - fragPos);
    float diff = max(dot(normal, lightDir), 0.0f);

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);

    float dist = length(light.position_ - fragPos);
    float attenuation = 1.0f / dist;

    float3 ambient = light.ambient_ * color;
    float3 diffuse = light.diffuse_ * diff * color;
    float3 specular = light.specular_ * spec * color;

    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse;
}

float4 fs_main(PS_Input i) : SV_TARGET
{
    float2 uv = i.uv;
    uv.x *= 0.0f;
    float3 color = srv.Sample(samplerState, uv).rgb;
    
    float3 normal = normalize(i.nor);
    float3 viewDir = normalize(viewPos - i.fragPos);
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for (int index1 = 0; index1 < lightCount; index1++)
    {
        PointLight light = lights[0];
        result += CalcPointLight(color, light, normal, viewDir, i.fragPos);
    }
    
    color = result;
    
    float3 fresnel = smoothstep(1.5f, 0.01f, dot(viewDir, normal));
    fresnel = pow(fresnel, 8.0);
    
    color = lerp(color, float3(0.1, 0.25, 0.6)*1.0f, fresnel);
    
    return float4(color, 1.0f);

}