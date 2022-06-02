#pragma once
#include "EffectKernel/CEffectPart.h"
#include "Toolbox/Drawable.hpp"
//#include "EffectModel.hpp"
#include "Toolbox/DXUtils/DXUtils.h"
#include <vector>
#include "Algorithm/InterFacePointDeform.h"
#include "Toolbox/Render/CC3DShaderDef.h"

class CBodyLift2 :public CEffectPart
{
public:
	CBodyLift2();
	~CBodyLift2();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;

	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(ConstantBufferVec4).SetParameter(name, value, size);
	}

private:
	DELCARE_SHADER_STRUCT_MEMBER(ConstantBufferVec4);


	void FilterToFaceFBO(BaseRenderParam &RenderParam, int nWidth, int nHeight);

	std::shared_ptr<InterFacePointDeform> m_InterFM;
	std::shared_ptr<CC3DRenderTargetRHI> m_pFBOFace;

	std::array<std::shared_ptr<CC3DVertexBuffer>, 10> mVertexBuffer;

	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;


	std::array<std::shared_ptr<CC3DVertexBuffer>, 10> mVertexBuffer2;

	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer2;


	DX11Shader * m_pShaderFace = nullptr;

	std::shared_ptr<CC3DTextureRHI> m_FaceMask;

	Vector2 m_Center = Vector2(0.5, 0.5);

	int m_nWidth = 0;
	int m_nHeight = 0;
};

