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

#include "3d/CCMesh.h"
#include "3d/CCMeshSkin.h"
#include "3d/CCSkeleton3D.h"
#include "3d/CCMeshVertexIndexData.h"
#include "math/Mat4.h"

using namespace std;

NS_CC_BEGIN

// Helpers

//sampler uniform names, only diffuse and normal texture are supported for now
std::string s_uniformSamplerName[] =
{
    "",//NTextureData::Usage::Unknown,
    "",//NTextureData::Usage::None
    "",//NTextureData::Usage::Diffuse
    "",//NTextureData::Usage::Emissive
    "",//NTextureData::Usage::Ambient
    "",//NTextureData::Usage::Specular
    "",//NTextureData::Usage::Shininess
    "u_normalTex",//NTextureData::Usage::Normal
    "",//NTextureData::Usage::Bump
    "",//NTextureData::Usage::Transparency
    "",//NTextureData::Usage::Reflection
};

static const char          *s_dirLightUniformColorName = "u_DirLightSourceColor";
static const char          *s_dirLightUniformDirName = "u_DirLightSourceDirection";

static const char          *s_pointLightUniformColorName = "u_PointLightSourceColor";
static const char          *s_pointLightUniformPositionName = "u_PointLightSourcePosition";
static const char          *s_pointLightUniformRangeInverseName = "u_PointLightSourceRangeInverse";

static const char          *s_spotLightUniformColorName = "u_SpotLightSourceColor";
static const char          *s_spotLightUniformPositionName = "u_SpotLightSourcePosition";
static const char          *s_spotLightUniformDirName = "u_SpotLightSourceDirection";
static const char          *s_spotLightUniformInnerAngleCosName = "u_SpotLightSourceInnerAngleCos";
static const char          *s_spotLightUniformOuterAngleCosName = "u_SpotLightSourceOuterAngleCos";
static const char          *s_spotLightUniformRangeInverseName = "u_SpotLightSourceRangeInverse";

static const char          *s_ambientLightUniformColorName = "u_AmbientLightSourceColor";

// helpers




Mesh::Mesh()
: _skin(nullptr)
, _visible(true)
, _isTransparent(false)
, _meshIndexData(nullptr)
, _blendDirty(true)
, _texFile("")
{
    
}
Mesh::~Mesh()
{
    CC_SAFE_RELEASE(_skin);
    CC_SAFE_RELEASE(_meshIndexData);

	std::map<NTextureData::Usage, DX11Texture*>::iterator it = _textures.begin();
	while (it != _textures.end())
	{
		if (it->second != NULL)
		{
			it->second->unref();
		}

		++it;
	}

	_textures.clear();
}

ID3D11Buffer *Mesh::getVertexBuffer() const
{
    return _meshIndexData->getVertexBuffer()->getVBO();
}

bool Mesh::hasVertexAttrib(int attrib) const
{
    return _meshIndexData->getMeshVertexData()->hasVertexAttrib(attrib);
}

size_t Mesh::getMeshVertexAttribCount() const
{
    return _meshIndexData->getMeshVertexData()->getMeshVertexAttribCount();
}

const MeshVertexAttrib& Mesh::getMeshVertexAttribute(int idx)
{
    return _meshIndexData->getMeshVertexData()->getMeshVertexAttrib(idx);
}

int Mesh::getVertexSizeInBytes() const
{
    return _meshIndexData->getVertexBuffer()->getSizePerVertex();
}

Mesh* Mesh::create(const std::vector<float>& positions, const std::vector<float>& normals, const std::vector<float>& texs, const IndexArray& indices)
{
    /*int perVertexSizeInFloat = 0;
    std::vector<float> vertices;
    std::vector<MeshVertexAttrib> attribs;
    MeshVertexAttrib att;
    att.size = 3;
    att.type = GL_FLOAT;
    att.attribSizeBytes = att.size * sizeof(float);
    
    if (positions.size())
    {
        perVertexSizeInFloat += 3;
        att.vertexAttrib = GLProgram::VERTEX_ATTRIB_POSITION;
        attribs.push_back(att);
    }
    if (normals.size())
    {
        perVertexSizeInFloat += 3;
        att.vertexAttrib = GLProgram::VERTEX_ATTRIB_NORMAL;
        attribs.push_back(att);
    }
    if (texs.size())
    {
        perVertexSizeInFloat += 2;
        att.vertexAttrib = GLProgram::VERTEX_ATTRIB_TEX_COORD;
        att.size = 2;
        att.attribSizeBytes = att.size * sizeof(float);
        attribs.push_back(att);
    }
    
    bool hasNormal = (normals.size() != 0);
    bool hasTexCoord = (texs.size() != 0);
    //position, normal, texCoordinate into _vertexs
    size_t vertexNum = positions.size() / 3;
    for(size_t i = 0; i < vertexNum; i++)
    {
        vertices.push_back(positions[i * 3]);
        vertices.push_back(positions[i * 3 + 1]);
        vertices.push_back(positions[i * 3 + 2]);

        if (hasNormal)
        {
            vertices.push_back(normals[i * 3]);
            vertices.push_back(normals[i * 3 + 1]);
            vertices.push_back(normals[i * 3 + 2]);
        }
    
        if (hasTexCoord)
        {
            vertices.push_back(texs[i * 2]);
            vertices.push_back(texs[i * 2 + 1]);
        }
    }
    return create(vertices, perVertexSizeInFloat, indices, attribs);*/
	return NULL;
}

Mesh* Mesh::create(const std::vector<float>& vertices, int /*perVertexSizeInFloat*/, const IndexArray& indices, const std::vector<MeshVertexAttrib>& attribs)
{
    MeshData meshdata;
    meshdata.attribs = attribs;
    meshdata.vertex = vertices;
    meshdata.subMeshIndices.push_back(indices);
    meshdata.subMeshIds.push_back("");
    auto meshvertexdata = MeshVertexData::create(meshdata);
    auto indexData = meshvertexdata->getMeshIndexDataByIndex(0);
    
    return create("", indexData);
}

Mesh* Mesh::create(const std::string& name, MeshIndexData* indexData, MeshSkin* skin)
{
    auto state = new (std::nothrow) Mesh();
    state->autorelease();
    state->_name = name;
    state->setMeshIndexData(indexData);
    state->setSkin(skin);
    
    return state;
}

void Mesh::setVisible(bool visible)
{
    if (_visible != visible)
    {
        _visible = visible;
        if (_visibleChanged)
            _visibleChanged();
    }
}

bool Mesh::isVisible() const
{
    return _visible;
}

void Mesh::setSkin(MeshSkin* skin)
{
    if (_skin != skin)
    {
        CC_SAFE_RETAIN(skin);
        CC_SAFE_RELEASE(_skin);
        _skin = skin;
    }
}

void Mesh::setMeshIndexData(MeshIndexData* subMesh)
{
    if (_meshIndexData != subMesh)
    {
        CC_SAFE_RETAIN(subMesh);
        CC_SAFE_RELEASE(_meshIndexData);
        _meshIndexData = subMesh;
    }
}

GLenum Mesh::getPrimitiveType() const
{
    return _meshIndexData->getPrimitiveType();
}

ssize_t Mesh::getIndexCount() const
{
    return _meshIndexData->getIndexBuffer()->getIndexNumber();
}

DXGI_FORMAT Mesh::getIndexFormat() const
{
    return DXGI_FORMAT_R16_UINT;
}

ID3D11Buffer *Mesh::getIndexBuffer() const
{
    return _meshIndexData->getIndexBuffer()->getVBO();
}

void Mesh::setTexture(DX11Texture *pTex, NTextureData::Usage usage)
{
	if (pTex == NULL)return;
	std::map<NTextureData::Usage, DX11Texture*>::iterator it = _textures.find(usage);
	if (it != _textures.end())
	{
		if (it->second != pTex)
		{
			pTex->ref();
			it->second = pTex;
		}
	}
	else
	{
		pTex->ref();
		_textures.insert(std::make_pair(usage, pTex));
	}
}
DX11Texture *Mesh::GetTexture(NTextureData::Usage usage)
{
	std::map<NTextureData::Usage, DX11Texture*>::iterator it = _textures.find(usage);
	if (it != _textures.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}

NS_CC_END
