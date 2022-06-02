cbuffer ConstantBuffer : register(b0)
{
    float4 param;
    float4 hairParam;
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

float RGBToL(float4 RGBColor)
{
    float3 rgb = RGBColor.xyz;  // input is 0-1
    float y = 0.212671*rgb.x + 0.715160*rgb.y + 0.072169*rgb.z;
    float l  = y > 0.008856 ? (116.0 * pow(y,1.0/3.0) - 16.0) : (903.3 * y); //fix the 1/3 power, output 0-100
    l = l / 100.0 * 255.0;    //output 0-255
    return l;
}

float4 PS(VS_OUTPUT input) : SV_Target
{

    float4 srcColor = txDiffuseSrc.Sample(samLinear,input.Tex);

	float mask = txDiffuseMask.Sample(samLinear, input.Tex).r;

    float4 HairColor = hairParam;
    
    // if(param.x == 0.0){
    //     HairColor = float4(0.6, 0.08, 1.0,0.25);//hairParam;//
    // }
    // else{
    //     HairColor = srcColor;
    // }
    
    // float reflector = 0.55552;
    // float la = RGBToL(srcColor) / reflector; //get shade

    // la = (1.0 / (1.0 + exp(-1.0*(la/200.0)))-0.5) * 800.0; //clip shade

    // la = la * mask + 255.0 * (1.0 - mask);

    // float l_alpha = (srcColor.x+srcColor.y+srcColor.z)/3.0;
    
    // float strength = 0.25262;
    // float _strength = clamp(strength*(1.0+l_alpha),0.0,1.0);

    // float4 c = (HairColor * _strength + srcColor * (1.0 - _strength) ) * mask + srcColor * (1.0 - mask); //get new reflector
    // float4 result = la * c  / 255.0;
    // result = clamp(result, 0.0, 1.0);
    
    HairColor = HairColor * 0.25 + srcColor * (1.0 - 0.25);
    float4 result = float4(lerp(srcColor, HairColor, mask).rgb, 1.0);

	return result;
    //return float4(mask, mask, mask, 1.0);
   
};
