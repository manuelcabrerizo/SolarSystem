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

#define iterations 20
#define formuparam 0.53

#define volsteps 4
#define stepsize 0.1

#define zoom   0.400
#define tile   0.850
#define speed  0.0150

#define brightness 0.0055
#define darkmatter 0.7300
#define distfading 0.730
#define saturation 0.850

float4 fs_main(PS_Input i) : SV_TARGET
{   
    float3 from = normalize(i.viewDir);
    //from += float3(time * 2.0f, time, -2.0f)*0.01f;
    //volumetric rendering
	float s=0.01, fade=1.;
	float3 v = float3(0.0f, 0.0f, 0.0f);
	for (int r=0; r<volsteps; r++) {
		float3 p = from+s*0.5f;
        p = abs(float3(tile, tile, tile) - fmod(p, float3(tile * 2., tile * 2., tile * 2.))); // tiling fold
		float pa,a=pa=0.;
		for (int i=0; i<iterations; i++) { 
			p=abs(p)/dot(p,p)-formuparam; // the magic formula
			a+=abs(length(p)-pa); // absolute sum of average change
			pa=length(p);
		}
		float dm=max(0.,darkmatter-a*a*.001); //dark matter
		a*=a*a; // add contrast
		if (r>6) fade*=1.-dm; // dark matter, don't render near
		//v+=vec3(dm,dm*.5,0.);
		v+=fade;
		v+=float3(s,s*s,s*s*s*s)*a*brightness*fade; // coloring based on distance
		fade*=distfading; // distance fading
		s+=stepsize;
	}
    float l = length(v);
    v = lerp(float3(l, l, l), v, saturation); //color adjust
    return float4(v * .01, 1.);
}