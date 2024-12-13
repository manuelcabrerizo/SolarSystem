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

    return ambient + diffuse + specular;
}

float4 fs_main(PS_Input i) : SV_TARGET
{
    float4 textureColor = srv.Sample(samplerState, i.uv);
    
    float3 red = float3(201.0f / 255.0f, 61.0f / 255.0f, 92.0f / 255.0f);
    float colorDiff = length(red - textureColor.rgb);
    
    if (colorDiff < 0.2)
    {
        
        float3 bloomColor = float3(10.0, 0.2, 0.2);
        return float4(lerp(textureColor.rgb, bloomColor, thrust*thrust), 1.0f);
    }
    
        float3 normal = normalize(i.nor);
    float3 viewDir = normalize(viewPos - i.fragPos);
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for (int index = 0; index < lightCount; index++)
    {
        PointLight light = lights[0];
        result += CalcPointLight(textureColor.rgb, light, normal, viewDir, i.fragPos);
    }
    
    return float4(result, 1.0f);
}