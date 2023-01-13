#pragma once
#include "Toolbox/inc.h"
#include "common.h"
#include "Toolbox/xmlParser.h"
#include "Toolbox/zip/unzip.h"

#include "Toolbox/Render/TextureRHI.h"

class MaterialTexRHI;

namespace CC3DImageFilter
{
	struct DX11IMAGEFILTER_EXPORTS_CLASS FurConfigData
	{
		float FurLength = 0.3f;
		float FurAmbientStrength = 0.75f;
		float FurLightExposure = 1.0f;
		float UVScale = 20.f;
		float Fresnel = 2.0f;
		float FurMask = 0.5f;
		float Tming = 0.5f;
		float FurGamma = 1.0f;
		float LightFilter = 1.56f;
		int FurLevel = 30;
		bool UseToneMapping = false;
		std::string FurMaterialName;
		std::string NoiseTex;
		std::string LengthTexName;
		Vector3 vGravity{ 0,-1,0 };
		Vector3 FurColor{ 1,1,1 };
	};

	class  FurConfig
	{
	public:
		FurConfig();
		~FurConfig();

	public:
		std::shared_ptr<MaterialTexRHI> noiseTex;
		std::shared_ptr<MaterialTexRHI> lengthTex;
		FurConfigData ConfigData;
	public:

		void ParseMaterial(XMLNode& childNode, HZIP hZip = 0, char* pFilePath = NULL);
	};
}
