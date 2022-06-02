cbuffer ConstantBuffer : register(b0)
{
    float4 param;
}
Texture2D txDiffuseSrc : register(t0);
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
    float texelWidthOffset = param.x;
    float texelHeightOffset = param.y;

    float3 src[9];
    src[0] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(-texelWidthOffset,-texelHeightOffset)).rgb;
    src[1] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(0.0,-texelHeightOffset)).rgb;
    src[2] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(texelWidthOffset,-texelHeightOffset)).rgb;
    src[3] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(-texelWidthOffset,0.0)).rgb;
    src[5] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(texelWidthOffset,0.0)).rgb;
    src[6] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(-texelWidthOffset,texelHeightOffset)).rgb;
    src[7] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(0,texelHeightOffset)).rgb;
    src[8] = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(texelWidthOffset,texelHeightOffset)).rgb;

    float3 sobelX = src[2]+2.*src[5]+src[8]-src[0]-2.*src[3]-src[6];
    float3 sobelY = src[6]+2.*src[7]+src[8]-src[0]-2.*src[1]-src[2];

    float g = 0.0;
    if (input.Tex.x > 0.03 && input.Tex.x < 0.97 && input.Tex.y > 0.03 && input.Tex.y < 0.97){
        g = step(0.4, abs(dot(sobelX,float3(0.2125, 0.7154, 0.0721)))+ abs(dot(sobelY,float3(0.2125, 0.7154, 0.0721))));
        g *= param.z;
    }
    float4 sobelColor = float4(g, g, g, g);
	
    float4 segColor = float4(0.0,0.0,0.0,0.0);
	float mask = txDiffuseMask.Sample(samLinear, input.Tex).r; 
    segColor = float4(lerp(segColor, sobelColor, mask).rgb, 1.0);

    //segColor = txDiffuseSrc.Sample(samLinear,input.Tex  + float2(0.0,0.0));
	return segColor;
   
};
