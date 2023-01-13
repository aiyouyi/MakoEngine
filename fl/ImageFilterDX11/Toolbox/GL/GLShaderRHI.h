#include "Toolbox/Render/ShaderRHI.h"

class CCProgram;

class GLShaderRHI : public ShaderRHI
{
public:
	GLShaderRHI();
	virtual ~GLShaderRHI();
	virtual bool InitShader(const std::string& vs_path, const std::string& ps_path) override;
	virtual void UseShader() override;
	virtual std::shared_ptr< GLProgramBase> GetGLProgram() override;

private:
	std::shared_ptr<CCProgram> Shader;
};