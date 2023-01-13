//
//  CCEffectModel.hpp
//  com.test.cc.ruf
//
//  Created by cc on 2017/6/14.
//

#ifndef CCEffectModel_hpp
#define CCEffectModel_hpp

#include "BaseDefine/Vectors.h"
#include "Toolbox/Drawable.hpp"
#include  <string>
#include  <cmath>

class CCHeaderModel
{
public:
    CCHeaderModel(){}
    
    Vector3 *m_arrPos;
    int m_nVertex;
    
    int m_nFaces;
    short *m_arrIndex;
};

class CCEffectModel
{
public:
    CCEffectModel();
    ~CCEffectModel();
    
    Vector3 *m_arrPos;
    Vector2 *m_arrUV;
    int m_nVertex;
    
    int m_nFaces;
    short *m_arrIndex;
    
    //unsigned int m_texID;
    //string m_szTexture;
    Drawable *m_drawable;
    bool m_bTransparent;
    Vector4 m_fMixColor;
    
public:
    struct ModeComp
    {
        bool operator()(const CCEffectModel *modelL, const CCEffectModel *modelR)
        {
            float zL = 0.0f;
            if(modelL->m_nVertex > 0)
            {
                for(int i=0; i<modelL->m_nVertex; ++i)
                {
                    zL += modelL->m_arrPos[i].z;
                }
                zL /= modelL->m_nVertex;
            }
            
            float zR = 0.0f;
            if(modelR->m_nVertex > 0)
            {
                for(int i=0; i<modelR->m_nVertex; ++i)
                {
                    zR += modelR->m_arrPos[i].z;
                }
                zR /= modelR->m_nVertex;
            }
            
            return zL < zR;
        }
    };
};

#define DESIGN_BASESIZE 720
enum en_AlignParentType{
    EAPT_LT = 0,  
    EAPT_LB = 1,
    EAPT_RT = 2,
    EAPT_RB = 3,
    EAPT_CT=4,
    EAPT_CB=5,
    EAPT_LC=6,
    EAPT_RC=7,
    EAPT_CC=8,
    EAPT_MAX
};
class CCEffect2DRect
{
public:
    CCEffect2DRect();
    ~CCEffect2DRect();
    void updateRenderInfo(int nTargetWidth, int nTargetHeight);
    
    void setRect(int x, int y, int width, int height, int AlignType);
    
    void invalidate();

    void copy(const CCEffect2DRect& CCEffect2DRect);
    
    void destory();
public:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    int m_nAlignType;
    
    int m_nTargetWidth;
    int m_nTargetHeight;
    
    bool m_bNeedUpdate;
    
    Vector2 *m_arrPos;
    Vector2 *m_arrUV;
    short *m_arrIndex;
    
    Drawable* m_drawable;
};

#endif /* CCEffectModel_hpp */
