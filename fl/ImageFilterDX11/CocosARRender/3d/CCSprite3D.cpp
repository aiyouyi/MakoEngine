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

#include "3d/CCSprite3D.h"
#include "3d/CCMeshSkin.h"
#include "3d/CCBundle3D.h"
#include "3d/CCMesh.h"

#include "base/ccUTF8.h"
#include "platform/CCPlatformMacros.h"
#include "platform/CCFileUtils.h"

NS_CC_BEGIN

Sprite3D* Sprite3D::create()
{
    //
	Sprite3D *sprite = new (std::nothrow) Sprite3D();
    if (sprite && sprite->init())
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite3D* Sprite3D::create(const std::string& modelPath)
{
    CCASSERT(modelPath.length() >= 4, "invalid filename for Sprite3D");
    
	Sprite3DResourceLoaderFromFile fileLoader;

	Sprite3D *sprite = new (std::nothrow) Sprite3D();
    if (sprite && sprite->initWithFile(modelPath, &fileLoader))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite3D* Sprite3D::createFromZip(const std::string &file, HZIP hZip, const std::string &zip)
{
	Sprite3DResourceLoaderFromZip zipLoader(hZip, zip);

	Sprite3D *sprite = new (std::nothrow) Sprite3D();
	if (sprite && sprite->initWithFile(file, &zipLoader))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}

bool Sprite3D::loadFromFile(const std::string& path, NodeDatas* nodedatas, MeshDatas* meshdatas,  MaterialDatas* materialdatas, Sprite3DResourceLoader *pLoader)
{
    std::string fullPath = path;
    
    std::string ext = FileUtils::getInstance()->getFileExtension(path);
    if (ext == ".obj")
    {
    }
    else if (ext == ".c3b" || ext == ".c3t")
    {
        //load from .c3b or .c3t
		Bundle3D *bundle = Bundle3D::createBundle();
        if (!bundle->load(fullPath, pLoader))
        {
            Bundle3D::destroyBundle(bundle);
            return false;
        }
        
        bool ret = bundle->loadMeshDatas(*meshdatas)
            && bundle->loadMaterials(*materialdatas) && bundle->loadNodes(*nodedatas);
        Bundle3D::destroyBundle(bundle);
        
        return ret;
    }
    return false;
}

Sprite3D::Sprite3D()
: _skeleton(nullptr)
{
}

Sprite3D::~Sprite3D()
{
    _meshes.clear();
    _meshVertexDatas.clear();
	CC_SAFE_RELEASE_NULL(_skeleton);

	m_renderCmds.clear();
}

bool Sprite3D::init()
{
	return true;
}

bool Sprite3D::initWithFile(const std::string &path, Sprite3DResourceLoader *pLoader)
{
    _meshes.clear();
    _meshVertexDatas.clear();
    CC_SAFE_RELEASE_NULL(_skeleton);
    
    MeshDatas* meshdatas = new (std::nothrow) MeshDatas();
    MaterialDatas* materialdatas = new (std::nothrow) MaterialDatas();
    NodeDatas* nodeDatas = new (std::nothrow) NodeDatas();
    if (loadFromFile(path, nodeDatas, meshdatas, materialdatas, pLoader))
    {
        if (initFrom(*nodeDatas, *meshdatas, *materialdatas, pLoader))
        {
			CC_SAFE_DELETE(meshdatas);
			CC_SAFE_DELETE(materialdatas);
			CC_SAFE_DELETE(nodeDatas);
            return true;
        }
    }
    CC_SAFE_DELETE(meshdatas);
    CC_SAFE_DELETE(materialdatas);
    CC_SAFE_DELETE(nodeDatas);
    
    return false;
}

bool Sprite3D::initFrom(const NodeDatas& nodeDatas, const MeshDatas& meshdatas, const MaterialDatas& materialdatas, Sprite3DResourceLoader *pLoader)
{
	for(size_t i=0; i<meshdatas.meshDatas.size(); ++i)
    {
		MeshData *meshData = meshdatas.meshDatas.at(i);
        if(meshData)
        {
//            Mesh* mesh = Mesh::create(*it);
//            _meshes.pushBack(mesh);
			MeshVertexData *meshvertex = MeshVertexData::create(*meshData);
            _meshVertexDatas.pushBack(meshvertex);
        }
    }
    _skeleton = Skeleton3D::create(nodeDatas.skeleton);
    CC_SAFE_RETAIN(_skeleton);
    
    size_t size = nodeDatas.nodes.size();
	for(size_t i=0; i<nodeDatas.nodes.size(); ++i)
    {
		NodeData *it=nodeDatas.nodes.at(i);
        if(it)
        {
            createNode(it, this, materialdatas, false, pLoader);
        }
    }
    
	size = nodeDatas.skeleton.size();
	for (size_t i = 0; i<nodeDatas.skeleton.size(); ++i)
	{
		NodeData *it = nodeDatas.skeleton.at(i);
		if (it)
		{
			createNode(it, this, materialdatas, false, pLoader);
		}
	}
    return true;
}

Sprite3D* Sprite3D::createSprite3DNode(NodeData* nodedata,ModelData* modeldata,const MaterialDatas& materialdatas, Sprite3DResourceLoader *pLoader)
{
	Sprite3D *sprite = new (std::nothrow) Sprite3D();
    if (sprite)
    {
        sprite->setName(nodedata->id);
		Mesh *mesh = Mesh::create(nodedata->id, getMeshIndexData(modeldata->subMeshId));
        
        if (_skeleton && modeldata->bones.size())
        {
			MeshSkin *skin = MeshSkin::create(_skeleton, modeldata->bones, modeldata->invBindPose);
            mesh->setSkin(skin);
        }
        
		
		if (modeldata->materialId == "" && materialdatas.materials.size())
		{
			const NTextureData* textureData = materialdatas.materials[0].getTextureData(NTextureData::Usage::Diffuse);
			if (textureData != NULL)
			{
				DX11Texture *pTex = pLoader->getTexture(textureData->filename, true); //ContextInst->fetchTexture(textureData->filename, false);
				mesh->setTexture(pTex, NTextureData::Usage::Diffuse);
				if (pTex != NULL)pTex->unref();
			}
		}
		else
		{
			const NMaterialData* materialData = materialdatas.getMaterialData(modeldata->materialId);
			if (materialData)
			{
				const NTextureData* textureData = materialData->getTextureData(NTextureData::Usage::Diffuse);
				if (textureData)
				{
					DX11Texture *tex = pLoader->getTexture(textureData->filename, true); //ContextInst->fetchTexture(textureData->filename, false);
					mesh->setTexture(tex, NTextureData::Usage::Diffuse);
					if (tex != NULL)
					{
						tex->unref();
						mesh->_isTransparent = (materialData->getTextureData(NTextureData::Usage::Transparency) != nullptr);
					}
				}
				textureData = materialData->getTextureData(NTextureData::Usage::Normal);
				if (textureData)
				{
					DX11Texture *tex = pLoader->getTexture(textureData->filename); //ContextInst->fetchTexture(textureData->filename, false);
					mesh->setTexture(tex, NTextureData::Usage::Normal);
					if (tex != NULL)tex->unref();
				}
			}
		}
		
        // set locale transform
        Vec3 pos;
        Quaternion qua;
        Vec3 scale;
        nodedata->transform.decompose(&scale, &qua, &pos);
        sprite->setPosition3D(pos);
        sprite->setRotationQuat(qua);
        sprite->setScaleX(scale.x);
        sprite->setScaleY(scale.y);
        sprite->setScaleZ(scale.z);
        
        sprite->addMesh(mesh);
        sprite->autorelease();
    }
    return  sprite;
}

void Sprite3D::createNode(NodeData* nodedata, Node* root, const MaterialDatas& materialdatas, bool singleSprite, Sprite3DResourceLoader *pLoader)
{
    Node* node=nullptr;
	for(size_t i=0; i<nodedata->modelNodeDatas.size(); ++i)
    {
		ModelData* it = nodedata->modelNodeDatas.at(i);
        if(it)
        {
            if(it->bones.size() > 0 || singleSprite)
            {
                if(singleSprite && root!=nullptr)
                    root->setName(nodedata->id);
				Mesh *mesh = Mesh::create(nodedata->id, getMeshIndexData(it->subMeshId));
                if(mesh)
                {
                    _meshes.pushBack(mesh);
                    if (_skeleton && it->bones.size())
                    {
						MeshSkin *skin = MeshSkin::create(_skeleton, it->bones, it->invBindPose);
                        mesh->setSkin(skin);
                    }
					
                    if (it->materialId == "" && materialdatas.materials.size())
                    {
                        const NTextureData* textureData = materialdatas.materials[0].getTextureData(NTextureData::Usage::Diffuse);
						if (textureData != NULL)
						{
							DX11Texture *pTex = pLoader->getTexture(textureData->filename, true);//ContextInst->fetchTexture(textureData->filename, false);
							mesh->setTexture(pTex, NTextureData::Usage::Diffuse);
							if (pTex != NULL)pTex->unref();
						}
                    }
                    else
                    {
                        const NMaterialData* materialData = materialdatas.getMaterialData(it->materialId);
                        if(materialData)
                        {
                            const NTextureData* textureData = materialData->getTextureData(NTextureData::Usage::Diffuse);
                            if(textureData)
                            {
								DX11Texture *tex = pLoader->getTexture(textureData->filename, true); // ContextInst->fetchTexture(textureData->filename, false);
								mesh->setTexture(tex, NTextureData::Usage::Diffuse);
                                if(tex)
                                {
									tex->unref();
									bool bTransparent = (materialData->getTextureData(NTextureData::Usage::Transparency) != nullptr);
                                    mesh->_isTransparent = bTransparent;
                                }
                            }
                            textureData = materialData->getTextureData(NTextureData::Usage::Normal);
                            if (textureData)
                            {
								DX11Texture *tex = pLoader->getTexture(textureData->filename); //ContextInst->fetchTexture(textureData->filename, false);
                                mesh->setTexture(tex, NTextureData::Usage::Normal);
								if (tex != NULL)tex->unref();
                            }
                        }
                    }
                    
                    Vec3 pos;
                    Quaternion qua;
                    Vec3 scale;
                    nodedata->transform.decompose(&scale, &qua, &pos);
                    setPosition3D(pos);
                    setRotationQuat(qua);
                    setScaleX(scale.x);
                    setScaleY(scale.y);
                    setScaleZ(scale.z);
                    
                    node = this;
                }
            }
            else
            {
				Sprite3D *sprite = createSprite3DNode(nodedata,it,materialdatas, pLoader);
                if (sprite)
                {
                    if(root)
                    {
                        root->addChild(sprite);
                    } 
                }
                node=sprite;
            } 
        }
    }
    if(nodedata->modelNodeDatas.size() ==0 )
    {
        node= Node::create();
        if(node)
        {
            node->setName(nodedata->id);
            
            // set locale transform
            Vec3 pos;
            Quaternion qua;
            Vec3 scale;
            nodedata->transform.decompose(&scale, &qua, &pos);
            node->setPosition3D(pos);
            node->setRotationQuat(qua);
            node->setScaleX(scale.x);
            node->setScaleY(scale.y);
            node->setScaleZ(scale.z);
            
            if(root)
            {
                root->addChild(node);
            } 
        }
    }

    size_t size = nodedata->children.size();
	for(size_t i=0; i<nodedata->children.size(); ++i)
    {
		NodeData * it = nodedata->children.at(i);
        createNode(it,node, materialdatas, false, pLoader);
    }
}

MeshIndexData* Sprite3D::getMeshIndexData(const std::string& indexId) const
{
	for (size_t i = 0; i < _meshVertexDatas.size(); ++i)
	{
		MeshVertexData *it = _meshVertexDatas.at(i);

		MeshIndexData *index = it->getMeshIndexDataById(indexId);
        if (index)
            return index;
    }
    return nullptr;
}

void  Sprite3D::addMesh(Mesh* mesh)
{
	MeshVertexData *meshVertex = mesh->getMeshIndexData()->_vertexData;
    _meshVertexDatas.pushBack(meshVertex);
    _meshes.pushBack(mesh);
}

Mesh* Sprite3D::getMeshByIndex(int index) const
{
    CCASSERT(index < _meshes.size(), "invalid index");
    return _meshes.at(index);
}

/**get Mesh by Name */
Mesh* Sprite3D::getMeshByName(const std::string& name) const
{
	for(size_t i=0; i<_meshes.size(); ++i)
	{
		Mesh *it = _meshes.at(i);
        if (it->getName() == name)
            return it;
    }
    return nullptr;
}

std::vector<Mesh*> Sprite3D::getMeshArrayByName(const std::string& name) const
{
    std::vector<Mesh*> meshes;
	for (size_t i = 0; i<_meshes.size(); ++i)
	{
		Mesh *it = _meshes.at(i);
        if (it->getName() == name)
            meshes.push_back(it);
    }
    return meshes;
}

Mesh* Sprite3D::getMesh() const 
{ 
    if(_meshes.empty())
    {
        return nullptr;
    }
    return _meshes.at(0); 
}

MeshSkin* Sprite3D::getSkin() const
{
	for (size_t i = 0; i<_meshes.size(); ++i)
	{
		Mesh *it = _meshes.at(i);
        if (it->getSkin())
            return it->getSkin();
    }
    return nullptr;
}

void Sprite3D::updateBlendWeights(int meshIndex, const map<int, float> &mapWeights)
{
	if (meshIndex < _meshVertexDatas.size())
	{
		_meshVertexDatas.at(meshIndex)->updateVertexBuffer(mapWeights);
	}
}

void Sprite3D::update(float fTime, float *arrExpAlpha)
{
	Node::update(fTime);
	//update buffer data if have vertex blendshape info

	for (int i = 0; i < 51; ++i)
	{
		arrExpAlpha[i] = arrExpAlpha[i] * 100;
	}
	for (int i = 0; i < _meshVertexDatas.size(); ++i)
	{
		_meshVertexDatas.at(i)->updateVertexBuffer(arrExpAlpha, 51);
	}
}
void Sprite3D::visit(Vector<MeshRenderCmd *> &renderCmds, bool lighting, bool bNPR)
{
	//render...
	Node::visit(renderCmds, lighting);

	if (_skeleton)
		_skeleton->updateBoneMatrix();

	if (m_renderCmds.size() != _meshes.size() || (m_renderCmds.size() > 0 && m_renderCmds.at(0)->getNPR() != bNPR))
	{
		m_renderCmds.clear();

		for (size_t i = 0; i<_meshes.size(); ++i)
		{
			Mesh *it = _meshes.at(i);
			MeshRenderCmd *cmd = MeshRenderCmd::create(it, this->getWTransform(), it->_isTransparent, lighting,bNPR);
			m_renderCmds.pushBack(cmd);
		}
	}

	for (size_t i = 0; i<m_renderCmds.size(); ++i)
	{
		MeshRenderCmd *cmd = m_renderCmds.at(i);
		cmd->updateTransform(this->getWTransform());

		renderCmds.pushBack(cmd);
	}
}

void Sprite3D::updateNeckRotate(float *arrQ)
{
	Quaternion qy(Vec3(1,0,0), -arrQ[1]*2/180.0*3.14f); //y
	Quaternion qz(Vec3(0, 1, 0), -arrQ[2]*2 / 180.0*3.14f); //z
	Quaternion qx(Vec3(0, 0, 1), (arrQ[0]*2+16) / 180.0*3.14f); //x
	Quaternion qRes;
	Quaternion::multiply(qx, qy, &qRes);
	Quaternion::multiply(qRes, qz, &qx);

	Bone3D *boneNeck = _skeleton->getBoneByName("neck");
	if (boneNeck != NULL)
	{
		boneNeck->setAnimationValue(NULL, (float *)&qx, NULL);
	}
}

NS_CC_END
