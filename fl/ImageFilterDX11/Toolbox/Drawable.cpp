//
//  Drawable.cpp
//  FaceDetector
//
//  Created by cc on 2017/6/28.
//  Copyright © 2017年 cc. All rights reserved.
//

#include "Drawable.hpp"
#include "Toolbox/Render/TextureRHI.h"
#include "Toolbox/DXUtils/DX11Resource.h"

BitmapDrawable::BitmapDrawable(std::shared_ptr<MaterialTexRHI> TexRHI)
	:m_MaterialTexRHI(TexRHI)
{

}

BitmapDrawable::BitmapDrawable(std::shared_ptr<CC3DTextureRHI> pTex)
	:m_TexRHI(pTex)
{

}

BitmapDrawable::~BitmapDrawable()
{
	
}
    

void BitmapDrawable::getSize(int &w, int &h)
{
	if (m_MaterialTexRHI != NULL)
	{
		w = m_MaterialTexRHI->GetWidth();
		h = m_MaterialTexRHI->GetHeight();
	}
	else
	{
		if (m_TexRHI)
		{
			w = m_TexRHI->GetWidth();
			h = m_TexRHI->GetHeight();
		}
		else
		{
			w = 0;
			h = 0;
		}
	}
}

Drawable *BitmapDrawable::Clone()
{
	if (m_MaterialTexRHI)
	{
		return new BitmapDrawable(m_MaterialTexRHI);
	}
	else
	{
		return new BitmapDrawable(m_TexRHI);
	}
}


AnimationDrawable::AnimationDrawable()
{
	m_bRunning = false; 
	m_offset = 0;
	m_effectLoopMode = ELM_REPEAT;
}
AnimationDrawable::~AnimationDrawable()
{
    
    m_aniInfo.clear();
}
    
void AnimationDrawable::start()
{
    m_bRunning = true;
}

void AnimationDrawable::stop()
{
    m_bRunning = false;
}
    
void AnimationDrawable::appandTex(long during, std::shared_ptr<MaterialTexRHI> pTex)
{
	if (pTex == NULL)return;

    AniPair ap;
    ap.MatTexRHI = pTex;
    
    if(m_aniInfo.size() > 0)
    {
        ap.during = m_aniInfo[m_aniInfo.size()-1].during+during;
    }
    else
    {
        ap.during = during;
    }
    
    m_aniInfo.push_back(ap);
}

void AnimationDrawable::appandTex(long during, std::shared_ptr<CC3DTextureRHI> pTex)
{
	if (pTex == NULL)return;

	AniPair ap;
	ap.TexRHI = pTex;

	if (m_aniInfo.size() > 0)
	{
		ap.during = m_aniInfo[m_aniInfo.size() - 1].during + during;
	}
	else
	{
		ap.during = during;
	}

	m_aniInfo.push_back(ap);
}

void AnimationDrawable::setLoopMode(en_EffectLoop_Mode effectLoopMode)
{
	m_effectLoopMode = effectLoopMode;
}

long AnimationDrawable::getDuring()
{
	if (ELM_REPEAT == m_effectLoopMode)
	{
		return -1;
	}

	if (m_aniInfo.size() > 0)
	{
		return m_aniInfo[m_aniInfo.size() - 1].during + m_offset;
	}

	return -1;
}


std::shared_ptr<MaterialTexRHI> AnimationDrawable::GetTex(long during)
{
	if (m_aniInfo.size() == 0) return 0;

	if (during < m_offset) return 0;

	if (m_aniInfo[m_aniInfo.size() - 1].during > 0)
	{
		while (during < m_offset)
		{
			during = during + m_aniInfo[m_aniInfo.size() - 1].during;
		}
	}
	else
	{
		if (during < m_offset)
		{
			return 0;
		}
	}


	during -= m_offset;

	if (m_effectLoopMode == ELM_REPEAT)
	{
		during = during % m_aniInfo[m_aniInfo.size() - 1].during;
	}
	else
	{
		if (during > m_aniInfo[m_aniInfo.size() - 1].during)
		{
			during = m_aniInfo[m_aniInfo.size() - 1].during;
		}
	}

	std::shared_ptr<MaterialTexRHI> TextureRHI;
	if (m_aniInfo[0].MatTexRHI != NULL)
	{
		TextureRHI = m_aniInfo[0].MatTexRHI;
	}
	for (int i = 0; i < m_aniInfo.size(); ++i)
	{
		if (during <= m_aniInfo[i].during)
		{
			if (m_aniInfo[i].MatTexRHI != NULL)
			{
				TextureRHI = m_aniInfo[i].MatTexRHI;
			}

			break;
		}
	}

	return TextureRHI;
}

std::shared_ptr<CC3DTextureRHI> AnimationDrawable::GetTexRHI(long during)
{
	if (m_aniInfo.size() == 0) return 0;

	if (during < m_offset) return 0;

	if (m_aniInfo[m_aniInfo.size() - 1].during > 0)
	{
		while (during < m_offset)
		{
			during = during + m_aniInfo[m_aniInfo.size() - 1].during;
		}
	}
	else
	{
		if (during < m_offset)
		{
			return 0;
		}
	}


	during -= m_offset;

	if (m_effectLoopMode == ELM_REPEAT)
	{
		during = during % m_aniInfo[m_aniInfo.size() - 1].during;
	}
	else
	{
		if (during > m_aniInfo[m_aniInfo.size() - 1].during)
		{
			during = m_aniInfo[m_aniInfo.size() - 1].during;
		}
	}

	std::shared_ptr<CC3DTextureRHI> TextureRHI;
	if (m_aniInfo[0].TexRHI != NULL)
	{
		TextureRHI = m_aniInfo[0].TexRHI;
	}
	for (int i = 0; i < m_aniInfo.size(); ++i)
	{
		if (during <= m_aniInfo[i].during)
		{
			if (m_aniInfo[i].TexRHI != NULL)
			{
				TextureRHI = m_aniInfo[i].TexRHI;
			}

			break;
		}
	}

	return TextureRHI;
}


void AnimationDrawable::getSize(int &w, int &h)
{
	if (m_aniInfo.size() > 0)
	{
		if (m_aniInfo[0].MatTexRHI)
		{
			w = m_aniInfo[0].MatTexRHI->GetWidth();
			h = m_aniInfo[0].MatTexRHI->GetHeight();
		}
		else if(m_aniInfo[0].TexRHI)
		{
			w = m_aniInfo[0].TexRHI->GetWidth();
			h = m_aniInfo[0].TexRHI->GetHeight();
		}
		else
		{
			w = 0;
			h = 0;
		}
	}
	else
	{
		w = 0;
		h = 0;
	}
}

void AnimationDrawable::setOffset(long offset)
{
    m_offset = offset;
}

Drawable *AnimationDrawable::Clone()
{
	//for (int i = 0; i<m_aniInfo.size(); ++i)
	//{
	//	m_aniInfo[i].pTex->ref();
	//}

    AnimationDrawable *drawable = new AnimationDrawable();
    (*drawable) = (*this);
    return drawable;
}


