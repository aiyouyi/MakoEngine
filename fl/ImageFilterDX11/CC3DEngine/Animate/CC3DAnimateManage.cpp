
#include "CC3DAnimateManage.h"
// #include <opencv2/core/core.hpp>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>


CC3DAnimateManage::CC3DAnimateManage()
{
}

CC3DAnimateManage::~CC3DAnimateManage()
{
	for (int i = 0; i < m_Animate.size(); i++)
	{
		SAFE_DELETE(m_Animate[i]);
	}
	m_Animate.clear();
}

void CC3DAnimateManage::InitAnimate()
{
	for (int i = 0; i < m_Model->animations.size(); i++)
	{
		CC3DAnimate *pAnimate = new CC3DAnimate();
		pAnimate->initModel(m_Model);
		pAnimate->InitAnimate(i);
		m_Animate.push_back(pAnimate);

		m_AnimateAllTime = (std::max)(m_AnimateAllTime, pAnimate->m_AnimateAllTime);
	}
}

void CC3DAnimateManage::ReadSkeletonAnimate(CC3DModel *pModel, CC3DSkeleton * pSkeleton, const char * pAnimateFile)
{
	CC3DAnimate *pAnimate = new CC3DAnimate();
	pAnimate->initModel(m_Model);
	pAnimate->ReadSkeletonAnimate(pModel,pSkeleton, pAnimateFile);
	m_Animate.push_back(pAnimate);
	m_AnimateAllTime = (std::max)(m_AnimateAllTime, pAnimate->m_AnimateAllTime);
}

void CC3DAnimateManage::play(float fSecond, CC3DModel * pModel, CC3DSkeleton *pSkeleton)
{
	if (m_AnimateAllTime < 0.001)
	{
		return ;
	}

	int nTmp = fSecond / m_AnimateAllTime;
	
	float during = fSecond - nTmp * m_AnimateAllTime;

	//during = frameCount * (1000.0f / 40.0f) / 1000.0f;
	frameCount++;

	for (int i=0;i<m_Animate.size();i++)
	{
		m_Animate[i]->play(during, pModel, pSkeleton);
		if (m_Animate[i]->hasModelAnimate)
		{
			m_hasModelAnimate = true;
		}

	}
	
}

void CC3DAnimateManage::playOnce(float fSecond, CC3DModel * pModel, CC3DSkeleton * pSkeleton)
{
	if (m_AnimateAllTime < 0.001)
	{
		m_hasModelAnimate = false;
		return;
	}
	for (int i = 0; i < m_Animate.size(); i++)
	{
		if (fSecond<m_Animate[i]->m_AnimateAllTime)
		{
			m_Animate[i]->play(fSecond, pModel, pSkeleton);
			if (m_Animate[i]->hasModelAnimate)
			{
				m_hasModelAnimate = true;
			}
		}


	}
}
