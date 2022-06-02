//
//  EffectModel.hpp
//  com.test.cc.ruf
//
//  Created by cc on 2017/6/14.
//  Copyright © 2017年 cc. All rights reserved.
//

#ifndef EffectModel_hpp
#define EffectModel_hpp
#include <d3d11.h>
#include "Drawable.hpp"
#include "mathlib.h"
#include <string>
#include <xnamath.h>
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11FBO.h"
using namespace std;


class HeaderModel
{
public:
	HeaderModel();

	void destory();

	void updateGpuBuffer();

	vec3 *m_arrPos;
	int m_nVertex;

	int m_nFaces;
	unsigned short *m_arrIndex;

	//渲染buffer
	ID3D11Buffer *m_headerVerticeBuffer;
	ID3D11Buffer *m_headerIndexBuffer;
};

struct EffectVertex
{
	vec3 vPos;
	vec2 vUV;
};
class EffectModel
{
public:
    EffectModel();

	void destory();

	EffectVertex *m_arrVertices;
    int m_nVertex;
    
    int m_nFaces;
    unsigned short *m_arrIndex;
    
    Drawable *m_drawable;

	bool m_bTransparent;
	vec4 m_fMixColor;
    
	int m_Layer;

	//渲染buffer
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;
public:
    struct ModeComp
    {
        bool operator()(const EffectModel &modelL, const EffectModel &modelR)
        {
            float zL = 0.0f;
            if(modelL.m_nVertex > 0)
            {
                for(int i=0; i<modelL.m_nVertex; ++i)
                {
                    zL += modelL.m_arrVertices[i].vPos.z;
                }
                zL /= modelL.m_nVertex;
            }
            
            float zR = 0.0f;
            if(modelR.m_nVertex > 0)
            {
                for(int i=0; i<modelR.m_nVertex; ++i)
                {
                    zR += modelR.m_arrVertices[i].vPos.z;
                }
                zR /= modelR.m_nVertex;
            }
            
            return zL > zR;
        }
    };
};


#define DESIGN_BASESIZE 720
enum en_AlignParentType {
	EAPT_LT = 0,  //左上对齐
	EAPT_LB = 1,  //左下对齐
	EAPT_RT = 2,  //右上对齐
	EAPT_RB = 3,   //右下对齐
	EAPT_CT = 4,     //中上对齐
	EAPT_CB = 5,     //中下对齐
	EAPT_LC = 6,     //左中对齐
	EAPT_RC = 7,     //右中对齐
	EAPT_CC = 8,     //中心对齐
	EAPT_SCALE = 9,  //按比例缩放
	EAPT_MAX
};

class Effect2DRect
{
public:
	Effect2DRect();
	~Effect2DRect();

	void updateRenderInfo(int nTargetWidth, int nTargetHeight);

	void updateRenderInfo2(int nTargetWidth, int nTargetHeight);


	void setRect(int x, int y, int width, int height, int AlignType, bool bAutoResize=true);

	void setLayer(int layer);

	void invalidate();

	void destory();

	void render(int width, int height, float during);

	bool prepare();

	struct EffectConstantBuffer
	{
		XMMATRIX mWVP; //混合矩阵
		XMVECTOR mColor;
	};
public:
	int m_Layer;

	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_nAlignType;

	int m_nTargetWidth;
	int m_nTargetHeight;

	bool m_bNeedUpdate;
	bool m_bAutoResize;

	EffectVertex *m_arrVertices;
	unsigned short *m_arrIndex;

	Drawable *m_drawable;
	vec4 m_fMixColor;

	//渲染buffer
	ID3D11Buffer *m_rectVerticeBuffer;
	ID3D11Buffer *m_rectIndexBuffer;


	//采样状态
	ID3D11SamplerState* m_pSamplerLinear;

	//混合以及buffer操作
	ID3D11BlendState *m_pBSEnable;
	ID3D11BlendState *m_pBSDisable;

	//深度操作
	ID3D11DepthStencilState *m_pDepthStateDisable;

	//渲染模型信息
	DX11Shader *m_pShader;
	ID3D11Buffer*       m_pConstantBuffer;  //转换矩阵

	bool m_bPrepare;
};

#endif /* EffectModel_hpp */
