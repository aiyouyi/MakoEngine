#include "2d/CCNode.h"
NS_CC_BEGIN

Node::Node()
{
	m_localScale = Vec3(1,1,1);

	m_localMatDirty = true;

	m_wvMatDirty = true;

	_parent = NULL;
}
Node::~Node()
{
	_children.clear();
	list<Action *>::iterator it = _runActions.begin();
	while (it != _runActions.end())
	{
		(*it)->release();
		++it;
	}
	_runActions.clear();
}

Node *Node::create()
{
	Node *pNode = new Node();
	pNode->autorelease();
	return pNode;
}

const std::string& Node::getName() const
{
	return m_szName;
}

void Node::setName(const std::string& name)
{
	m_szName = name;
}

void Node::startAction(Action *pAction)
{
	if (pAction != NULL)
	{
		pAction->retain();

		bool bFind = false;
		list<Action *>::iterator it = _runActions.begin();
		while (it != _runActions.end())
		{
			if ((*it) == pAction)
			{
				bFind = true;
			}
			++it;
		}

		pAction->startWithTarget(this);

		if(!bFind)_runActions.push_back(pAction);
	}
}
void Node::stopAction(Action *pAction)
{
	if (pAction == NULL) return;

	list<Action *>::iterator it = _runActions.begin();
	while (it != _runActions.end())
	{
		if ((*it) == pAction)
		{
			pAction->release();
			it = _runActions.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void Node::setPosition3D(const Vec3& position)
{
	m_localTranslate = position;

	m_localMatDirty = true;
}

Vec3 Node::getPosition3D() const
{
	return m_localTranslate;
}

void Node::setRotation3D(const Vec3& rotation)
{
	m_localRotate = rotation;

	//转成四元数

	m_localMatDirty = true;
}

Vec3 Node::getRotation3D() const
{
	return m_localRotate;
}

void Node::setRotationQuat(const Quaternion& quat)
{
	m_localQuat = quat;
	m_localMatDirty = true;
}

Quaternion Node::getRotationQuat() const
{
	return m_localQuat;
}

void Node::setScaleX(float scaleX)
{
	m_localScale.x = scaleX;
	m_localMatDirty = true;
}

float Node::getScaleX() const
{
	return m_localScale.x;
}

void Node::setScaleY(float scaleY)
{
	m_localScale.y = scaleY;
	m_localMatDirty = true;
}

float Node::getScaleY() const
{
	return m_localScale.y;
}

void Node::setScaleZ(float scaleZ)
{
	m_localScale.z = scaleZ;
	m_localMatDirty = true;
}

float Node::getScaleZ() const
{
	return m_localScale.z;
}

void Node::addChild(Node * child)
{
	child->setParent(this);
	_children.pushBack(child);
}

void Node::addChild(Node* child, const std::string &name)
{
	child->setName(name);
	addChild(child);
}

Node* Node::getChildByName(const std::string& name)
{
	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		if ((*it)->getName() == name)
		{
			return (*it);
		}
		++it;
	}

	return NULL;
}


size_t Node::getChildrenCount()
{
	return _children.size();
}

void Node::setParent(Node* parent)
{
	_parent = parent;
	notifyTransformUpdate();
}

void Node::removeFromParent()
{
	_parent = NULL;
	notifyTransformUpdate();
}

void Node::removeChild(Node* child)
{
	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		if ((*it) == child)
		{
			it = _children.erase(it);
			child->removeFromParent();
			break;
		}
		else
		{
			++it;
		}
	}
}

void Node::removeChildByName(const std::string &name)
{
	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		if ((*it)->getName() == name)
		{
			_children.erase(it);
			Node *child = (*it);
			child->removeFromParent();
			break;
		}
		else
		{
			++it;
		}
	}
}

void Node::removeAllChildren()
{
	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		Node *child = (*it);
		child->removeFromParent();

		++it;
	}

	_children.clear();
}

void Node::update(float fTime)
{
	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		Node *child = (*it);
		child->update(fTime);

		++it;
	}

	list<Action *>::iterator itAction = _runActions.begin();
	while (itAction != _runActions.end())
	{
		(*itAction)->step(fTime);
		if ((*itAction)->isDone())
		{
			itAction = _runActions.erase(itAction);
		}
		else
		{
			++itAction;
		}
	}
}

void Node::visit(Vector<MeshRenderCmd *> &renderCmds, bool lighting, bool bNPR)
{
	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		Node *child = (*it);
		child->visit(renderCmds, lighting, bNPR);

		++it;
	}
}

Mat4 Node::getWTransform()
{
	updateTransfrom();

	return m_worldTransform;
}

void Node::updateLocalTransform()
{
	if (m_localMatDirty)
	{
		m_localMatDirty = false;
		notifyTransformUpdate();

		//update local transform...
		Mat4 matScale;
		Mat4::createScale(m_localScale, &matScale);

		Mat4 matRotateX;
		Mat4::createRotationX(m_localRotate.x*3.14159f / 180.f, &matRotateX);

		Mat4 matRotateY;
		Mat4::createRotationY(m_localRotate.y*3.14159f / 180.f, &matRotateY);

		Mat4 matRotateZ;
		Mat4::createRotationZ(m_localRotate.z*3.14159f / 180.f, &matRotateZ);

		Mat4 matRotate = matRotateX * matRotateY*matRotateZ;

		// 		Mat4 matRotate;
		// 		Mat4::createRotation(m_localQuat, &matRotate);

		Mat4 matTrans;
		Mat4::createTranslation(m_localTranslate, &matTrans);

		m_localTransform = matTrans * matRotate*matScale;
	}
}

void Node::updateTransfrom()
{
	updateLocalTransform();

	if (m_wvMatDirty)
	{
		Mat4 matParent;
		if (_parent != NULL)
		{
			matParent = _parent->getWTransform();
		}

		m_worldTransform = matParent * m_localTransform;

		m_wvMatDirty = false;
	}
}

void Node::notifyTransformUpdate()
{
	m_wvMatDirty = true;

	Vector<Node*>::iterator it = _children.begin();
	while (it != _children.end())
	{
		Node *child = (*it);
		child->notifyTransformUpdate();

		++it;
	}
}

NS_CC_END
