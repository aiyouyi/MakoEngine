/********************************************************************
	created:	2009/11/26
	created:	26:11:2009   9:47
	filename: 	e:\RDevelopProjects\ProjectSource\MSDGLESEngine\CommonCtrls\IReferenceCounted.h
	file path:	e:\RDevelopProjects\ProjectSource\MSDGLESEngine\CommonCtrls
	file base:	IReferenceCounted
	file ext:	h
	author:		wucongyang
	
	purpose:	控件引用次数类，确保控件指针的安全性
*********************************************************************/
#ifndef IREFERENCE_COUNTED_H
#define IREFERENCE_COUNTED_H
#include "DX11ImageFilterDef.h"
#pragma warning(disable : 4275)

class DX11IMAGEFILTER_EXPORTS_CLASS IReferenceCounted
{
public:
	IReferenceCounted():ReferenceCounter(1)
	{
	}

	virtual ~IReferenceCounted()
	{
	}

	unsigned int GetReferenceCount()
	{
		return ReferenceCounter;
	}
	void ref(){ref_Impl();};
	bool unref(){return unref_Impl();};
protected:

	virtual void ref_Impl() 
	{
		++ReferenceCounter;
	}

	virtual bool unref_Impl()
	{
		if (ReferenceCounter > 0)
		{
			--ReferenceCounter;
			return true;
		}
		return false;
	}

	long ReferenceCounter;
};
#endif

