#ifndef _CC_FUR_MATERIAL_H_
#define _CC_FUR_MATERIAL_H_

#include "CC3DMaterial.h"

struct FurConstBuffer
{
	Vector3 vForce = { 0, -0.1, 0 };
	float furOffset = 0.0f;

	float furLength = 0.1;
	float uvScale = 20.0f;
	float lightFilter = 0.0f;
	float furLightExposure = 1.0f;

	float fresnelLV = 1.0f;
	float furMask = 0.5f;
	float tming = 0.5f;
	int drawSolid = 0;

	Vector3 furColor = Vector3(1.0f, 1.0f, 1.0f);
	float furGamma = 1.0f;
	int useToneMapping = 0;
	int useLengthTex = 0;
	Vector2 padding;
};

class CC3DMesh;
class CC3DFurMaterial : public CC3DMaterial
{
public:
	CC3DFurMaterial();
	virtual ~CC3DFurMaterial();


	virtual void InitShaderProgram(std::string path) override;


	virtual void PreRenderSet(CC3DMesh* pMesh) override;


	virtual void RenderSet(CC3DMesh* pMesh) override;


	int numLayers = 20;
	float furLength = 0.1f;
	float UVScale = 20.0f;
	std::shared_ptr<CC3DTextureRHI> noiseTexture = nullptr;
	std::shared_ptr<CC3DTextureRHI> lengthTexture = nullptr;
	FurConstBuffer fur_constBuffer;
private:
	std::string noise_tex_file;
	std::string default_tex_file;

	std::shared_ptr<CC3DTextureRHI> colorTexture = nullptr;
	std::shared_ptr<CC3DConstantBuffer> fur_uniformbuffer;


};

#endif