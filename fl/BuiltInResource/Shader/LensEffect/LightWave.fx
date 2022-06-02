cbuffer ConstantBuffer : register(b0)
{
    float4 param; 
}
Texture2D txDiffuse : register(t0);
Texture2D txDiffuseMask : register(t1);
SamplerState samLinear : register(s0);

struct VS_INPUT
{
	float4 Pos : POSITION;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos.xy = input.Pos*2.0 - float2(1.0,1.0);
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w =1.0;
    output.Tex = input.Pos;
    return output;  
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    float2 coloruv = float2(param.a, 0);

    float4 color = txDiffuse.Sample(samLinear, coloruv); 

    float2 UVoffset = float2(param.x, param.y);         
    float2 tex2 = input.Tex*float2(0.88165, 0.89709) + UVoffset; 
	float4 sobelMask = txDiffuseMask.Sample(samLinear, tex2);
    
    if(sobelMask.r < 0.05)
    {
        sobelMask.r = 0.05;
        sobelMask.g = 0.05;
        sobelMask.b = 0.05;
    }

    float thick = 0.075;
    float extraIntensity = 0.0;
    if(input.Tex.x <= thick)
    {
        extraIntensity = (thick - input.Tex.x) / thick;
    }
    else if(input.Tex.x >= (1.0 - thick))
    {
        extraIntensity = (thick - (1.0 - input.Tex.x)) / thick;
    }
    if(input.Tex.y <= thick * 0.5625)
    {
        extraIntensity = max((thick * 0.5625 - input.Tex.y) / (thick * 0.5625), extraIntensity);
    }
    else if(input.Tex.y >= (1.0 - thick * 0.5625))
    {
        extraIntensity = max(((thick * 0.5625) - (1.0 - input.Tex.y)) / (thick * 0.5625), extraIntensity);
    }
    extraIntensity *= clamp(0.0, 1.0, frac(param.x * 80.0));

    extraIntensity = smoothstep(0.0, 1.0, extraIntensity);
    
    float luminanceIntensity = 5.0;
    if(param.z == 1.0)
    {
        sobelMask *= (1.0 + extraIntensity * 5.0 * luminanceIntensity);
    }
    else{
        sobelMask *= (extraIntensity * 5.0 * luminanceIntensity);
    }  
    float4 waveColor = float4(sobelMask.rgb, 0.25)*color;
	return waveColor;
};
