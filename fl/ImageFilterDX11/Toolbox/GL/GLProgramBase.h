#pragma once
#include "Toolbox/inc.h"

class GLProgramBase 
{
public:
	GLProgramBase();
	virtual ~GLProgramBase();

	virtual void SetUniform1i(const char* str, uint32_t value) = 0;
	virtual void SetUniform1f(const char* str, float value) = 0;
	virtual void SetUniform2f(const char* str, float x, float y) = 0;
	virtual void SetUniform3f(const char* str, float x, float y, float z) = 0;
	virtual void SetUniform4f(const char* str, float x, float y, float z, float w) = 0;
	virtual void SetUniformMatrix4fv(const char* str, const float* array, bool transpose, int count) = 0; 
	virtual void Use() = 0;
};