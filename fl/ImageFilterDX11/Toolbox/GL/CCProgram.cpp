#include "CCProgram.h"

CCProgram::CCProgram()
{
    m_programObjID = 0;
    m_locationMap.clear();
}

CCProgram::~CCProgram()
{
    GL_DELETE_PROGRAM(m_programObjID);
    m_locationMap.clear();
}

void CCProgram::CreatePorgramForFile(const char *vs_file, const char *fs_file)
{
    GL_DELETE_PROGRAM(m_programObjID);
    m_programObjID = OpenGLUtil::CreatePorgramForFile(vs_file, fs_file);
}

void CCProgram::CreatePorgramForText(const char *vs_text, const char *fs_text)
{
    GL_DELETE_PROGRAM(m_programObjID);
    m_programObjID = OpenGLUtil::CreatePorgramForText(vs_text,fs_text);
}


bool CCProgram::IsUseful()
{
    if (m_programObjID == 0)
    {
        return false;
    }

    return true;
}

void CCProgram::Use()
{
    glUseProgram(m_programObjID);
}

GLuint CCProgram::GetProgramID()
{
    return m_programObjID;
}

GLint CCProgram::GetUniformLocation(const char* str)
{
    std::string temp(str);
    if (m_locationMap.find(temp) != m_locationMap.end())
    {
        return m_locationMap[temp];
    }

    GLint ptr = glGetUniformLocation(this->m_programObjID, str);

    //if (ptr != -1)
    {
        m_locationMap.insert(std::map<std::string, GLint>::value_type(std::string(str), ptr));
    }

    return ptr;
}

GLint CCProgram::GetAttribLocation(const char* str)
{
    std::string temp(str);
    if (m_locationMap.find(temp) != m_locationMap.end())
    {
        return m_locationMap[temp];
    }

    GLint ptr = glGetAttribLocation(this->m_programObjID, str);

    //if (ptr != -1)
    {
        m_locationMap.insert(std::map<std::string, GLint>::value_type(std::string(str), ptr));
    }

    return ptr;
}

void CCProgram::SetUniform1i(const char* str, GLuint value)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform1i(ptr, value);
    }
    else
    {
        //LOGE("CCProgram::SetUniform1i there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniform1iv(const char* str, const GLint *array, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform1iv(ptr, count, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniform1i there is no uniform called: %s , m_programObjID = %d", str, m_programObjID);
    }
}

void CCProgram::SetUniform3f(const char * str, float x, float y, float z)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform3f(ptr, x,y,z);
    }
    else
    {

       // LOGE("CCProgram::SetUniform3f there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniform3fv(const char * str, const GLfloat *array, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform3fv(ptr, count, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniform3fv there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniform4fv(const char * str, const GLfloat *array, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform4fv(ptr, count, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniform4fv there is no uniform called: %s , m_programObjID = %d", str, m_programObjID);
    }
}

void CCProgram::SetUniformMatrix4fv(const char *str, const float *array, bool transpose, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniformMatrix4fv(ptr,count,transpose,array);
    }
    else
    {
        //LOGE("CCProgram::SetUniformMatrix4fv there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniformMatrix3fv(const char *str, const GLfloat *array, bool transpose, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniformMatrix3fv(ptr, count, transpose, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniformMatrix3fv there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniformMatrix2fv(const char *str, const GLfloat *array, bool transpose, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniformMatrix2fv(ptr, count, transpose, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniformMatrix2fv there is no uniform called: %s , m_programObjID = %d", str, m_programObjID);
    }
}

void CCProgram::SetVertexAttribPointer(const char *str, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    GLint ptr = this->GetAttribLocation(str);
    if (ptr != -1)
    {
        glEnableVertexAttribArray(ptr);
        glVertexAttribPointer(ptr, size, type, normalized, stride, pointer);
    }
    else
    {
        //LOGE("CCProgram::SetVertexAttribPointer there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::DisableVertexAttribPointer(const char *str)
{
    GLint ptr = this->GetAttribLocation(str);
    if (ptr != -1)
    {
        glDisableVertexAttribArray(ptr);
    }
    else
    {
       // LOGE("CCProgram::DisableVertexAttribPointer there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniform1f(const char* str, float value)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform1f(ptr, value);
    }
    else
    {
        //LOGE("CCProgram::SetUniform1f there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
   }
}

void CCProgram::SetUniform2f(const char* str, float x, float y)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform2f(ptr, x, y);
    }
    else
    {
       // LOGE("CCProgram::SetUniform2f there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniform4f(const char* str, float r, float g, float b, float a)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform4f(ptr, r, g, b, a);
    }
    else
    {
        //LOGE("CCProgram::SetUniform4f there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

void CCProgram::SetUniform1fv(const char* str, const GLfloat *array, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform1fv(ptr, count, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniform1fv there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}


void CCProgram::SetUniform2fv(const char* str, const GLfloat *array, int count)
{
    GLint ptr = this->GetUniformLocation(str);
    if (ptr != -1)
    {
        glUniform2fv(ptr, count, array);
    }
    else
    {
        //LOGE("CCProgram::SetUniform2fv there is no uniform called: %s , m_programObjID = %d", str,m_programObjID);
    }
}

