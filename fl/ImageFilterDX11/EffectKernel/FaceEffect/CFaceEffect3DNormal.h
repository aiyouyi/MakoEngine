#pragma once
#include "Toolbox/DXUtils/DXUtils.h"
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face3DReconstruct/MultiLinearVideoReconstructor.h"
#include "Toolbox/Drawable.hpp"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/CC3DShaderDef.h"


BEING_SHADER_STRUCT(NormalConstantBuffer,0)
	DELCARE_PARAM(glm::mat4, matWVP)
	DELCARE_PARAM(glm::mat4, matNormal)
	BEING_STRUCT_CONSTRUCT(NormalConstantBuffer)
		IMPLEMENT_PARAM("matWVP", UniformType::MAT4)
		IMPLEMENT_PARAM("matNormal", UniformType::MAT4)
	END_STRUCT_CONSTRUCT
END_SHADER_STRUCT

class CFaceEffect3DNormal : public CEffectPart
{
public:
	CFaceEffect3DNormal();
	virtual ~CFaceEffect3DNormal();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam &RenderParam);

	//TODO:还有数组类型的uniform没有解决，好像可以直接使用size去包含整个数组大小
	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(NormalConstantBuffer).SetParameter(name, value, size);
	}


private:
	std::array<std::shared_ptr<CC3DVertexBuffer>,10> VerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer;

	std::shared_ptr<CC3DTextureRHI> m_NormalMap;


	mt3dface::MultiLinearVideoReconstructor *m_Reconstruct3D;

	DELCARE_SHADER_STRUCT_MEMBER(NormalConstantBuffer);


	bool m_isFirstInit = true;

};