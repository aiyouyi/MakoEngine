#ifndef __CCNODE_H__
#define __CCNODE_H__

#include <cstdint>
#include "base/CCVector.h"
#include "math/CCAffineTransform.h"
#include "math/CCMath.h"
#include <string>
#include "../renderer/MeshRenderCmd.h"
#include "CCAction.h"
using namespace std;

NS_CC_BEGIN


class CC_DLL Node : public Ref
{
public:
	Node();
	virtual ~Node();

	static Node *create();

	virtual const std::string& getName() const;

	virtual void setName(const std::string& name);

	virtual void startAction(Action *pAction);
	virtual void stopAction(Action *pAction);

	virtual void setPosition3D(const Vec3& position);

	virtual Vec3 getPosition3D() const;

	virtual void setRotation3D(const Vec3& rotation);

	virtual Vec3 getRotation3D() const;

	virtual void setRotationQuat(const Quaternion& quat);

	virtual Quaternion getRotationQuat() const;

	virtual void setScaleX(float scaleX);

	virtual float getScaleX() const;

	virtual void setScaleY(float scaleY);

	virtual float getScaleY() const;

	virtual void setScaleZ(float scaleZ);

	virtual float getScaleZ() const;

	virtual void addChild(Node * child);

	virtual void addChild(Node* child, const std::string &name);

	virtual Node* getChildByName(const std::string& name);

	virtual Vector<Node*>& getChildren() { return _children; }
	virtual const Vector<Node*>& getChildren() const { return _children; }

	virtual size_t getChildrenCount();

	virtual void setParent(Node* parent);

	virtual Node* getParent() { return _parent; }
	virtual const Node* getParent() const { return _parent; }

	virtual void removeFromParent();

	virtual void removeChild(Node* child);

	virtual void removeChildByName(const std::string &name);

	virtual void removeAllChildren();

	//‰÷»æÀ≥–Ú....
	virtual void update(float fTime);
	virtual void visit(Vector<MeshRenderCmd *> &renderCmds, bool lighting = false, bool bNPR = false);

	Mat4 getWTransform();

	void notifyTransformUpdate();
protected:
	void updateLocalTransform();
	void updateTransfrom();
private:
	Vec3 m_localRotate;
	Vec3 m_localScale;
	Vec3 m_localTranslate;
	Quaternion m_localQuat;

	Mat4 m_worldTransform;
	Mat4 m_localTransform;
	bool m_localMatDirty;
	bool m_wvMatDirty;

	Node *_parent;
	Vector<Node*> _children;

	list<Action *> _runActions;

	string m_szName;
};
NS_CC_END

#endif // __CCNODE_H__
