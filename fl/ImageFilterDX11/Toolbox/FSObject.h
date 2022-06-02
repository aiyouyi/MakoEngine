#pragma once
#include "./3d/CCSprite3D.h"
#include "./2d/CCAction.h"
#include <map>
using namespace std;
using namespace cocos2d;
class DX11IMAGEFILTER_EXPORTS_CLASS FSObject :public cocos2d::Sprite3D
{
public:
	FSObject();
	virtual ~FSObject();

	static FSObject *createFromZip(const std::string &file, HZIP hZip, const std::string &zip);

	void addAnimation(string id, cocos2d::Action *pAction);

	void play(const char *szID, ActionCallback pCallBack=NULL, void *pUserInfo=NULL);

	virtual void update(float ftime);

	void getAnimationKeys(vector<string>& keys) {
		for (auto&& ele : m_animations) {
			keys.push_back(ele.first);
		}
	}
	cocos2d::Action* getAnimationAction(string key) {
		if (m_animations.find(key) != m_animations.end()) {
			return m_animations[key];
		}
		return nullptr;
	}
	void eraseAnimation(string key) {
		auto res = m_animations.find(key);
		if (res != m_animations.end()) {
			if(m_animations[key]) m_animations[key]->release();
			m_animations.erase(res);
		}
	}

private:
	map<string, cocos2d::Action *> m_animations;
};