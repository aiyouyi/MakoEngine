/****************************************************************************
 Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#include "renderer/CCVertexIndexBuffer.h"

NS_CC_BEGIN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
bool VertexBuffer::_enableShadowCopy = true;
#else
bool VertexBuffer::_enableShadowCopy = false;
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
bool IndexBuffer::_enableShadowCopy = true;
#else
bool IndexBuffer::_enableShadowCopy = false;
#endif

VertexBuffer* VertexBuffer::create(int sizePerVertex, int vertexNumber, D3D11_USAGE usage/* = GL_STATIC_DRAW*/)
{
    auto result = new (std::nothrow) VertexBuffer();
    if(result && result->init(sizePerVertex, vertexNumber, usage))
    {
        result->autorelease();
        return result;
    }
    CC_SAFE_DELETE(result);
    return nullptr;
    
}

VertexBuffer* VertexBuffer::create(int sizePerVertex, int vertexNumber, void *pData, D3D11_USAGE usage/* = GL_STATIC_DRAW*/)
{
	auto result = new (std::nothrow) VertexBuffer();
	if (result && result->init(sizePerVertex, vertexNumber, pData, usage))
	{
		result->autorelease();
		return result;
	}
	CC_SAFE_DELETE(result);
	return nullptr;
}

VertexBuffer::VertexBuffer()
: _recreateVBOEventListener(nullptr)
, _vbo(0)
, _sizePerVertex(0)
, _vertexNumber(0)
{
    
#if CC_ENABLE_CACHE_TEXTURE_DATA
    auto callBack = [this](EventCustom* event)
    {
        this->recreateVBO();
    };

    _recreateVBOEventListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(EVENT_RENDERER_RECREATED, callBack);

#endif
}

VertexBuffer::~VertexBuffer()
{
	if (_vbo != NULL)
	{
		_vbo->Release();
		_vbo = NULL;
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_recreateVBOEventListener);
#endif
}

bool VertexBuffer::init(int sizePerVertex, int vertexNumber, D3D11_USAGE usage/* = GL_STATIC_DRAW*/)
{
    if(0 == sizePerVertex || 0 == vertexNumber)
        return false;
    _sizePerVertex = sizePerVertex;
    _vertexNumber = vertexNumber;
    _usage = usage;
    
    if(isShadowCopyEnabled())
    {
        _shadowCopy.resize(sizePerVertex * _vertexNumber);
    }
    
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = sizePerVertex * vertexNumber;
	verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	verBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, NULL, &_vbo);
	return SUCCEEDED(hr);
}

bool VertexBuffer::init(int sizePerVertex, int vertexNumber, void *pData, D3D11_USAGE usage)
{
	if (0 == sizePerVertex || 0 == vertexNumber)
		return false;
	_sizePerVertex = sizePerVertex;
	_vertexNumber = vertexNumber;
	_usage = usage;

	if (isShadowCopyEnabled())
	{
		_shadowCopy.resize(sizePerVertex * _vertexNumber);
	}

	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = sizePerVertex * vertexNumber;
	verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	verBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA dataInit;
	memset(&dataInit, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	dataInit.pSysMem = pData;
	HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, &dataInit, &_vbo);
	return SUCCEEDED(hr);
}

int VertexBuffer::getSizePerVertex() const
{
    return _sizePerVertex;
}

int VertexBuffer::getVertexNumber() const
{
    return _vertexNumber;
}

bool VertexBuffer::updateVertices(const void* verts, int count, int begin)
{
    if(count <= 0 || nullptr == verts) return false;
    
    if(begin < 0)
    {
        CCLOGERROR("Update vertices with begin = %d, will set begin to 0", begin);
        begin = 0;
    }
    
    if(count + begin > _vertexNumber)
    {
        CCLOGERROR("updated vertices exceed the max size of vertex buffer, will set count to _vertexNumber-begin");
        count = _vertexNumber - begin;
    }
    
    if(isShadowCopyEnabled())
    {
        memcpy(&_shadowCopy[begin * _sizePerVertex], verts, count * _sizePerVertex);
    }
    
	D3D11_MAPPED_SUBRESOURCE mapSubResource;
	HRESULT hr = DeviceContextPtr->Map(_vbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapSubResource);
	if(FAILED(hr))
	{
		return false;
	}

	memcpy((unsigned char *)mapSubResource.pData + begin*getSizePerVertex(), verts, count*getSizePerVertex());

	DeviceContextPtr->Unmap(_vbo, 0);
    return true;
}

bool VertexBuffer::updateVertices(const void* verts, int count, int begin, int offset, int perUpdateSize)
{
	if (count <= 0 || nullptr == verts) return false;

	if (begin < 0)
	{
		CCLOGERROR("Update vertices with begin = %d, will set begin to 0", begin);
		begin = 0;
	}

	if (count + begin > _vertexNumber)
	{
		CCLOGERROR("updated vertices exceed the max size of vertex buffer, will set count to _vertexNumber-begin");
		count = _vertexNumber - begin;
	}

	if (isShadowCopyEnabled())
	{
		if (perUpdateSize == _sizePerVertex && offset == 0)
		{
			memcpy(&_shadowCopy[begin * _sizePerVertex], verts, count * _sizePerVertex);
		}
		else
		{
			unsigned char *pDest = &_shadowCopy[begin * _sizePerVertex + offset];
			unsigned char* pSrc = (unsigned char *)verts;
			for (int i = 0; i < count; ++i)
			{
				memcpy(pDest, pSrc, perUpdateSize);
				pDest += _sizePerVertex;
				pSrc += perUpdateSize;
			}
		}
	}

	D3D11_MAPPED_SUBRESOURCE mapSubResource;
	HRESULT hr = DeviceContextPtr->Map(_vbo, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mapSubResource);
	if (FAILED(hr))
	{
		return false;
	}
	
	if (perUpdateSize == _sizePerVertex && offset == 0)
	{
		memcpy((unsigned char *)mapSubResource.pData + begin*getSizePerVertex(), verts, count*getSizePerVertex());
	}
	else
	{
		unsigned char *pDest = ((unsigned char *)mapSubResource.pData) + begin * _sizePerVertex + offset;
		unsigned char* pSrc = (unsigned char *)verts;
		for (int i = 0; i < count; ++i)
		{
			memcpy(pDest, pSrc, perUpdateSize);
			pDest += _sizePerVertex;
			pSrc += perUpdateSize;
		}
	}
	
	DeviceContextPtr->Unmap(_vbo, 0);
	return true;
}

ID3D11Buffer *VertexBuffer::getVBO() const
{
    return _vbo;
}

void VertexBuffer::recreateVBO() const
{
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = _sizePerVertex * _vertexNumber;
	verBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	verBufferDesc.Usage = _usage;
	HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, NULL, &_vbo);

	
}

int VertexBuffer::getSize() const
{
    return _sizePerVertex * _vertexNumber;
}

IndexBuffer* IndexBuffer::create(IndexType type, int number, D3D11_USAGE usage/* = GL_STATIC_DRAW*/)
{
    auto result = new (std::nothrow) IndexBuffer();
    if(result && result->init(type, number, usage))
    {
        result->autorelease();
        return result;
    }
    CC_SAFE_DELETE(result);
    return nullptr;
}

IndexBuffer* IndexBuffer::create(IndexType type, int number, void *pData, D3D11_USAGE usage)
{
	auto result = new (std::nothrow) IndexBuffer();
	if (result && result->init(type, number, pData, usage))
	{
		result->autorelease();
		return result;
	}
	CC_SAFE_DELETE(result);
	return nullptr;
}

IndexBuffer::IndexBuffer()
: _vbo(0)
, _type(IndexType::INDEX_TYPE_SHORT_16)
, _indexNumber(0)
, _recreateVBOEventListener(nullptr)
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
    auto callBack = [this](EventCustom* event)
    {
        this->recreateVBO();
    };

    _recreateVBOEventListener = Director::getInstance()->getEventDispatcher()->addCustomEventListener(EVENT_RENDERER_RECREATED, callBack);
#endif
}

IndexBuffer::~IndexBuffer()
{
	if (_vbo != NULL)
	{
		_vbo->Release();
		_vbo = NULL;
	}
#if CC_ENABLE_CACHE_TEXTURE_DATA
    Director::getInstance()->getEventDispatcher()->removeEventListener(_recreateVBOEventListener);
#endif
}

bool IndexBuffer::init(IndexBuffer::IndexType type, int number, D3D11_USAGE usage/* = GL_STATIC_DRAW*/)
{
    if(number <=0 ) return false;
    
    _type = type;
    _indexNumber = number;
    _usage = usage;
    
	if (isShadowCopyEnabled())
	{
		_shadowCopy.resize(getSize());
	}

	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = getSize();
	verBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	verBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, NULL, &_vbo);
	return SUCCEEDED(hr);
}

bool IndexBuffer::init(IndexBuffer::IndexType type, int number, void *pData, D3D11_USAGE usage/* = GL_STATIC_DRAW*/)
{
	if (number <= 0) return false;

	_type = type;
	_indexNumber = number;
	_usage = usage;

	if (isShadowCopyEnabled())
	{
		_shadowCopy.resize(getSize());
	}

	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = getSize();
	verBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	verBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA dataInit;
	memset(&dataInit, 0, sizeof(D3D11_SUBRESOURCE_DATA));
	dataInit.pSysMem = pData;
	HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, &dataInit, &_vbo);
	return SUCCEEDED(hr);
}

IndexBuffer::IndexType IndexBuffer::getType() const
{
    return _type;
}

int IndexBuffer::getSizePerIndex() const
{
    return IndexType::INDEX_TYPE_SHORT_16 == _type ? 2 : 4;
}

int IndexBuffer::getIndexNumber() const
{
    return _indexNumber;
}

bool IndexBuffer::updateIndices(const void* indices, int count, int begin)
{
    if(count <= 0 || nullptr == indices) return false;
    
    if(begin < 0)
    {
        CCLOGERROR("Update indices with begin = %d, will set begin to 0", begin);
        begin = 0;
    }
    
    if(count + begin > _indexNumber)
    {
        CCLOGERROR("updated indices exceed the max size of vertex buffer, will set count to _indexNumber-begin");
        count = _indexNumber - begin;
    }
    
	D3D11_MAPPED_SUBRESOURCE mapSubResource;
	HRESULT hr = DeviceContextPtr->Map(_vbo, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapSubResource);
	if (FAILED(hr))
	{
		return false;
	}

	memcpy((unsigned char *)mapSubResource.pData + begin*getSizePerIndex(), indices, count*getSizePerIndex());

	DeviceContextPtr->Unmap(_vbo, 0);


    if(isShadowCopyEnabled())
    {
        memcpy(&_shadowCopy[begin * getSizePerIndex()], indices, count * getSizePerIndex());
    }
    
    return true;
}

int IndexBuffer::getSize() const
{
    return getSizePerIndex() * _indexNumber;
}

ID3D11Buffer *IndexBuffer::getVBO() const
{
    return _vbo;
}

void IndexBuffer::recreateVBO() const
{
    CCLOG("come to foreground of IndexBuffer");
	D3D11_BUFFER_DESC verBufferDesc;
	memset(&verBufferDesc, 0, sizeof(D3D11_BUFFER_DESC));
	verBufferDesc.ByteWidth = getSize();
	verBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	verBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	verBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	HRESULT hr = DevicePtr->CreateBuffer(&verBufferDesc, NULL, &_vbo);
}

NS_CC_END
