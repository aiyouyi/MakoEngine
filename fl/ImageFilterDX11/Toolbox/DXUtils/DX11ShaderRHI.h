#pragma once
#include "ToolBox/Render/ShaderRHI.h"

class DX11Shader;
class DX11ShaderRHI : public ShaderRHI
{
public:
	DX11ShaderRHI();
	virtual ~DX11ShaderRHI();

	virtual bool InitShader(const std::string& str_path, CCVetexAttribute* pAttribs, int nAttri, bool seperate = false) override;
	virtual bool InitShaderWithString(const char* szShader) override;
	virtual bool initGeometryWithStreamOutput(const std::string& fileName, CCVetexAttribute* pAttribs, int nAttri,const std::string& VSMain, const std::string& GSMain,const std::vector< D3D11SOLayout>& SOLayouts);
	virtual bool initGeometryWithFile(const std::string& fileName, CCVetexAttribute* pAttribs, int nAttri,const std::string& VSMain, const std::string& GSMain, const std::string& PSMain);
	
	virtual bool initVSShader(const std::string& fileName, const std::string& VSMain, bool InitInputLayout, const std::vector< D3DShaderMacro>& MacroDefines = {}) override;
	virtual bool initPSShader(const std::string& fileName, const std::string& PSMain, const std::vector< D3DShaderMacro>& MacroDefines = {}) override;
	virtual bool initGeometryShader(const std::string& fileName, const std::string& GSMain, const std::vector< D3DShaderMacro>& MacroDefines = {}) override;
	virtual bool initComputeShader(const std::string& fileName, const std::string& CSMain, const std::vector< D3DShaderMacro>& MacroDefines = {}) override;
	
	virtual void UseShader() override;

private:
	DX11Shader* Shader = nullptr;
	bool NeedToDelete = false;
};