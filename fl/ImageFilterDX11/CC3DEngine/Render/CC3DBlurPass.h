#ifndef _CC3D_BLUR_PASS_H_
#define _CC3D_BLUR_PASS_H_

#include <memory>
#include <string>
#include <array>
#include"BaseDefine/Define.h"
#include "Toolbox/DXUtils/DX11DoubleBuffer.h"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11Texture.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/RenderTargetRHI.h"


class CC3DBlurPass
{
public:
	CC3DBlurPass();
	~CC3DBlurPass();

	void Init(int nWidth, int nHeight);
	void SetShaderResource(const std::string& path);
	void Process(std::shared_ptr<CC3DRenderTargetRHI> input);
	std::shared_ptr<CC3DRenderTargetRHI> GetResult();

private:
	DX11Shader* mShader = nullptr;

	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;
	std::shared_ptr<CC3DConstantBuffer> mContanstBuffer;


	std::array<std::shared_ptr<CC3DRenderTargetRHI>, 2> mDoubleBuffer;
	bool mbInit = false;
	int32_t mWidth = 0;
	int32_t mHeight = 0;
};

#endif