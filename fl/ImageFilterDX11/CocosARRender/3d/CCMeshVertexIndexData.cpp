/****************************************************************************
 Copyright (c) 2014-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include <list>
#include <fstream>
#include <iostream>
#include <sstream>

#include "3d/CCMeshVertexIndexData.h"
#include "3d/CCMesh.h"
#include "3d/CCBundle3D.h"
#include "base/ccMacros.h"


using namespace std;

NS_CC_BEGIN


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeshIndexData* MeshIndexData::create(const std::string& id, MeshVertexData* vertexData, IndexBuffer* indexbuffer, const AABB& aabb)
{
	MeshIndexData *meshindex = new (std::nothrow) MeshIndexData();

	meshindex->_id = id;
	meshindex->_indexBuffer = indexbuffer;
	meshindex->_vertexData = vertexData;
	indexbuffer->retain();
	meshindex->_aabb = aabb;

	meshindex->autorelease();
	return meshindex;
}

const VertexBuffer* MeshIndexData::getVertexBuffer() const
{
	return _vertexData->getVertexBuffer();
}

MeshIndexData::MeshIndexData()
	: _indexBuffer(nullptr)
	, _vertexData(nullptr)
	, _primitiveType(0)
{

}
MeshIndexData::~MeshIndexData()
{
	CC_SAFE_RELEASE(_indexBuffer);
}

MeshVertexData* MeshVertexData::create(const MeshData& meshdata)
{
	MeshVertexData *vertexdata = new (std::nothrow) MeshVertexData();
	int pervertexsize = meshdata.getPerVertexSize();
	int vertexCount = (int)(meshdata.vertex.size() / (pervertexsize / 4));
	vertexdata->_vertexBuffer = VertexBuffer::create(pervertexsize, vertexCount, (void*)&meshdata.vertex[0]);
	vertexdata->_vertexData = VertexData::create();
	vertexdata->vBlendChanels = meshdata.vBlendChanels;

	int posOffset = 0;
	for (int iAttrib = 0; iAttrib < meshdata.attribs.size(); ++iAttrib)
	{
		if (meshdata.attribs.at(iAttrib).type == VertexData::VERTEX_ATTRIB_POSITION)
		{
			break;
		}

		posOffset += meshdata.attribs.at(iAttrib).size;
	}
	vertexdata->vOrignXYZ.resize(vertexCount*3);
	vertexdata->vBlendXYZ.resize(vertexCount*3);

	float *pSrc = (float *)&(meshdata.vertex[0]) + posOffset;
	float *pDest = (float *)&(vertexdata->vOrignXYZ[0]);
	for (int iVertex = 0; iVertex < vertexCount; ++iVertex)
	{
		*pDest = *pSrc;
		pDest[0] = pSrc[0];
		pDest[1] = pSrc[1];
		pDest[2] = pSrc[2];

		pSrc += pervertexsize / 4;
		pDest += 3;
	}


	CC_SAFE_RETAIN(vertexdata->_vertexData);
	CC_SAFE_RETAIN(vertexdata->_vertexBuffer);

	int offset = 0;
	for(size_t i=0; i<meshdata.attribs.size(); ++i)
	{
		MeshVertexAttrib it = meshdata.attribs.at(i);
		vertexdata->_vertexData->setStream(vertexdata->_vertexBuffer, VertexStreamAttribute(offset, it.vertexAttrib, it.type, it.size));
		offset += it.attribSizeBytes;
	}

	vertexdata->_attribs = meshdata.attribs;

	if (vertexdata->_vertexBuffer)
	{
		//vertexdata->_vertexBuffer->updateVertices((void*)&meshdata.vertex[0], (int)meshdata.vertex.size() * 4 / vertexdata->_vertexBuffer->getSizePerVertex(), 0);
	}

	bool needCalcAABB = (meshdata.subMeshAABB.size() != meshdata.subMeshIndices.size());
	for (size_t i = 0, size = meshdata.subMeshIndices.size(); i < size; ++i) {

		const MeshData::IndexArray& index = meshdata.subMeshIndices[i];
		IndexBuffer *indexBuffer = IndexBuffer::create(IndexBuffer::IndexType::INDEX_TYPE_SHORT_16, (int)(index.size()), (void *)&index[0], D3D11_USAGE_DEFAULT);
		//indexBuffer->updateIndices(&index[0], (int)index.size(), 0);
		std::string id = (i < meshdata.subMeshIds.size() ? meshdata.subMeshIds[i] : "");
		MeshIndexData* indexdata = nullptr;
		if (needCalcAABB)
		{
			AABB aabb = Bundle3D::calculateAABB(meshdata.vertex, meshdata.getPerVertexSize(), index);
			indexdata = MeshIndexData::create(id, vertexdata, indexBuffer, aabb);
		}
		else
			indexdata = MeshIndexData::create(id, vertexdata, indexBuffer, meshdata.subMeshAABB[i]);

		vertexdata->_indexs.pushBack(indexdata);
	}

	vertexdata->autorelease();
	return vertexdata;
}

void MeshVertexData::updateVertexBuffer(float fTime)
{
	if (vBlendChanels.size() > 0)
	{
		m_fTestTime += fTime;
		while (m_fTestTime > 1.0f)
		{
			m_fTestTime -= 1.0f;
		}

		float fWeight = min(m_fTestTime / 1.0f, 1.0f)*100;

		//原始坐标数据
		vBlendXYZ = vOrignXYZ;
		for (int i = 0; i < vBlendChanels.size(); ++i)
		{
			//寻找当前chanel下播放的关键位置
			int iEndKey = 0;
			for (int iKey = 0; iKey < vBlendChanels[i].blendShapeKeys.size(); ++iKey)
			{
				if (vBlendChanels[i].blendShapeKeys[iKey].weight >= fWeight)
				{
					iEndKey = iKey;
					break;
				}
			}
			if (iEndKey >= vBlendChanels[i].blendShapeKeys.size())
			{
				iEndKey = vBlendChanels[i].blendShapeKeys.size() - 1;
			}

			int iStartKey = iEndKey - 1;

			std::vector<float> *pStartValues = &vOrignXYZ;
			std::vector<float> *pEndValues = &(vBlendChanels[i].blendShapeKeys[iEndKey].keyPositions);
			float fBlendWeight = 0.0f;
			if (iStartKey < 0)
			{
				fBlendWeight = fWeight / 100.0f;
			}
			else
			{
				fBlendWeight = (fWeight - vBlendChanels[i].blendShapeKeys[iStartKey].weight) / (vBlendChanels[i].blendShapeKeys[iEndKey].weight - vBlendChanels[i].blendShapeKeys[iStartKey].weight);
				pStartValues = &(vBlendChanels[i].blendShapeKeys[iStartKey].keyPositions);
			}

			for (int iValue = 0; iValue < pStartValues->size(); ++iValue)
			{
				vBlendXYZ[iValue] = (*pStartValues)[iValue] * (1 - fBlendWeight) + (*pEndValues)[iValue]*fBlendWeight;
			}
		}

		int vertexCount = _vertexBuffer->getVertexNumber();
		_vertexBuffer->updateVertices(&(vBlendXYZ[0]), vertexCount, 0, 0, 3*4);
	}
}

void MeshVertexData::updateVertexBuffer(float *pWeights, int nCount)
{
	if (vBlendChanels.size() > 0 && nCount > 0 && vBlendChanels.size() == nCount)
	{
		//原始坐标数据
		vBlendXYZ = vOrignXYZ;
		for (int i = 0; i < vBlendChanels.size() && i<nCount; ++i)
		{
			float fWeight = pWeights[i];

			//寻找当前chanel下播放的关键位置
			int iEndKey = 0;
			for (int iKey = 0; iKey < vBlendChanels[i].blendShapeKeys.size(); ++iKey)
			{
				if (vBlendChanels[i].blendShapeKeys[iKey].weight >= fWeight)
				{
					iEndKey = iKey;
					break;
				}
			}
			if (iEndKey >= vBlendChanels[i].blendShapeKeys.size())
			{
				iEndKey = vBlendChanels[i].blendShapeKeys.size() - 1;
			}

			int iStartKey = iEndKey - 1;

			std::vector<float> *pStartValues = &vOrignXYZ;
			std::vector<float> *pEndValues = &(vBlendChanels[i].blendShapeKeys[iEndKey].keyPositions);
			float fBlendWeight = 0.0f;
			if (iStartKey < 0)
			{
				fBlendWeight = fWeight / 100.0f;
			}
			else
			{
				fBlendWeight = (fWeight - vBlendChanels[i].blendShapeKeys[iStartKey].weight) / (vBlendChanels[i].blendShapeKeys[iEndKey].weight - vBlendChanels[i].blendShapeKeys[iStartKey].weight);
				pStartValues = &(vBlendChanels[i].blendShapeKeys[iStartKey].keyPositions);
			}

			for (int iValue = 0; iValue < pStartValues->size(); ++iValue)
			{
				float fDValue = (*pStartValues)[iValue] * (1 - fBlendWeight) + (*pEndValues)[iValue] * fBlendWeight - vOrignXYZ[iValue];
				vBlendXYZ[iValue] += fDValue;
			}
		}

		int vertexCount = _vertexBuffer->getVertexNumber();

		_vertexBuffer->updateVertices(&(vBlendXYZ[0]), vertexCount, 0, 0, 3 * 4);
	}
}

void MeshVertexData::updateVertexBuffer(const map<int, float> &mapWeights)
{
	#define MAX_CHANEL 100
	float arrWeights[MAX_CHANEL];
	if (vBlendChanels.size() > 0 && vBlendChanels.size() < MAX_CHANEL && mapWeights.size() > 0)
	{
		memset(arrWeights, 0, sizeof(float)*vBlendChanels.size());
		for (const auto &iter : mapWeights)
		{
			if (iter.first < vBlendChanels.size())
			{
				arrWeights[iter.first] = iter.second;
			}
		}

		updateVertexBuffer(arrWeights, vBlendChanels.size());
	}
}

MeshIndexData* MeshVertexData::getMeshIndexDataById(const std::string& id) const
{
	for(size_t i=0; i<_indexs.size(); ++i)
	{
		MeshIndexData *it = _indexs.at(i);
		if (it->getId() == id)
			return it;
	}
	return nullptr;
}

bool MeshVertexData::hasVertexAttrib(int attrib) const
{
	for(size_t i=0; i<_attribs.size(); ++i)
	{
		MeshVertexAttrib it = _attribs.at(i);
        if (it.vertexAttrib == attrib)
            return true;
    }
    return false;
}

MeshVertexData::MeshVertexData()
: _vertexData(nullptr)
, _vertexBuffer(nullptr)
, _vertexCount(0)
{
	m_fTestTime = 0.0f;
}
MeshVertexData::~MeshVertexData()
{
    CC_SAFE_RELEASE(_vertexData);
    CC_SAFE_RELEASE(_vertexBuffer);
    _indexs.clear();
}

NS_CC_END
