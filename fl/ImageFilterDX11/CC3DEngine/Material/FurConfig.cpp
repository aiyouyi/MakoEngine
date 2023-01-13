#include "FurConfig.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/json.hpp"

namespace CC3DImageFilter
{
	FurConfig::FurConfig()
	{

	}

	FurConfig::~FurConfig()
	{

	}

	void FurConfig::ParseMaterial(XMLNode& childNode, HZIP hZip /*= 0*/, char* pFilePath /*= NULL*/)
	{
		//Ã«·¢²ÄÖÊ½âÎö
		XMLNode nodeMaterial = childNode.getChildNode("FurMaterial", 0);
		if (!nodeMaterial.isEmpty())
		{
			nlohmann::json mtl_json;
			const char* szFurName = nodeMaterial.getAttribute("name");
			if (szFurName != nullptr)
			{
				ConfigData.FurMaterialName = szFurName;
			}

			const char* noise_name = nodeMaterial.getAttribute("NoiseTex");
			if (noise_name != nullptr)
			{
				ConfigData.NoiseTex = noise_name;
				noiseTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, noise_name, false);
			}

			const char* length_name = nodeMaterial.getAttribute("LengthTex");
			if (length_name)
			{
				ConfigData.LengthTexName = length_name;
				lengthTex = GetDynamicRHI()->CreateAsynTextureZIP(hZip, length_name, false);
			}

			nodeMaterial.getAttributeFloatValue("FurLength", ConfigData.FurLength);
			nodeMaterial.getAttributeIntValue("FurLevel", ConfigData.FurLevel);
			nodeMaterial.getAttributeFloatValue("UVScale", ConfigData.UVScale);
			nodeMaterial.getAttributeFloatValue("gamma", ConfigData.FurGamma);

			int UseToneMapping = 0;
			nodeMaterial.getAttributeIntValue("useToneMapping", UseToneMapping);
			ConfigData.UseToneMapping = UseToneMapping == 1;

			nodeMaterial.getAttributeVectorValue("FurColor", ConfigData.FurColor);
			nodeMaterial.getAttributeVectorValue("Gravity", ConfigData.vGravity);
			nodeMaterial.getAttributeFloatValue("LightFilter", ConfigData.LightFilter);
			nodeMaterial.getAttributeFloatValue("FurLightExposure", ConfigData.FurLightExposure);
			nodeMaterial.getAttributeFloatValue("FurAmbientStrength", ConfigData.FurAmbientStrength);
			nodeMaterial.getAttributeFloatValue("Fresnel", ConfigData.Fresnel);
			nodeMaterial.getAttributeFloatValue("FurMask", ConfigData.FurMask);
			nodeMaterial.getAttributeFloatValue("Tming", ConfigData.Tming);
		}
	}
}

