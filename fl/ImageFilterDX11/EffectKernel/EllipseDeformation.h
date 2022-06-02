#pragma once
#ifndef __ELLIPSE_DEFORMATION_H__
#define __ELLIPSE_DEFORMATION_H__
#include "DX11ImageFilterDef.h"
#include "Algorithm/StackBlur.h"
#include "BaseDefine/Vectors.h"
#include "BaseDefine/Define.h"
#include "Toolbox/xmlParser.h"
#include "Toolbox/stb_image_write.h"
#include <vector>
#include "math.h"
#include <algorithm>

struct EllipseRect
{
	float left;
	float top;
	float width;
	float height;
};

class MAGICSTUDIO_EXPORTS_CLASS EllipseDeformation
{

public:
	EllipseDeformation();

	~EllipseDeformation();

	void SetEllipseControlPoint(EllipseRect &rect, bool iSymmetry);

	BYTE* GetOffsetTexture();

	void updateOffset();

	void SetLiquifyFactorX(float &xRadius);

	void SetLiquifyFactorY(float &yRadius);

	void ReadConfig(XMLNode &childNode);

	void WriteConfig(XMLNode &nodeEffect);

	void SetSymmetryAxis(float x);
private:
	void Liquify();
	void LiquifySymmetry();
	bool InEllipse(float x, float y, float ellipse_left, float ellipse_top, float ellipse_width, float ellipse_height);

public:
	Vector2 *m_pShiftRGBA;
	BYTE *m_pOffestTexture;

	EllipseRect m_EllipseRect;
	float m_LiquifyFactorX = 1.0;
	float m_LiquifyFactorY = 1.0;
	float m_LiquifyAlpha = 1.0;
	bool m_iSymmetry = false;
	float m_AxisX = 0.6;

private:
	
	float m_SmoothRadius = 2.0;

	int m_SmallTextureHeight = 80;
	int m_SmallTextureWidth = 80;
	int nWidth = 1500;
	int nHeight = 1500;



};





#endif