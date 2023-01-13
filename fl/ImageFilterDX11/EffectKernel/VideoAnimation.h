#ifndef _VIDEO_ANIMATION_H_
#define _VIDEO_ANIMATION_H_

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <array>
#include "Toolbox/xmlParser.h"
#include "Toolbox/zip/unzip.h"
#include "Toolbox/json.hpp"

class DX11Texture;
class DX11Shader;
class CC3DRenderTargetRHI;
class CC3DTextureRHI;
class Video;
class CC3DVertexBuffer;
class CC3DIndexBuffer;

class VideoAnimation
{
public:
	VideoAnimation();
	~VideoAnimation();
public:
	void Init(const std::string& m_resourcePath, int videoWidth, int videoHeight);
	bool ReadJson(const nlohmann::json& root);
	bool ReadJsonFromZip(const std::string &jsonName, HZIP hZip);
	bool ReadJsonFromFile(const std::string& jsonName);
	void SetTexture(int index, const std::string& texturePath);
	void SetTexture(int index, long long id);
	void SetMaskWriteAlpha(int index, bool alphaOn);
	std::shared_ptr<CC3DRenderTargetRHI> Render(Video* vdo);
	std::shared_ptr<CC3DRenderTargetRHI> GetMaskFbo() noexcept;
	std::shared_ptr<CC3DTextureRHI> GetMaskTex();
private:
	std::map< int, std::map<int, std::vector<float>> > frameMaskMap;
	std::map<int, std::shared_ptr<CC3DTextureRHI>> indexMaskTextureMap;
	std::map<int, bool> indexWriteAlpha;
	std::shared_ptr<CC3DTextureRHI> maskTex;
	int maxMaskCount = 0;

	std::shared_ptr<CC3DRenderTargetRHI> renderMaskFbo;
	std::shared_ptr<CC3DRenderTargetRHI> renderResultFbo;
	std::shared_ptr<CC3DVertexBuffer> mVertexBuffer;
	std::shared_ptr<CC3DIndexBuffer> mIndexBuffer;
	DX11Shader* pShaderRect = nullptr;
	DX11Shader* pShaderBlend = nullptr;
	std::array< std::array<std::shared_ptr<CC3DVertexBuffer>, 10>, 10> mRectVertexBuffer;    // maskIndex¶ÔÓ¦µÄVB

	float g_TextureCoordinate[8] = { 0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
};

#endif