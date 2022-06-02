#pragma once
#ifndef _H_CC3D_MESHBUFFER_H_
#define _H_CC3D_MESHBUFFER_H_

#include "Common/CC3DUtils.h"
#include "Toolbox/DXUtils/DXUtils.h"
#include "Toolbox/Render/DynamicRHI.h"

#define  CC_MAX_VBO 10
class CC3DMeshBuffer
{
public:
	CC3DMeshBuffer();
	~CC3DMeshBuffer();


	void InitMesh(CC3DMeshInfo *pMesh);
	void UpdateVert(Vector3 *pVert, int nVert);

public:
	std::array<std::shared_ptr<CC3DVertexBuffer>, CC_MAX_VBO> VerticeBuffer;
	std::shared_ptr<CC3DIndexBuffer> IndexBuffer;

	int AtrributeCount = 3;

};


#endif // 
