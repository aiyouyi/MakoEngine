#ifndef _CC3D_BLUR_PASS_H_
#define _CC3D_BLUR_PASS_H_

#include "Toolbox/Render/CC3DShaderDef.h"
#include "Toolbox/Render/RenderTargetRHI.h"


BEGIN_SHADER_STRUCT(Gaussianblur, 0)
	DECLARE_PARAM_VALUE(int, horizontal, false)
	BEGIN_STRUCT_CONSTRUCT(Gaussianblur)
		IMPLEMENT_PARAM("horizontal", UniformType::BOOL)
	INIT_TEXTURE_INDEX("image", 0)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CC3DBlurPass
{
public:
	CC3DBlurPass();
	~CC3DBlurPass();

	void Init(int nWidth, int nHeight,uint32_t format);
	void SetShaderResource(const std::string& path);
	void Process(std::shared_ptr<CC3DRenderTargetRHI> input);
	std::shared_ptr<CC3DRenderTargetRHI> GetResult();

private:
	std::shared_ptr<ShaderRHI> mShader = nullptr;

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;


	std::array<std::shared_ptr<CC3DRenderTargetRHI>, 2> mDoubleBuffer;
	bool mbInit = false;
	int32_t mWidth = 0;
	int32_t mHeight = 0;

	DECLARE_SHADER_STRUCT_MEMBER(Gaussianblur)
};

#endif