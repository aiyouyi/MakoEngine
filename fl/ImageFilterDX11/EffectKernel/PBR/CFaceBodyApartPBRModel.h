#pragma once
#include "EffectKernel/CEffectPart.h"
#include "EffectKernel/PBR/CRenderPBRModel.h"

#include <vector>
#include <string>
#include<map>
#include <chrono>
#include "Toolbox/Render/CC3DShaderDef.h"


class CFaceBodyApartPBRModel :public CEffectPart
{
public:
	CFaceBodyApartPBRModel();
	~CFaceBodyApartPBRModel();
	virtual void* Clone() override;
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL) override;
	virtual bool Prepare() override;
	virtual void Render(BaseRenderParam& RenderParam) override;
	virtual void Release() override;
	//TODO:还有数组类型的uniform没有解决，好像可以直接使用size去包含整个数组大小

	template<typename T>
	void SetParameter(std::string name, const T* value, size_t count, size_t size)
	{
		GET_SHADER_STRUCT_MEMBER(ConstantBufferMat4).SetParameter(name, value, size);
	}

	std::shared_ptr<CRenderPBRModel>m_RenderUtils;
private:
	std::shared_ptr<CC3DVertexBuffer> VerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer;
	float m_MatScale = 100.f;
	float m_MatScaleHead = 1.f;
	float m_rotateX = 0.0f;
	float m_GradientTime[3] = { 0,0,0 };
	std::string rotateNodeName;
	DELCARE_SHADER_STRUCT_MEMBER(ConstantBufferMat4);
	bool bFollowFace = false;
	bool bMatrixInit = false;
	glm::mat4 worldMat = glm::mat4(1.0);
	glm::mat4 projMat = glm::mat4(1.0);
	glm::ivec4 facerect;
	std::chrono::time_point<std::chrono::system_clock> begin = std::chrono::system_clock::now();
	int m_transition_frames = 10;
	//glm::vec3 m_pre_position;
};

