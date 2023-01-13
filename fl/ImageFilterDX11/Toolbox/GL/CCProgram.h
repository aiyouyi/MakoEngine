#ifndef CCPROGRAM_H
#define CCPROGRAM_H

#include"openglutil.h"
#include <map>
#include <string>
#include "GLProgramBase.h"

class CCProgram : public GLProgramBase
{
public:
    CCProgram();
    virtual ~CCProgram();

    void CreatePorgramForText(const char * vs_text, const char * fs_text);

    void CreatePorgramForFile(const char * vs_file, const char * fs_file);


    ///-------------------------------------------------------------------------------------------------
    /// @fn	virtual bool Program::IsUseful();
    ///
    /// @brief	查询program是否可用.
    ///
    /// @return	True if useful, false if not.
    ///-------------------------------------------------------------------------------------------------

    virtual bool IsUseful();

    ///-------------------------------------------------------------------------------------------------
    /// @fn	virtual void Program::Use();
    ///
    /// @brief	使用program.
    ///-------------------------------------------------------------------------------------------------

    virtual void Use();

    ///-------------------------------------------------------------------------------------------------
    /// @fn	virtual GLuint Program::GetProgramID();
    ///
    /// @brief	获取program ID.
    ///
    /// @return	返回program ID.
    ///-------------------------------------------------------------------------------------------------

    virtual GLuint GetProgramID();

    virtual void SetUniform1i(const char* str, uint32_t value);

    virtual void SetUniform1iv(const char* str, const GLint *array, int count);

    virtual void SetUniform3f(const char * str, float x, float y, float z);

    virtual void SetUniform3fv(const char * str, const GLfloat *array, int count);

    virtual void SetUniformMatrix4fv(const char *str, const float *array, bool transpose, int count);

    virtual void SetUniformMatrix3fv(const char *str, const GLfloat *array, bool transpose, int count);

    virtual void SetUniformMatrix2fv(const char *str, const GLfloat *array, bool transpose, int count);

    virtual void SetVertexAttribPointer(const char *str, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

    virtual void DisableVertexAttribPointer(const char *str);

    virtual void SetUniform1f(const char* str, float value);

    virtual void SetUniform2f(const char* str, float x, float y);

    virtual void SetUniform4f(const char* str, float r, float g, float b, float a);

    virtual void SetUniform4fv(const char * str, const GLfloat *array, int count);

    virtual void SetUniform1fv(const char* str, const GLfloat *array, int count);

    virtual void SetUniform2fv(const char* str, const GLfloat *array, int count);


    ///-------------------------------------------------------------------------------------------------
    /// @fn	GLint Program::GetUniformLocation(const char* str);
    ///
    /// @brief	Gets uniform location.
    ///
    /// @param	str	The string.
    ///
    /// @note	内存缓存结果，避免反复查询
    ///
    /// @return	The uniform location.
    ///-------------------------------------------------------------------------------------------------

    GLint GetUniformLocation(const char* str);

    ///-------------------------------------------------------------------------------------------------
    /// @fn	GLint Program::GetAttribLocation(const char* str);
    ///
    /// @brief	Gets attribute location.
    ///
    /// @param	str	The string.
    ///
    /// @note	内存缓存结果，避免反复查询
    ///
    /// @return	The attribute location.
    ///-------------------------------------------------------------------------------------------------

    GLint GetAttribLocation(const char* str);
protected:
    GLuint m_programObjID;
    std::map<std::string, GLint> m_locationMap;
};

#endif // CCPROGRAM_H
