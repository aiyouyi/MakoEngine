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

#ifndef __CCMESH_H__
#define __CCMESH_H__

#include <string>
#include <map>

#include "3d/CCBundle3DData.h"
#include "base/CCRef.h"
#include "math/CCMath.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include "CCMeshVertexIndexData.h"
#include "CCMeshSkin.h"
NS_CC_BEGIN

/**
 * @addtogroup _3d
 * @{
 */

/** 
 * @brief Mesh: contains ref to index buffer, GLProgramState, texture, skin, blend function, aabb and so on
 */
class CC_DLL Mesh : public Ref
{
    friend class Sprite3D;
public:
    typedef std::vector<unsigned short> IndexArray;
    /**create mesh from positions, normals, and so on, single SubMesh*/
    static Mesh* create(const std::vector<float>& positions, const std::vector<float>& normals, const std::vector<float>& texs, const IndexArray& indices);
    /**create mesh with vertex attributes*/
    CC_DEPRECATED_ATTRIBUTE static Mesh* create(const std::vector<float>& vertices, int perVertexSizeInFloat, const IndexArray& indices, int /*numIndex*/, const std::vector<MeshVertexAttrib>& attribs, int /*attribCount*/){ return create(vertices, perVertexSizeInFloat, indices, attribs); }
    
    /**
     * @lua NA
     */
    static Mesh* create(const std::vector<float>& vertices, int perVertexSizeInFloat, const IndexArray& indices, const std::vector<MeshVertexAttrib>& attribs);
    
    /** 
     * create mesh
     * @lua NA
     */
    static Mesh* create(const std::string& name, MeshIndexData* indexData, MeshSkin* skin = nullptr);
    
    /**
     * get vertex buffer
     * 
     * @lua NA
     */
	ID3D11Buffer *getVertexBuffer() const;
    /**
     * has vertex attribute?
     *
     * @lua NA
     */
    bool hasVertexAttrib(int attrib) const;
    /**get mesh vertex attribute count*/
    size_t getMeshVertexAttribCount() const;
    /**get MeshVertexAttribute by index*/
    const MeshVertexAttrib& getMeshVertexAttribute(int idx);
    /**get per vertex size in bytes*/
    int getVertexSizeInBytes() const;
    
    /**visible getter and setter*/
    void setVisible(bool visible);
    bool isVisible() const;
    
    /**
     * skin getter
     *
     * @lua NA
     */
    MeshSkin* getSkin() const { return _skin; }
    
    /**
     * mesh index data getter
     *
     * @lua NA
     */
    MeshIndexData* getMeshIndexData() const { return _meshIndexData; }
    
    
    /**name getter */
    const std::string& getName() const { return _name; }
    
    /** 
     * get primitive type
     *
     * @lua NA
     */
    GLenum getPrimitiveType() const;
    /**
     * get index count
     *
     * @lua NA
     */
    ssize_t getIndexCount() const;
    /**
     * get index format
     *
     * @lua NA
     */
	DXGI_FORMAT getIndexFormat() const;
    /**
     * get index buffer
     *
     * @lua NA
     */
	ID3D11Buffer *getIndexBuffer() const;

    /**skin setter*/
    void setSkin(MeshSkin* skin);
    /**Mesh index data setter*/
    void setMeshIndexData(MeshIndexData* indexdata);
    /**name setter*/
    void setName(const std::string& name) { _name = name; }

    std::string getTextureFileName(){ return _texFile; }

	void setTexture(DX11Texture *pTex, NTextureData::Usage usage = NTextureData::Usage::Diffuse);
	DX11Texture *GetTexture(NTextureData::Usage usage = NTextureData::Usage::Diffuse);
CC_CONSTRUCTOR_ACCESS:

    Mesh();
    virtual ~Mesh();

protected:

    std::map<NTextureData::Usage, DX11Texture*> _textures; //textures that submesh is using
    MeshSkin*           _skin;     //skin
    bool                _visible; // is the submesh visible
    bool                _isTransparent; // is this mesh transparent, it is a property of material in fact
    
    std::string         _name;
    MeshIndexData*      _meshIndexData;
    bool                _blendDirty;
    std::function<void()> _visibleChanged;

    std::string _texFile;
};

// end of 3d group
/// @}

/// @cond
extern std::string CC_DLL s_uniformSamplerName[];//uniform sampler names array
/// @endcond

NS_CC_END

#endif // __CCMESH_H__
