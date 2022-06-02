//--------------------------------------------------------------------------------------  
//   
//  
// Copyright (c) Microsoft Corporation. All rights reserved.  
//--------------------------------------------------------------------------------------  
  
Texture2D txDiffuse : register( t0 );
Texture2D txMeterial : register(t1);
Texture2D txMask : register(t2);

SamplerState samLinear : register( s0 );
  
//--------------------------------------------------------------------------------------  
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
    float2 TexMask : TEXCOORD1;
};
struct VS_OUTPUT  
{  
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float2 TexMask : TEXCOORD2;
    float2 TexAlpha : TEXCOORD3;
};  
  
//--------------------------------------------------------------------------------------  
// Vertex Shader  
//--------------------------------------------------------------------------------------  
VS_OUTPUT VS( VS_INPUT input)  
{  
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = input.Pos;
    output.Pos.y = -output.Pos.y;
    output.Pos.z=0.5;
    output.Pos.w=1.0;
    output.Tex = input.Pos.zw;

    output.Tex1 = input.Tex;
    output.TexMask = input.TexMask;
    output.TexAlpha = input.Pos.xy*0.5+0.5;
    return output;  
}  
  
float4 BlendNormal(float4 color1 ,float4 color2,float alpha)
{
    float4 color = color1*(1.0-alpha) + color2*alpha;
    color.a = 1.0;
    return color;
}
float4 PS( VS_OUTPUT input ) : SV_Target  
{  

    
    float4 color2 = txMeterial.Sample(samLinear,input.Tex1);
    float4 color3 = txMask.Sample(samLinear,input.TexMask);
    
    
    float texAlpha = color3.r*2.0;
    if(texAlpha>1.0)
    {
        texAlpha = 1.0;
    }
    float2 texCoord = input.Tex*texAlpha + input.TexAlpha*(1.0-texAlpha);
    float4 color1 = txDiffuse.Sample(samLinear, texCoord);

    float alpha = color3.r*color3.r*0.9;
    
    return color1*(1.0-alpha) + color2*alpha;//BlendNormal(color1,color2,alpha);
}  