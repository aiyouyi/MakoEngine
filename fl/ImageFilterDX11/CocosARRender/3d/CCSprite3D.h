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

#ifndef __CCSPRITE3D_H__
#define __CCSPRITE3D_H__

#include <unordered_map>
#include "base/CCVector.h"
#include "base/ccTypes.h"
#include "3d/CCSkeleton3D.h"
#include "3d/CCBundle3DData.h"
#include "3d/CCMeshVertexIndexData.h"
#include "2d/CCNode.h"
#include "3d/CCMesh.h"
#include "CCBundle3D.h"

NS_CC_BEGIN

/**
 * @addtogroup _3d
 * @{
 */

class Mesh;
class Texture2D;
class MeshSkin;
class AttachNode;
struct NodeData;
class CC_DLL Sprite3D: public Node
{
public:
    /**
     * Creates an empty sprite3D without 3D model and texture.
     *
     * @return An autoreleased sprite3D object.
     */
    static Sprite3D* create();
    
    /** creates a Sprite3D*/
    static Sprite3D* create(const std::string &modelPath);

	static Sprite3D* createFromZip(const std::string &file, HZIP hZip, const std::string &zip);
    
    /**set diffuse texture, set the first if multiple textures exist*/
	void setTexture(const std::string& texFile) {}
	void setTexture(Texture2D* texture) {}
    
    /**get Mesh by index*/
    Mesh* getMeshByIndex(int index) const;
    
    /**get Mesh by Name, it returns the first one if there are more than one mesh with the same name */
    Mesh* getMeshByName(const std::string& name) const;
    
    /** 
     * get mesh array by name, returns all meshes with the given name
     *
     * @lua NA
     */
    std::vector<Mesh*> getMeshArrayByName(const std::string& name) const;

    /**get mesh*/
    Mesh* getMesh() const;
    
    /** get mesh count */
    size_t getMeshCount() const { return _meshes.size(); }
    
    /**get skin*/
    MeshSkin* getSkin() const;
    
    Skeleton3D* getSkeleton() const { return _skeleton; }
    
    Sprite3D();
    virtual ~Sprite3D();
    
    virtual bool init();
    
    bool initWithFile(const std::string &path, Sprite3DResourceLoader *pLoader);
    
    bool initFrom(const NodeDatas& nodedatas, const MeshDatas& meshdatas, const MaterialDatas& materialdatas, Sprite3DResourceLoader *pLoader);
    
    /** load file and set it to meshedatas, nodedatas and materialdatas, obj file .mtl file should be at the same directory if exist */
    bool loadFromFile(const std::string& path, NodeDatas* nodedatas, MeshDatas* meshdatas,  MaterialDatas* materialdatas, Sprite3DResourceLoader *pLoader);

	void createNode(NodeData* nodedata, Node* root, const MaterialDatas& materialdatas, bool singleSprite, Sprite3DResourceLoader *pLoader);
	Sprite3D* createSprite3DNode(NodeData* nodedata, ModelData* modeldata, const MaterialDatas& materialdatas, Sprite3DResourceLoader *pLoader);

	/**get MeshIndexData by Id*/
	MeshIndexData* getMeshIndexData(const std::string& indexId) const;

	void addMesh(Mesh* mesh);

	virtual void updateBlendWeights(int meshIndex, const map<int, float> &mapWeights);

	//‰÷»æÀ≥–Ú....
	void update(float fTime, float *arrExpAlpha);
	virtual void visit(Vector<MeshRenderCmd *> &renderCmds, bool lighting = false,bool bNPR = false);
	void updateNeckRotate(float *arrQ);
protected:

    Skeleton3D*                  _skeleton; //skeleton
    
    Vector<MeshVertexData*>      _meshVertexDatas;
    
    Vector<Mesh *>              _meshes;
	Vector<MeshRenderCmd *> m_renderCmds;

    mutable Mat4                 _nodeToWorldTransform; // cache the matrix
};
NS_CC_END
#endif // __SPRITE3D_H_
