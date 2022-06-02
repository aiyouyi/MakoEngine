static const float PI = 3.14159265359;
struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float4  BlendIndices  : BLENDINDICES;
	float4  BlendWeights  : BLENDWEIGHT;

};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
};

cbuffer ConstantBuffer1 : register(b0)
{
    float4x4 world;
    float4x4 meshMat;
    float4x4 lightSpaceMatrix;
    float4x4 meshMatInverse;
    int AnimationEnable;
    float3 pad;
}

static const int MAX_MATRICES = 200;
cbuffer ConstantBuffer2 : register(b1)
{
    matrix BoneMat[MAX_MATRICES];
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

    float4x4 ModeltoWorld ={
							1.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 1.0, 0.0,
							0.0, 0.0, 0.0, 1.0 };
    if (AnimationEnable == 1)
    {
        int i0 = int(input.BlendIndices.x);
        int i1 = int(input.BlendIndices.y);
        int i2 = int(input.BlendIndices.z);
        int i3 = int(input.BlendIndices.w);
        matrix BoneTransform = BoneMat[i0] * input.BlendWeights.x;
        BoneTransform += BoneMat[i1] * input.BlendWeights.y;
        BoneTransform += BoneMat[i2] * input.BlendWeights.z;
        BoneTransform += BoneMat[i3] * input.BlendWeights.w;

        ModeltoWorld = mul(meshMatInverse, BoneTransform);
    }
	
    ModeltoWorld = mul(meshMat, ModeltoWorld);
   // ModeltoWorld = mul(ModeltoWorld, world);


	float4 PosL = mul(float4(input.Pos, 1.0), ModeltoWorld);
    output.Pos = mul(PosL, lightSpaceMatrix);
	output.Pos/= output.Pos.w;
    output.Pos.z = output.Pos.z*0.5+0.5;
	return output;
};

float2 PS(VS_OUTPUT input) : SV_Target
{             
	 float depth = input.Pos.z/input.Pos.w;
    return float2(depth, depth * depth);
}