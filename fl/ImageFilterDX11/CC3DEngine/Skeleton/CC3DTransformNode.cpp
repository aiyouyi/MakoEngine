#include "CC3DTransformNode.h"
#include "BaseDefine/Define.h"

#define NODE_DIRTY_WORLD 1
#define NODE_DIRTY_BOUNDS 2
#define NODE_DIRTY_HIERARCHY 4
#define NODE_DIRTY_ALL (NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS | NODE_DIRTY_HIERARCHY)

CC3DTransformNode::CC3DTransformNode(const char* id)
	: _firstChild(NULL), _nextSibling(NULL), _prevSibling(NULL), _parent(NULL), _childCount(0), _enabled(true), _tags(NULL),
	_dirtyBits(NODE_DIRTY_ALL)
{
	if (id)
	{
		_id = id;
	}
}

CC3DTransformNode::~CC3DTransformNode()
{
	removeAllChildren();
	SAFE_DELETE(_tags);
}

CC3DTransformNode* CC3DTransformNode::create(const char* id)
{
	return new CC3DTransformNode(id);
}

const char* CC3DTransformNode::getTypeName() const
{
	return "CC3DTransformNode";
}

const char* CC3DTransformNode::getId() const
{
	return _id.c_str();
}

void CC3DTransformNode::setId(const char* id)
{
	if (id)
	{
		_id = id;
	}
}

CC3DTransformNode::Type CC3DTransformNode::getType() const
{
	return CC3DTransformNode::NODE;
}

void CC3DTransformNode::addChild(CC3DTransformNode* child)
{
	//GP_ASSERT(child);

	if (child->_parent == this)
	{
		// This node is already present in our hierarchy
		return;
	}
	//child->addRef();

	// If the item belongs to another hierarchy, remove it first.
	if (child->_parent)
	{
		child->_parent->removeChild(child);
	}
	//else if (child->_scene)
	//{
	//	child->_scene->removeNode(child);
	//}
	// Add child to the end of the list.
	// NOTE: This is different than the original behavior which inserted nodes
	// into the beginning of the list. Although slightly slower to add to the
	// end of the list, it makes scene traversal and drawing order more
	// predictable, so I've changed it.
	if (_firstChild)
	{
		CC3DTransformNode* n = _firstChild;
		while (n->_nextSibling)
			n = n->_nextSibling;
		n->_nextSibling = child;
		child->_prevSibling = n;
	}
	else
	{
		_firstChild = child;
	}
	child->_parent = this;
	++_childCount;
	//setBoundsDirty();

	//if (_dirtyBits & NODE_DIRTY_HIERARCHY)
	//{
	//	hierarchyChanged();
	//}
}

void CC3DTransformNode::addToSingleListEnd(CC3DTransformNode* child)
{
	if (child->_parent == this)
	{
		return;
	}

	if (child->_parent)
	{
		child->_parent->removeChild(child);
	}

	if (_firstChild == nullptr)
	{
		_firstChild = child;
		child->_parent = this;
		++_childCount;
	}
	else
	{
		if (_nextSibling == nullptr && _prevSibling == nullptr )
		{
			_firstChild->addToSingleListEnd(child);
		}
	}
}

void CC3DTransformNode::removeChild(CC3DTransformNode* child)
{
	//if (child == NULL || child->_parent != this)
	//{
	//	// The child is not in our hierarchy.
	//	return;
	//}
	//// Call remove on the child.
	//child->remove();
	//SAFE_RELEASE(child);
}

void CC3DTransformNode::removeAllChildren()
{
	//_dirtyBits &= ~NODE_DIRTY_HIERARCHY;
	//while (_firstChild)
	//{
	//	removeChild(_firstChild);
	//}
	//_dirtyBits |= NODE_DIRTY_HIERARCHY;
	//hierarchyChanged();
}

void CC3DTransformNode::remove()
{
	//// Re-link our neighbours.
	//if (_prevSibling)
	//{
	//	_prevSibling->_nextSibling = _nextSibling;
	//}
	//if (_nextSibling)
	//{
	//	_nextSibling->_prevSibling = _prevSibling;
	//}
	//// Update our parent.
	//CC3DTransformNode* parent = _parent;
	//if (parent)
	//{
	//	if (this == parent->_firstChild)
	//	{
	//		parent->_firstChild = _nextSibling;
	//	}
	//	--parent->_childCount;
	//}
	//_nextSibling = NULL;
	//_prevSibling = NULL;
	//_parent = NULL;

	//if (parent && parent->_dirtyBits & NODE_DIRTY_HIERARCHY)
	//{
	//	parent->hierarchyChanged();
	//}
}

CC3DTransformNode* CC3DTransformNode::getFirstChild() const
{
	return _firstChild;
}

CC3DTransformNode* CC3DTransformNode::getNextSibling() const
{
	return _nextSibling;
}

CC3DTransformNode* CC3DTransformNode::getPreviousSibling() const
{
	return _prevSibling;
}

CC3DTransformNode* CC3DTransformNode::getParent() const
{
	return _parent;
}

unsigned int CC3DTransformNode::getChildCount() const
{
	return _childCount;
}

CC3DTransformNode* CC3DTransformNode::getRootNode() const
{
	CC3DTransformNode* n = const_cast<CC3DTransformNode*>(this);
	while (n->getParent())
	{
		n = n->getParent();
	}
	return n;
}

CC3DTransformNode* CC3DTransformNode::findNode(const char* id, bool recursive, bool exactMatch) const
{
	return findNode(id, recursive, exactMatch, false);
	return nullptr;
}

CC3DTransformNode* CC3DTransformNode::findNode(const char* id, bool recursive, bool exactMatch, bool skipSkin) const
{
	if (id == this->_id)
	{
		return const_cast<CC3DTransformNode*>(this);
	}
	//return nullptr;

	//GP_ASSERT(id);

	// If not skipSkin hierarchy, try searching the skin hierarchy
	//if (!skipSkin)
	//{
	//	// If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
	//	CC3DTransformNode* rootNode = NULL;
	//	Model* model = dynamic_cast<Model*>(_drawable);
	//	if (model)
	//	{
	//		if (model->getSkin() != NULL && (rootNode = model->getSkin()->_rootNode) != NULL)
	//		{
	//			if ((exactMatch && rootNode->_id == id) || (!exactMatch && rootNode->_id.find(id) == 0))
	//				return rootNode;

	//			CC3DTransformNode* match = rootNode->findNode(id, true, exactMatch, true);
	//			if (match)
	//			{
	//				return match;
	//			}
	//		}
	//	}
	//}
	// Search immediate children first.
	for (CC3DTransformNode* child = getFirstChild(); child != NULL; child = child->getNextSibling())
	{
		// Does this child's ID match?
		if ((exactMatch && child->_id == id) || (!exactMatch && child->_id.find(id) == 0))
		{
			return child;
		}
	}
	// Recurse.
	if (recursive)
	{
		for (CC3DTransformNode* child = getFirstChild(); child != NULL; child = child->getNextSibling())
		{
			CC3DTransformNode* match = child->findNode(id, true, exactMatch, skipSkin);
			if (match)
			{
				return match;
			}
		}
	}
	return nullptr;
}

unsigned int CC3DTransformNode::findNodes(const char* id, std::vector<CC3DTransformNode*>& nodes, bool recursive, bool exactMatch) const
{
	return findNodes(id, nodes, recursive, exactMatch, false);
}

unsigned int CC3DTransformNode::findNodes(const char* id, std::vector<CC3DTransformNode*>& nodes, bool recursive, bool exactMatch, bool skipSkin) const
{
	return 0;
// 	GP_ASSERT(id);
// 
 	// If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
 	unsigned int count = 0;
 
 	//if (!skipSkin)
 	//{
 	//	CC3DTransformNode* rootNode = NULL;
 	//	Model* model = dynamic_cast<Model*>(_drawable);
 	//	if (model)
 	//	{
 	//		if (model->getSkin() != NULL && (rootNode = model->getSkin()->_rootNode) != NULL)
 	//		{
 	//			if ((exactMatch && rootNode->_id == id) || (!exactMatch && rootNode->_id.find(id) == 0))
 	//			{
 	//				nodes.push_back(rootNode);
 	//				++count;
 	//			}
 	//			count += rootNode->findNodes(id, nodes, recursive, exactMatch, true);
 	//		}
 	//	}
 	//}
 
 	// Search immediate children first.
 	for (CC3DTransformNode* child = getFirstChild(); child != NULL; child = child->getNextSibling())
 	{
 		// Does this child's ID match?
 		if ((exactMatch && child->_id == id) || (!exactMatch && child->_id.find(id) == 0))
 		{
 			nodes.push_back(child);
 			++count;
 		}
 	}
 	// Recurse.
 	if (recursive)
 	{
 		for (CC3DTransformNode* child = getFirstChild(); child != NULL; child = child->getNextSibling())
 		{
 			count += child->findNodes(id, nodes, recursive, exactMatch, skipSkin);
 		}
 	}
 
 	return count;
}


bool CC3DTransformNode::hasTag(const char* name) const
{
	//GP_ASSERT(name);
	return (_tags ? _tags->find(name) != _tags->end() : false);
}

const char* CC3DTransformNode::getTag(const char* name) const
{
	//GP_ASSERT(name);

	if (!_tags)
		return NULL;

	std::map<std::string, std::string>::const_iterator itr = _tags->find(name);
	return (itr == _tags->end() ? NULL : itr->second.c_str());
}

void CC3DTransformNode::setTag(const char* name, const char* value)
{
	//GP_ASSERT(name);

	if (value == NULL)
	{
		// Removing tag
		if (_tags)
		{
			_tags->erase(name);
			if (_tags->size() == 0)
			{
				SAFE_DELETE(_tags);
			}
		}
	}
	else
	{
		// Setting tag
		if (_tags == NULL)
		{
			_tags = new std::map<std::string, std::string>();
		}
		(*_tags)[name] = value;
	}
}

void CC3DTransformNode::setEnabled(bool enabled)
{
	//if (_enabled != enabled)
	//{
	//	if (_collisionObject)
	//	{
	//		_collisionObject->setEnabled(enabled);
	//	}
	//	_enabled = enabled;
	//}
}

bool CC3DTransformNode::isEnabled() const
{
	return _enabled;
}

bool CC3DTransformNode::isEnabledInHierarchy() const
{
	//if (!_enabled)
	//	return false;

	//CC3DTransformNode* node = _parent;
	//while (node)
	//{
	//	if (!node->_enabled)
	//	{
	//		return false;
	//	}
	//	node = node->_parent;
	//}
	//return true;
}

void CC3DTransformNode::update(float elapsedTime)
{
	for (CC3DTransformNode* node = _firstChild; node != NULL; node = node->_nextSibling)
	{
		if (node->isEnabled())
		{
			node->update(elapsedTime);
		}
	}
	//fireScriptEvent<void>(GP_GET_SCRIPT_EVENT(CC3DTransformNode, update), dynamic_cast<void*>(this), elapsedTime);
}

bool CC3DTransformNode::isStatic() const
{
	//return (_collisionObject && _collisionObject->isStatic());
	return false;
}

const CC3DMath::CC3DMatrix& CC3DTransformNode::getWorldMatrix() const
{
	if (_dirtyBits & NODE_DIRTY_WORLD)
	{
		// Clear our dirty flag immediately to prevent this block from being entered if our
		// parent calls our getWorldMatrix() method as a result of the following calculations.
		_dirtyBits &= ~NODE_DIRTY_WORLD;

		if (!isStatic())
		{
			// If we have a parent, multiply our parent world transform by our local
			// transform to obtain our final resolved world transform.
			CC3DTransformNode* parent = getParent();
			if (parent /*&& (!_collisionObject || _collisionObject->isKinematic())*/)
			{
				//Matrix::multiply(parent->getWorldMatrix(), getMatrix(), &_world);
				_world = parent->getWorldMatrix() * getMatrix();
			}
			else
			{
				_world = getMatrix();
			}

			// Our world matrix was just updated, so call getWorldMatrix() on all child nodes
			// to force their resolved world matrices to be updated.
			for (CC3DTransformNode* child = getFirstChild(); child != NULL; child = child->getNextSibling())
			{
				child->getWorldMatrix();
			}
		}
	}
	return _world;
}

//const Matrix& CC3DTransformNode::getWorldViewMatrix() const
//{
//	static Matrix worldView;
//	Matrix::multiply(getViewMatrix(), getWorldMatrix(), &worldView);
//	return worldView;
//}

//const Matrix& CC3DTransformNode::getInverseTransposeWorldViewMatrix() const
//{
//	static Matrix invTransWorldView;
//	Matrix::multiply(getViewMatrix(), getWorldMatrix(), &invTransWorldView);
//	invTransWorldView.invert();
//	invTransWorldView.transpose();
//	return invTransWorldView;
//}

//const Matrix& CC3DTransformNode::getInverseTransposeWorldMatrix() const
//{
//	static Matrix invTransWorld;
//	invTransWorld = getWorldMatrix();
//	invTransWorld.invert();
//	invTransWorld.transpose();
//	return invTransWorld;
//}

//const Matrix& CC3DTransformNode::getViewMatrix() const
//{
//	Scene* scene = getScene();
//	Camera* camera = scene ? scene->getActiveCamera() : NULL;
//	if (camera)
//	{
//		return camera->getViewMatrix();
//	}
//	else
//	{
//		return Matrix::identity();
//	}
//}

//const Matrix& CC3DTransformNode::getInverseViewMatrix() const
//{
//	Scene* scene = getScene();
//	Camera* camera = scene ? scene->getActiveCamera() : NULL;
//	if (camera)
//	{
//		return camera->getInverseViewMatrix();
//	}
//	else
//	{
//		return Matrix::identity();
//	}
//}

//const Matrix& CC3DTransformNode::getProjectionMatrix() const
//{
//	Scene* scene = getScene();
//	Camera* camera = scene ? scene->getActiveCamera() : NULL;
//	if (camera)
//	{
//		return camera->getProjectionMatrix();
//	}
//	else
//	{
//		return Matrix::identity();
//	}
//}

//const Matrix& CC3DTransformNode::getViewProjectionMatrix() const
//{
//	Scene* scene = getScene();
//	Camera* camera = scene ? scene->getActiveCamera() : NULL;
//	if (camera)
//	{
//		return camera->getViewProjectionMatrix();
//	}
//	else
//	{
//		return Matrix::identity();
//	}
//}

//const Matrix& CC3DTransformNode::getInverseViewProjectionMatrix() const
//{
//	Scene* scene = getScene();
//	Camera* camera = scene ? scene->getActiveCamera() : NULL;
//	if (camera)
//	{
//		return camera->getInverseViewProjectionMatrix();
//	}
//	return Matrix::identity();
//}

//const Matrix& CC3DTransformNode::getWorldViewProjectionMatrix() const
//{
//	// Always re-calculate worldViewProjection matrix since it's extremely difficult
//	// to track whether the camera has changed (it may frequently change every frame).
//	static Matrix worldViewProj;
//	Matrix::multiply(getViewProjectionMatrix(), getWorldMatrix(), &worldViewProj);
//	return worldViewProj;
//}

Vector3 CC3DTransformNode::getTranslationWorld() const
{
	Vector3 translation;
	getWorldMatrix().getTranslation(&translation);
	return translation;
}

//Vector3 CC3DTransformNode::getTranslationView() const
//{
//	Vector3 translation;
//	getWorldMatrix().getTranslation(&translation);
//	getViewMatrix().transformPoint(&translation);
//	return translation;
//}

//Vector3 CC3DTransformNode::getForwardVectorWorld() const
//{
//	Vector3 vector;
//	getWorldMatrix().getForwardVector(&vector);
//	return vector;
//}

//Vector3 CC3DTransformNode::getForwardVectorView() const
//{
//	Vector3 vector;
//	getWorldMatrix().getForwardVector(&vector);
//	getViewMatrix().transformVector(&vector);
//	return vector;
//}

//Vector3 CC3DTransformNode::getRightVectorWorld() const
//{
//	Vector3 vector;
//	getWorldMatrix().getRightVector(&vector);
//	return vector;
//}

Vector3 CC3DTransformNode::getUpVectorWorld() const
{
	Vector3 vector;
	getWorldMatrix().getUpVector(&vector);
	return vector;
}

//Vector3 CC3DTransformNode::getActiveCameraTranslationWorld() const
//{
//	Scene* scene = getScene();
//	if (scene)
//	{
//		Camera* camera = scene->getActiveCamera();
//		if (camera)
//		{
//			CC3DTransformNode* cameraNode = camera->getNode();
//			if (cameraNode)
//			{
//				return cameraNode->getTranslationWorld();
//			}
//		}
//	}
//	return Vector3::zero();
//}

// Vector3 CC3DTransformNode::getActiveCameraTranslationView() const
// {
// 	Scene* scene = getScene();
// 	if (scene)
// 	{
// 		Camera* camera = scene->getActiveCamera();
// 		if (camera)
// 		{
// 			CC3DTransformNode* cameraNode = camera->getNode();
// 			if (cameraNode)
// 			{
// 				return cameraNode->getTranslationView();
// 			}
// 		}
// 	}
// 	return Vector3::zero();
// }

//void CC3DTransformNode::hierarchyChanged()
//{
//	// When our hierarchy changes our world transform is affected, so we must dirty it.
//	_dirtyBits |= NODE_DIRTY_HIERARCHY;
//	transformChanged();
//}

//void CC3DTransformNode::transformChanged()
//{
//	// Our local transform was changed, so mark our world matrices dirty.
//	_dirtyBits |= NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS;
//
//	// Notify our children that their transform has also changed (since transforms are inherited).
//	for (CC3DTransformNode* n = getFirstChild(); n != NULL; n = n->getNextSibling())
//	{
//		if (Transform::isTransformChangedSuspended())
//		{
//			// If the DIRTY_NOTIFY bit is not set
//			if (!n->isDirty(Transform::DIRTY_NOTIFY))
//			{
//				n->transformChanged();
//				suspendTransformChange(n);
//			}
//		}
//		else
//		{
//			n->transformChanged();
//		}
//	}
//	Transform::transformChanged();
//}

//void CC3DTransformNode::setBoundsDirty()
//{
//	// Mark ourself and our parent nodes as dirty
//	_dirtyBits |= NODE_DIRTY_BOUNDS;
//
//	// Mark our parent bounds as dirty as well
//	if (_parent)
//		_parent->setBoundsDirty();
//}


//const BoundingSphere& CC3DTransformNode::getBoundingSphere() const
//{
//	if (_dirtyBits & NODE_DIRTY_BOUNDS)
//	{
//		_dirtyBits &= ~NODE_DIRTY_BOUNDS;
//
//		const Matrix& worldMatrix = getWorldMatrix();
//
//		// Start with our local bounding sphere
//		// TODO: Incorporate bounds from entities other than mesh (i.e. particleemitters, audiosource, etc)
//		bool empty = true;
//		Terrain* terrain = dynamic_cast<Terrain*>(_drawable);
//		if (terrain)
//		{
//			_bounds.set(terrain->getBoundingBox());
//			empty = false;
//		}
//		Model* model = dynamic_cast<Model*>(_drawable);
//		if (model && model->getMesh())
//		{
//			if (empty)
//			{
//				_bounds.set(model->getMesh()->getBoundingSphere());
//				empty = false;
//			}
//			else
//			{
//				_bounds.merge(model->getMesh()->getBoundingSphere());
//			}
//		}
//		if (_light)
//		{
//			switch (_light->getLightType())
//			{
//			case Light::POINT:
//				if (empty)
//				{
//					_bounds.set(Vector3::zero(), _light->getRange());
//					empty = false;
//				}
//				else
//				{
//					_bounds.merge(BoundingSphere(Vector3::zero(), _light->getRange()));
//				}
//				break;
//			case Light::SPOT:
//				// TODO: Implement spot light bounds
//				break;
//			}
//		}
//		if (empty)
//		{
//			// Empty bounding sphere, set the world translation with zero radius
//			worldMatrix.getTranslation(&_bounds.center);
//			_bounds.radius = 0;
//		}
//
//		// Transform the sphere (if not empty) into world space.
//		if (!empty)
//		{
//			bool applyWorldTransform = true;
//			if (model && model->getSkin())
//			{
//				// Special case: If the root joint of our mesh skin is parented by any nodes, 
//				// multiply the world matrix of the root joint's parent by this node's
//				// world matrix. This computes a final world matrix used for transforming this
//				// node's bounding volume. This allows us to store a much smaller bounding
//				// volume approximation than would otherwise be possible for skinned meshes,
//				// since joint parent nodes that are not in the matrix palette do not need to
//				// be considered as directly transforming vertices on the GPU (they can instead
//				// be applied directly to the bounding volume transformation below).
//				GP_ASSERT(model->getSkin()->getRootJoint());
//				CC3DTransformNode* jointParent = model->getSkin()->getRootJoint()->getParent();
//				if (jointParent)
//				{
//					// TODO: Should we protect against the case where joints are nested directly
//					// in the node hierachy of the model (this is normally not the case)?
//					Matrix boundsMatrix;
//					Matrix::multiply(getWorldMatrix(), jointParent->getWorldMatrix(), &boundsMatrix);
//					_bounds.transform(boundsMatrix);
//					applyWorldTransform = false;
//				}
//			}
//			if (applyWorldTransform)
//			{
//				_bounds.transform(getWorldMatrix());
//			}
//		}
//
//		// Merge this world-space bounding sphere with our childrens' bounding volumes.
//		for (CC3DTransformNode* n = getFirstChild(); n != NULL; n = n->getNextSibling())
//		{
//			const BoundingSphere& childSphere = n->getBoundingSphere();
//			if (!childSphere.isEmpty())
//			{
//				if (empty)
//				{
//					_bounds.set(childSphere);
//					empty = false;
//				}
//				else
//				{
//					_bounds.merge(childSphere);
//				}
//			}
//		}
//	}
//
//	return _bounds;
//}

//CC3DTransformNode* CC3DTransformNode::clone() const
//{
//	NodeCloneContext context;
//	return cloneRecursive(context);
//}

//CC3DTransformNode* CC3DTransformNode::cloneSingleNode(NodeCloneContext &context) const
//{
//	CC3DTransformNode* copy = CC3DTransformNode::create(getId());
//	context.registerClonedNode(this, copy);
//	cloneInto(copy, context);
//	return copy;
//}
//
//CC3DTransformNode* CC3DTransformNode::cloneRecursive(NodeCloneContext &context) const
//{
//	CC3DTransformNode* copy = cloneSingleNode(context);
//	GP_ASSERT(copy);
//
//	// Add child nodes
//	for (CC3DTransformNode* child = getFirstChild(); child != NULL; child = child->getNextSibling())
//	{
//		CC3DTransformNode* childCopy = child->cloneRecursive(context);
//		GP_ASSERT(childCopy);
//		copy->addChild(childCopy);
//		childCopy->release();
//	}
//
//	return copy;
//}

//void CC3DTransformNode::cloneInto(CC3DTransformNode* node, NodeCloneContext& context) const
//{
//	GP_ASSERT(node);
//
//	Transform::cloneInto(node, context);
//
//	if (Drawable* drawable = getDrawable())
//	{
//		Drawable* clone = drawable->clone(context);
//		node->setDrawable(clone);
//		Ref* ref = dynamic_cast<Ref*>(clone);
//		if (ref)
//			ref->release();
//	}
//	if (Camera* camera = getCamera())
//	{
//		Camera* clone = camera->clone(context);
//		node->setCamera(clone);
//		Ref* ref = dynamic_cast<Ref*>(clone);
//		if (ref)
//			ref->release();
//	}
//	if (Light* light = getLight())
//	{
//		Light* clone = light->clone(context);
//		node->setLight(clone);
//		Ref* ref = dynamic_cast<Ref*>(clone);
//		if (ref)
//			ref->release();
//	}
//	if (AudioSource* audio = getAudioSource())
//	{
//		AudioSource* clone = audio->clone(context);
//		node->setAudioSource(clone);
//		Ref* ref = dynamic_cast<Ref*>(clone);
//		if (ref)
//			ref->release();
//	}
//	if (_tags)
//	{
//		node->_tags = new std::map<std::string, std::string>(_tags->begin(), _tags->end());
//	}
//
//	node->_world = _world;
//	node->_bounds = _bounds;
//
//	// TODO: Clone the rest of the node data.
//}




NodeCloneContext::NodeCloneContext()
{
}

NodeCloneContext::~NodeCloneContext()
{
}




CC3DTransformNode* NodeCloneContext::findClonedNode(const CC3DTransformNode* node)
{
	//GP_ASSERT(node);

	std::map<const CC3DTransformNode*, CC3DTransformNode*>::iterator it = _clonedNodes.find(node);
	return it != _clonedNodes.end() ? it->second : NULL;
}

void NodeCloneContext::registerClonedNode(const CC3DTransformNode* original, CC3DTransformNode* clone)
{
	//GP_ASSERT(original);
	//GP_ASSERT(clone);

	_clonedNodes[original] = clone;
}