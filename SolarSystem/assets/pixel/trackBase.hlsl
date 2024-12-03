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

#define PI 3.14159265359
#define TAU 6.28318530718

float3 CalcPointLight(float3 color, PointLight light, float3 normal, float3 viewDir, float3 fragPos)
{

    float3 lightDir = normalize(light.position_ - fragPos);
    float diff = max(dot(normal, lightDir), 0.0f);

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);

    float dist = length(light.position_ - fragPos);
    float attenuation = 1.0f / (light.constant_ + light.linear_ * dist + light.quadratic_ * (dist * dist));

    float3 ambient = light.ambient_ * color;
    float3 diffuse = light.diffuse_ * diff * color;
    float3 specular = light.specular_ * spec * color;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

float4 fs_main(PS_Input i) : SV_TARGET
{
    float e = i.uv.y;
    float right = step(e, 0.1f);
    float left = step(1.0f - e, 0.1f);
    
    float4 borderColor = float4(0.8f, 0.8f, 1.0f, 1.0f);
    float4 baseColor0 = float4(1.0f, 0.6f, 1.0f, 0.4f);
    float4 baseColor1 = float4(1.0f, 0.6f, 1.0f, 0.7f);
    
    float trackBorders = left + right;
    float trackBase = smoothstep(0.5f, 0.9f, sin((i.uv.y + 0.3) * TAU * 50 + sin(i.uv.x * TAU * 4.0f)) * 0.5f + 0.5f);
    float4 color = lerp(baseColor0, baseColor1, trackBase);
    color = lerp(color, borderColor, trackBorders);
    
    float3 normal = normalize(i.nor);
    float3 viewDir = normalize(viewPos - i.fragPos);
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for (int index = 0; index < lightCount; index++)
    {
        PointLight light = lights[0];
        result += CalcPointLight(color.rgb, light, normal, viewDir, i.fragPos);
    }
    
    return float4(result, color.a);
}