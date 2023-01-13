#pragma once
#include "Toolbox/inc.h"

class CC3DTextureRHI;

class DoubleBufferRHI
{
public:
	DoubleBufferRHI() ;
	virtual ~DoubleBufferRHI() ;

	virtual void BindFBOA() = 0;
	virtual void unBindFBOA() ;
	virtual void BindFBOB() = 0;
	virtual void unBindFBOB();
	virtual void SwapFBO() = 0;

	virtual void SyncAToB() = 0;
	virtual void SyncBToA() = 0;

	virtual void SyncAToBRegion(float* pVerts, int nVerts, int nStep = 2, int type = 0) = 0;

	virtual int GetWidth() const = 0 ;
	virtual int GetHeight() const = 0;

	virtual void InitDoubleBuffer(uint32_t format, int32_t width, int32_t height, bool UseDepth, bool UseMultiTarget, bool bMSAA) {};
	virtual void InitDoubleBuffer(std::shared_ptr<CC3DTextureRHI> pTexIn, std::shared_ptr<CC3DTextureRHI> pTexOut, int nWidth, int nHeight, bool bMSAA = false) {}

	virtual std::shared_ptr<CC3DTextureRHI> GetFBOTextureA() = 0;
	virtual std::shared_ptr<CC3DTextureRHI> GetFBOTextureB() = 0;

	virtual void BindDoubleBuffer() {}

	virtual  void ClearA(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0) {};
	virtual  void ClearDepthA(float depth = 1.0f, unsigned char Stencil = 0) {};

	virtual  void ClearB(float r, float g, float b, float a, float depth = 1.0f, unsigned char Stencil = 0) {};
	virtual  void ClearDepthB(float depth = 1.0f, unsigned char Stencil = 0) {};
};