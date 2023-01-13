#pragma once
#include "Toolbox/inc.h"

enum CCVetexData
{
	/**Index 0 will be used as Position.*/
	VERTEX_ATTRIB_POSITION = 0,
	/**Index 1 will be used as Color.*/
	VERTEX_ATTRIB_COLOR,
	/**Index 2 will be used as Tex coord unit 0.*/
	VERTEX_ATTRIB_TEX_COORD,
	/**Index 3 will be used as Tex coord unit 1.*/
	VERTEX_ATTRIB_TEX_COORD1,
	/**Index 4 will be used as Tex coord unit 2.*/
	VERTEX_ATTRIB_TEX_COORD2,
	/**Index 5 will be used as Tex coord unit 3.*/
	VERTEX_ATTRIB_TEX_COORD3,
	/**Index 6 will be used as Normal.*/
	VERTEX_ATTRIB_NORMAL,
	/**Index 7 will be used as Blend weight for hardware skin.*/
	VERTEX_ATTRIB_BLEND_WEIGHT,
	/**Index 8 will be used as Blend index.*/
	VERTEX_ATTRIB_BLEND_INDEX,
	/**Index 9 will be used as tangent.*/
	VERTEX_ATTRIB_TANGENT,
	/**Index 10 will be used as Binormal.*/
	VERTEX_ATTRIB_BINORMAL,
	/**particle*/
	VERTEX_ATTRIB_VELOCITY,
	VERTEX_ATTRIB_SIZE,
	VERTEX_ATTRIB_AGE,
	VERTEX_ATTRIB_TYPE,

	VERTEX_ATTRIB_MAX,

	// backward compatibility
	VERTEX_ATTRIB_TEX_COORDS = VERTEX_ATTRIB_TEX_COORD,
};
enum CCVetexClass
{
	FLOAT_C1 = 0,
	FLOAT_C2,
	FLOAT_C3,
	FLOAT_C4,
	SHORT_C4,
};

struct D3D11SOLayout
{
	uint32_t Stream;
	std::string SemanticName;
	uint32_t SemanticIndex;
	uint8_t StartComponent;
	uint8_t ComponentCount;
	uint8_t OutputSlot;
};

struct CCVetexAttribute
{
	CCVetexData m_data;
	CCVetexClass m_class;
};

class GLProgramBase;

struct D3DShaderMacro
{
	std::string Name;
	std::string Definition;
};


class ShaderRHI
{
public:
	ShaderRHI();
	virtual ~ShaderRHI();

	virtual bool InitShader(const std::string& str_path, CCVetexAttribute* pAttribs, int nAttri, bool seperate = false) { return false; }
	virtual bool InitShader(const std::string& vs_path, const std::string& ps_path) { return false; }
	virtual bool InitShaderWithString(const char* szShader) { return false; }
	virtual bool initGeometryWithStreamOutput(const std::string& fileName, CCVetexAttribute* pAttribs, int nAttri, const std::string& VSMain, const std::string& GSMain, const std::vector< D3D11SOLayout>& SOLayouts) {
		return false;
	}
	virtual bool initGeometryWithFile(const std::string& fileName, CCVetexAttribute* pAttribs, int nAttri, const std::string& VSMain, const std::string& GSMain, const std::string& PSMain) {
		return false;
	}

	virtual bool initVSShader(const std::string& fileName, const std::string& VSMain, bool InitInputLayout, const std::vector< D3DShaderMacro>& MacroDefines = {}) { return false; }
	virtual bool initPSShader(const std::string& fileName, const std::string& PSMain, const std::vector< D3DShaderMacro>& MacroDefines = {}) { return false; }
	virtual bool initGeometryShader(const std::string& fileName, const std::string& GSMain, const std::vector< D3DShaderMacro>& MacroDefines = {}) { return false; }
	virtual bool initComputeShader(const std::string& fileName, const std::string& CSMain, const std::vector< D3DShaderMacro>& MacroDefines = {}) { return false; }

	virtual void UseShader() = 0;
	virtual std::shared_ptr< GLProgramBase> GetGLProgram();
};