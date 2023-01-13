//
//  CCEffectModel.cpp
//  com.test.cc.ruf
//
//  Created by cc on 2017/6/14.
//

#include "CCEffectModel.hpp"
#include <string.h>
#include <assert.h>
CCEffectModel::CCEffectModel()
{
    m_arrPos = 0;
    m_arrUV = 0;
    m_nVertex = 0;
    
    m_nFaces = 0;
    m_arrIndex = 0;
    
    m_drawable = 0;
    
    m_bTransparent = true;
    m_fMixColor = Vector4(1,1,1,1);
}
CCEffectModel::~CCEffectModel()
{
    SAFE_DELETE_ARRAY(m_arrPos);
    SAFE_DELETE_ARRAY(m_arrUV);
    SAFE_DELETE_ARRAY(m_arrIndex);
//    m_drawable->Init();
//    SAFE_DELETE(m_drawable);
}


CCEffect2DRect::CCEffect2DRect()
{
    m_x = 0;
    m_y = 0;
    m_width = 0;
    m_height = 0;
    
    m_nAlignType = EAPT_CC;
    
    m_nTargetWidth = 0;
    m_nTargetHeight = 0;
    
    m_bNeedUpdate = true;
    
    m_arrPos = new Vector2[4];
    memset(m_arrPos, 0, sizeof(Vector2)*4);
    
    m_arrUV = new Vector2[4];
    m_arrUV[0] = Vector2(0,0);
    m_arrUV[1] = Vector2(1,0);
    m_arrUV[2] = Vector2(0,1);
    m_arrUV[3] = Vector2(1,1);
    
    m_arrIndex = new short[6];
    m_arrIndex[0] = 0;
    m_arrIndex[1] = 1;
    m_arrIndex[2] = 2;
    m_arrIndex[3] = 1;
    m_arrIndex[4] = 3;
    m_arrIndex[5] = 2;
    
    m_drawable = NULL;
}
    
void CCEffect2DRect::updateRenderInfo(int nTargetWidth, int nTargetHeight)
{
    assert(nTargetWidth>0);
    assert(nTargetHeight>0);
    
    if(m_nTargetWidth != nTargetWidth || m_nTargetHeight != nTargetHeight || m_bNeedUpdate)
    {
        m_bNeedUpdate = true;
        m_nTargetWidth = nTargetWidth;
        m_nTargetHeight = nTargetHeight;
    }
    
    if(m_bNeedUpdate)
    {
        m_bNeedUpdate = false;
        
        int w = m_width;
        int h = m_height;
        
        float fStepX = 2.0f/DESIGN_BASESIZE;
        float fStepY = fStepX*nTargetWidth/nTargetHeight;
        if(nTargetHeight < nTargetWidth)
        {
            fStepY = 2.0f/DESIGN_BASESIZE;
            fStepX = fStepY*nTargetHeight/nTargetWidth;
        }
        
        float fLBX = 0;
        float fLBY = 0;
        float fLogoW = w*fStepX;
        float fLogoH = h*fStepY;

        switch(m_nAlignType)
        {
            case EAPT_LT:
            {
                fLBX = -1.0f+m_x*fStepX;
                fLBY = -1.0f+m_y*fStepY;
                break;
            }
            case EAPT_LB:
            {
                fLBX = -1.0f+m_x*fStepX;
                fLBY = 1.0f-m_y*fStepY-fLogoH;
                break;
            }
            case EAPT_RT:
            {
                fLBX = 1.0f-m_x*fStepX-fLogoW;
                fLBY = -1.0f+m_y*fStepY;
                break;
            }
            case EAPT_RB:
            {
                fLBX = 1.0f-m_x*fStepX-fLogoW;
                fLBY = 1.0f-m_y*fStepY-fLogoH;
                break;
            }
                
            case EAPT_CT:
            {
                fLBX = m_x*fStepX - fLogoW*0.5f;
                fLBY = -1.0f+m_y*fStepY;
                break;
            }
            case EAPT_CB:
            {
                fLBX = m_x*fStepX - fLogoW*0.5f;
                fLBY = 1.0f-m_y*fStepY-fLogoH;
                break;
            }
                
                
            case EAPT_LC:
            {
                fLBX = -1.0f+m_x*fStepX;
                fLBY = m_y*fStepY - fLogoH*0.5f;
                break;
            }
            case EAPT_RC:
            {
                fLBX = 1.0f-m_x*fStepX-fLogoW;
                fLBY = m_y*fStepY - fLogoH*0.5f;
                break;
            }
            case EAPT_CC:
            {
                fLBX = m_x*fStepX-fLogoW*0.5f;
                fLBY = m_y*fStepY-fLogoH*0.5f;
                break;
            }
        }
        
        m_arrPos[0].x = fLBX;
        m_arrPos[0].y = fLBY;
        
        m_arrPos[1].x = fLBX+fLogoW;
        m_arrPos[1].y = fLBY;
        
        m_arrPos[2].x = fLBX;
        m_arrPos[2].y = fLBY+fLogoH;
        
        m_arrPos[3].x = fLBX+fLogoW;
        m_arrPos[3].y = fLBY+fLogoH;
    }
}

void CCEffect2DRect::setRect(int x, int y, int width, int height, int AlignType)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    
    if(AlignType < EAPT_MAX)
    {
        m_nAlignType = AlignType;
    }
    else
    {
        m_nAlignType = EAPT_CC;
    }
    
    invalidate();
}

void CCEffect2DRect::invalidate()
{
    m_bNeedUpdate = true;
}

CCEffect2DRect::~CCEffect2DRect()
{
    SAFE_DELETE_ARRAY(m_arrPos);
    SAFE_DELETE_ARRAY(m_arrUV);
    SAFE_DELETE_ARRAY(m_arrIndex);
//    m_drawable->Init();
//    SAFE_DELETE(m_drawable);
}
void CCEffect2DRect::destory()
{
    if(m_arrPos != NULL)
    {
        delete []m_arrPos;
        m_arrPos = NULL;
    }
    
    if(m_arrUV != NULL)
    {
        delete []m_arrUV;
        m_arrUV = NULL;
    }
    
    if(m_arrIndex != NULL)
    {
        delete []m_arrIndex;
        m_arrIndex = NULL;
    }
    
}

void CCEffect2DRect::copy(const CCEffect2DRect& effect2DRect) {

    m_x = effect2DRect.m_x;
    m_y = effect2DRect.m_y;
    m_width = effect2DRect.m_width;
    m_height = effect2DRect.m_height;

    m_nAlignType = effect2DRect.m_nAlignType;

    m_nTargetWidth = effect2DRect.m_nTargetWidth;
    m_nTargetHeight = effect2DRect.m_nTargetHeight;

    m_bNeedUpdate = effect2DRect.m_bNeedUpdate;

    m_arrPos = new Vector2[4];
    memcpy(m_arrPos, effect2DRect.m_arrPos, sizeof(Vector2) * 4);

    m_arrUV = new Vector2[4];
    memcpy(m_arrUV, effect2DRect.m_arrUV, sizeof(Vector2) * 4);

    m_arrIndex = new short[6];
    memcpy(m_arrIndex, effect2DRect.m_arrIndex, sizeof(short) * 6);

    m_drawable = effect2DRect.m_drawable;//->Clone();
}

