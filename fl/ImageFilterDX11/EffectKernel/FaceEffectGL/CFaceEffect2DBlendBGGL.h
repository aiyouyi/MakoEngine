#pragma once
#include <vector>
#include <memory>
#include "EffectKernel/CEffectPart.h"
#include "Algorithm/Face2DMesh/MTFace2DInterFace.h"

class Drawable;

class CFaceEffect2DBlendBGGL : public CEffectPart
{
public:
	CFaceEffect2DBlendBGGL();
	virtual ~CFaceEffect2DBlendBGGL();
	virtual void Release();
	virtual void* Clone();
	virtual bool ReadConfig(XMLNode& childNode, HZIP hZip = 0,char *pFilePath=NULL);
	virtual bool Prepare();
	virtual void Render(BaseRenderParam& RenderParam);

	bool DrawTriLineToFBO(GLfloat* points, int count, unsigned short* indexs);
public:
	std::shared_ptr<Drawable> m_Drawable;

private:
	std::shared_ptr<class CCProgram> m_pShader;
	mt3dface::MTFace2DInterFace *m_2DInterFace;

};

