#include "CLUTFIlterGL.h"
#include "BaseDefine/commonFunc.h"
#include "Toolbox/GL/GLResource.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include <windows.h>

bool saveAsBMP(const char* pData, uint32_t nLen, int w, int h, int bitCount, const wchar_t* pszFileName)
{
	// Writes a BMP file
	// save to file
	HANDLE hFile = ::CreateFileW(pszFileName,            // file to create 
		GENERIC_WRITE,                // open for writing 
		0,                            // do not share 
		NULL,                         // default security 
		OPEN_ALWAYS,                  // overwrite existing 
		FILE_ATTRIBUTE_NORMAL,        // normal file 
		NULL);                        // no attr. template 
	if (!hFile || hFile == INVALID_HANDLE_VALUE)
	{
		return false;	// 
	}

	DWORD dwSizeBytes = nLen;

	// fill in the headers
	BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4D42; // 'BM'
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwSizeBytes;//整个文件的大小
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//图像数据偏移量，即图像数据在文件中的保存位置

	DWORD dwBytesWritten;
	::WriteFile(hFile, &bmfh, sizeof(bmfh), &dwBytesWritten, NULL);
	if (dwBytesWritten != sizeof(bmfh))
	{
	}

	BITMAPINFOHEADER bmih;

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = w;
	bmih.biHeight = -h;
	bmih.biPlanes = 1; // 图像的目标显示设备的位数，通常为1
	bmih.biBitCount = bitCount; // 每个像素的位数，可以为1、4、8、24、32
	bmih.biCompression = BI_RGB;// 是否压缩
	bmih.biSizeImage = 0;//图像大小的字节数
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	::WriteFile(hFile, &bmih, sizeof(bmih), &dwBytesWritten, NULL);
	if (dwBytesWritten != sizeof(bmih))
	{
	}

	::WriteFile(hFile, pData, dwSizeBytes, &dwBytesWritten, NULL);
	if (dwBytesWritten != dwSizeBytes)
	{
	}

	::CloseHandle(hFile);
	return true;
}

CLUTFIlterGL::CLUTFIlterGL()
{
    m_alpha = 0.0;
    m_EffectPart = LUT_FILTER_EFFECT;

}


CLUTFIlterGL::~CLUTFIlterGL()
{
    Release();
}

void CLUTFIlterGL::Release()
{
}

void * CLUTFIlterGL::Clone()
{
    CLUTFIlterGL* result = new CLUTFIlterGL();
    *result = *this;
    return result;
}

bool CLUTFIlterGL::ReadConfig(XMLNode & childNode, HZIP hZip , char *pFilePath)
{
    if (!childNode.isEmpty())
    {
        XMLNode nodeDrawable = childNode.getChildNode("drawable", 0);
        if (!nodeDrawable.isEmpty())
        {
            const char *szDrawableName = nodeDrawable.getAttribute("Material");
            if (szDrawableName != NULL)
            {
                m_material = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
            }
            const char *szAlpha = nodeDrawable.getAttribute("Alpha");
            if(szAlpha!=NULL)
            {
                m_alpha = atof(szAlpha);
            }

			szDrawableName = nodeDrawable.getAttribute("MaterialAdjust");
			if (szDrawableName != NULL)
			{
                m_materialAdjust = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
			}
			szDrawableName = nodeDrawable.getAttribute("MaterialClear");
			if (szDrawableName != NULL)
			{
                m_materialClear = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
			}
			szDrawableName = nodeDrawable.getAttribute("MaterialType");
			if (szDrawableName != NULL)
			{
                m_materialType = GetDynamicRHI()->CreateAsynTextureZIP(hZip, szDrawableName);
			}
        }
        return true;
    }
    return false;
}

bool CLUTFIlterGL::Prepare()
{
    m_pShader = std::make_shared<CCProgram>();
    std::string  vspath = m_resourcePath + "/Shader/LUTFilter.vs";
    std::string  fspath = m_resourcePath + "/Shader/LUTFilter.fs";
    m_pShader->CreatePorgramForFile(vspath.c_str(),fspath.c_str());
    return true;
}

void CLUTFIlterGL::Render(BaseRenderParam& RenderParam)
{

    if(m_alpha<0.001f&&m_alphaClear<0.001f&&m_alphaAdjust<0.001f&&m_alphaType<0.001f)
    {
        return;
    }

    static const GLfloat g_TextureCoordinate[] = {0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f};
    auto pDoubleBuffer = RenderParam.GetDoubleBuffer();
	pDoubleBuffer->SwapFBO();

    pDoubleBuffer->BindFBOA();
    glViewport(0,0,RenderParam.GetWidth(), RenderParam.GetHeight());
    m_pShader->Use();

	m_material->UpdateTexture();
	m_materialAdjust->UpdateTexture();
	m_materialClear->UpdateTexture();
	m_materialType->UpdateTexture();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, RHIResourceCast(pDoubleBuffer.get())->GetFBOTextureBID());
	m_pShader->SetUniform1i("inputImageTexture", 0);

	m_material->Bind(1);
	m_pShader->SetUniform1i("inputImageTexture2", 1);

    m_materialAdjust->Bind(2);
	m_pShader->SetUniform1i("inputImageTextureAdjust", 2);

    m_materialClear->Bind(3);
	m_pShader->SetUniform1i("inputImageTextureClear", 3);

    m_materialType->Bind(4);
	m_pShader->SetUniform1i("inputImageTextureType", 4);
    
    m_pShader->SetVertexAttribPointer("inputTextureCoordinate", 2, GL_FLOAT, false, 0, g_TextureCoordinate);
    m_pShader->SetUniform1f("alpha", m_alpha);
	m_pShader->SetUniform1f("alphaAdjust", m_alphaAdjust);
	m_pShader->SetUniform1f("alphaClear", m_alphaClear);
	m_pShader->SetUniform1f("alphaType", m_alphaType);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_pShader->DisableVertexAttribPointer("inputTextureCoordinate");

	pDoubleBuffer->unBindFBOA();

}
