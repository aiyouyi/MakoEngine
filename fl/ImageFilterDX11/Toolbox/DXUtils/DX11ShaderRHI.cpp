#include "DX11ShaderRHI.h"
#include "EffectKernel/ShaderProgramManager.h"
#include "Toolbox/DXUtils/DX11Shader.h"

DX11ShaderRHI::DX11ShaderRHI()
{

}

DX11ShaderRHI::~DX11ShaderRHI()
{
	if (NeedToDelete)
	{
		delete Shader;
	}
}

bool DX11ShaderRHI::InitShader(const std::string& str_path, CCVetexAttribute* pAttribs, int nAttri, bool seperate /*= false*/)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
		NeedToDelete = false;
	}
	Shader = ShaderProgramManager::GetInstance()->GetOrCreateShaderByPathAndAttribs(str_path, pAttribs, nAttri, seperate);
	return Shader != nullptr;
}

bool DX11ShaderRHI::InitShaderWithString(const char* szShader)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();
	return Shader->initShaderWithString(szShader);
}

bool DX11ShaderRHI::initGeometryWithStreamOutput(const std::string& fileName, CCVetexAttribute* pAttribs, int nAttri, const std::string& VSMain, const std::string& GSMain, const std::vector< D3D11SOLayout>& SOLayouts)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();
	Shader->SetAttribute(pAttribs, nAttri);
	Shader->SetSeparate(false);
	return Shader->initGeometryWithStreamOutput(fileName,VSMain,GSMain,SOLayouts);
}

bool DX11ShaderRHI::initGeometryWithFile(const std::string& fileName, CCVetexAttribute* pAttribs, int nAttri,const std::string& VSMain, const std::string& GSMain, const std::string& PSMain)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();
	Shader->SetAttribute(pAttribs, nAttri);
	Shader->SetSeparate(false);
	return Shader->initGeometryWithFile(fileName,VSMain,GSMain,PSMain);
}

bool DX11ShaderRHI::initVSShader(const std::string& fileName, const std::string& VSMain, bool InitInputLayout, const std::vector< D3DShaderMacro>& MacroDefines /*= {}*/)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();

	if (MacroDefines.empty())
	{
		return Shader->initVSShader(fileName, VSMain, InitInputLayout, nullptr);
	}
	else
	{
		std::vector< D3D_SHADER_MACRO> Macros;
		Macros.resize(MacroDefines.size() + 1);
		for (size_t index = 0; index < MacroDefines.size();++index)
		{
			Macros[index] = { MacroDefines[index].Name.c_str(),MacroDefines[index].Definition.c_str() };
		}
		Macros[MacroDefines.size()] = { nullptr,nullptr };
		return Shader->initVSShader(fileName, VSMain, InitInputLayout, Macros.data());
	}

}

bool DX11ShaderRHI::initPSShader(const std::string& fileName, const std::string& PSMain, const std::vector< D3DShaderMacro>& MacroDefines /*= {}*/)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();

	if (MacroDefines.empty())
	{
		return Shader->initPSShader(fileName, PSMain, nullptr);
	}
	else
	{
		std::vector< D3D_SHADER_MACRO> Macros;
		Macros.resize(MacroDefines.size() + 1);
		for (size_t index = 0; index < MacroDefines.size(); ++index)
		{
			Macros[index] = { MacroDefines[index].Name.c_str(),MacroDefines[index].Definition.c_str() };
		}
		Macros[MacroDefines.size()] = { nullptr,nullptr };
		return Shader->initPSShader(fileName, PSMain, Macros.data());
	}
}

bool DX11ShaderRHI::initGeometryShader(const std::string& fileName, const std::string& GSMain, const std::vector< D3DShaderMacro>& MacroDefines /*= {}*/)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();

	if (MacroDefines.empty())
	{
		return Shader->initGeometryShader(fileName, GSMain, nullptr);
	}
	else
	{
		std::vector< D3D_SHADER_MACRO> Macros;
		Macros.resize(MacroDefines.size() + 1);
		for (size_t index = 0; index < MacroDefines.size(); ++index)
		{
			Macros[index] = { MacroDefines[index].Name.c_str(),MacroDefines[index].Definition.c_str() };
		}
		Macros[MacroDefines.size()] = { nullptr,nullptr };
		return Shader->initGeometryShader(fileName, GSMain, Macros.data());
	}
}

bool DX11ShaderRHI::initComputeShader(const std::string& fileName, const std::string& CSMain, const std::vector< D3DShaderMacro>& MacroDefines /*= {}*/)
{
	if (NeedToDelete && Shader)
	{
		delete Shader;
	}
	NeedToDelete = true;
	Shader = new DX11Shader();

	if (MacroDefines.empty())
	{
		return Shader->initComputeShader(fileName, CSMain, nullptr);
	}
	else
	{
		std::vector< D3D_SHADER_MACRO> Macros;
		Macros.resize(MacroDefines.size() + 1);
		for (size_t index = 0; index < MacroDefines.size(); ++index)
		{
			Macros[index] = { MacroDefines[index].Name.c_str(),MacroDefines[index].Definition.c_str() };
		}
		Macros[MacroDefines.size()] = { nullptr,nullptr };
		return Shader->initComputeShader(fileName, CSMain, Macros.data());
	}
}

void DX11ShaderRHI::UseShader()
{
	if (Shader)
	{
		Shader->useShader();
	}
}
