struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
	matrix matWVP;
}

Texture2D u_recordTexture0 : register(t0); 
Texture2D u_recordTexture1 : register(t1); 
Texture2D u_recordTexture2 : register(t2); 
Texture2D u_recordTexture3 : register(t3); 
Texture2D u_recordTexture4 : register(t4); 
Texture2D u_recordTexture5 : register(t5); 
Texture2D u_lastCovTexture: register(t6); 
SamplerState samLinear : register(s0); 
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul(matWVP, input.Pos);
	output.Tex = input.Tex;
	return output;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 vMax = u_recordTexture0.Sample(samLinear, input.Tex);
	float4 vMin = vMax;

	float4 vColor = u_recordTexture1.Sample(samLinear, input.Tex);
	vMax = max(vMax, vColor); 
	vMin = min(vMin, vColor); 
	vColor = u_recordTexture2.Sample(samLinear, input.Tex);
	vMax = max(vMax, vColor); 
	vMin = min(vMin, vColor); 
	vColor = u_recordTexture3.Sample(samLinear, input.Tex);
	vMax = max(vMax, vColor); 
	vMin = min(vMin, vColor); 
	vColor = u_recordTexture4.Sample(samLinear, input.Tex);
	vMax = max(vMax, vColor); 
	vMin = min(vMin, vColor); 
	vColor = u_recordTexture5.Sample(samLinear, input.Tex);
	vMax = max(vMax, vColor); 
	vMin = min(vMin, vColor); 

	float fChange = (vMax.x - vMin.x) / 6.0; 
	
	float fMeasureCov = 150.0 / 255.0; 
	float fPredictionCov = max(fMeasureCov*fChange*255.0 / 5.0, fMeasureCov / 1.5); 
	
	float fFilterCov = u_lastCovTexture.Sample(samLinear, input.Tex).x; 
	
	float fPreConv = fPredictionCov*fPredictionCov + fFilterCov*fFilterCov; 
	float fMesCov = fMeasureCov*fMeasureCov; 
	float fKG = sqrt(fPreConv / (fPreConv + fMesCov)); 
	fFilterCov = sqrt((1.0 - fKG)*fPreConv); 
	
	return float4(fFilterCov, fKG, fChange, 1.0); 
}