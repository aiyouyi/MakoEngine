#pragma once

#include "Toolbox/Render/CC3DShaderDef.h"
#include "ToolBox/Render/TextureRHI.h"

class CC3DVertexBuffer;
class CC3DVertexBuffer;

enum CC3DMakeUpType
{
	CC3D_EYEBROW,
	CC3D_EYE,
	CC3D_MOUTH,
	CC3D_BEARD,
	CC3D_ANIMATE,
};


BEGIN_SHADER_STRUCT(MakUpConstantBuffer,0)
	DECLARE_PARAM(Vector4, HSVColors)
	DECLARE_PARAM_VALUE(int, bUseMask, 0)
	DECLARE_PARAM(Vector3, Pad)
	BEGIN_STRUCT_CONSTRUCT(MakUpConstantBuffer)
		IMPLEMENT_PARAM("HSVColors", UniformType::FLOAT4)
		IMPLEMENT_PARAM("bUseMask", UniformType::INT)
		IMPLEMENT_PARAM("Pad", UniformType::FLOAT3)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

BEGIN_SHADER_STRUCT(MakUpTexture, -1)
	BEGIN_STRUCT_CONSTRUCT(MakUpTexture)
		INIT_TEXTURE_INDEX("InputImage", 0)
		INIT_TEXTURE_INDEX("MaskTexture", 1)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT


class CC3DMakeUp 
{
public:
	CC3DMakeUp();
	~CC3DMakeUp();

	void Init(int width,int height);
	void InitShaderProgram(const std::string& path);
	void ChangeColor(float r, float g, float b, float a, std::shared_ptr<CC3DTextureRHI> pTexture, std::shared_ptr<CC3DTextureRHI> pMaskTexture = nullptr, CC3DMakeUpType type = CC3D_EYEBROW);

	std::shared_ptr<CC3DRenderTargetRHI> GetOutPut() { return mOutPut; }
private:
	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;
	std::shared_ptr<ShaderRHI> mShader;
	std::shared_ptr<CC3DRenderTargetRHI> mOutPut;

	DECLARE_SHADER_STRUCT_MEMBER(MakUpConstantBuffer);
	DECLARE_SHADER_STRUCT_MEMBER(MakUpTexture);
};