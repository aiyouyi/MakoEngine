#include "FSObject.h"



FSObject::FSObject()
{
}


FSObject::~FSObject()
{

	map<string, cocos2d::Action *>::iterator it = m_animations.begin();
	while(it != m_animations.end())
	{
		if (it->second != NULL)
		{
			it->second->release();
		}
		++it;
	}
}

FSObject* FSObject::createFromZip(const std::string &file, HZIP hZip, const std::string &zip)
{
	Sprite3DResourceLoaderFromZip zipLoader(hZip, zip);

	FSObject *object = new (std::nothrow) FSObject();
	if (object && object->initWithFile(file, &zipLoader))
	{
		object->autorelease();
		return object;
	}
	CC_SAFE_DELETE(object);
	return nullptr;
}

void FSObject::addAnimation(string id, cocos2d::Action *pAction)
{
	if (pAction != NULL)
	{
		pAction->retain();
		map<string, cocos2d::Action *>::iterator it = m_animations.find(id);
		if (it == m_animations.end())
		{
			m_animations.insert(std::make_pair(id, pAction));
		}
		else
		{
			it->second->release();
			it->second = pAction;
		}
	}
}

void FSObject::play(const char *szID, ActionCallback pCallBack, void *pUserInfo)
{
	if (szID != NULL)
	{
		map<string, cocos2d::Action *>::iterator it = m_animations.find(szID);
		if (it != m_animations.end() && it->second != NULL)
		{
			it->second->setActionCallback(pCallBack, (Ref *)(pUserInfo));
			this->startAction(it->second);
		}
	}
}

void FSObject::update(float ftime)
{
	Node::update(ftime);
	/*
	map<string, cocos2d::Action *>::iterator it = m_animations.begin();
	while (it != m_animations.end())
	{
		it->second->step(ftime);
		++it;
	}*/
}