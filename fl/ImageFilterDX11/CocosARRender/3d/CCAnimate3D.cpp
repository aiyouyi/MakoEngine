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

#include "3d/CCAnimate3D.h"
#include "3d/CCSprite3D.h"
#include "3d/CCSkeleton3D.h"
#include "platform/CCFileUtils.h"

NS_CC_BEGIN
std::unordered_map<Node*, Animate3D*> Animate3D::s_fadeInAnimates;
std::unordered_map<Node*, Animate3D*> Animate3D::s_fadeOutAnimates;
std::unordered_map<Node*, Animate3D*> Animate3D::s_runningAnimates;
float      Animate3D::_transTime = 0.1f;

//create Animate3D using Animation.
Animate3D* Animate3D::create(Animation3D* animation)
{
	auto animate = new (std::nothrow) Animate3D();
	animate->init(animation);
	animate->autorelease();

	return animate;
}

Animate3D* Animate3D::create(Animation3D* animation, float fromTime, float duration)
{
	auto animate = new (std::nothrow) Animate3D();
	animate->init(animation, fromTime, duration);
	animate->autorelease();

	return  animate;
}

Animate3D* Animate3D::createWithFrames(Animation3D* animation, int startFrame, int endFrame, float frameRate)
{
	auto animate = new (std::nothrow) Animate3D();
	animate->initWithFrames(animation, startFrame, endFrame, frameRate);
	animate->autorelease();

	return  animate;
}

bool Animate3D::init(Animation3D* animation)
{
	_animation = animation;
	animation->retain();
	setDuration(animation->getDuration());
	setOriginInterval(animation->getDuration());
	setQuality(Animate3DQuality::QUALITY_LOW);
	return true;
}

bool Animate3D::init(Animation3D* animation, float fromTime, float duration)
{
	float fullDuration = animation->getDuration();
	if (duration > fullDuration - fromTime)
		duration = fullDuration - fromTime;

	_start = fromTime / fullDuration;
	_last = duration / fullDuration; //printf("%f, %f\r\n", _start, _last);
	setDuration(duration);
	setOriginInterval(duration);
	_animation = animation;
	animation->retain();
	setQuality(Animate3DQuality::QUALITY_LOW);
	return true;
}

bool Animate3D::initWithFrames(Animation3D* animation, int startFrame, int endFrame, float frameRate)
{
	float perFrameTime = 1.f / frameRate;
	float fromTime = startFrame * perFrameTime;
	float duration = (endFrame - startFrame) * perFrameTime;
	_frameRate = frameRate;
	init(animation, fromTime, duration);
	return true;
}

/** returns a clone of action */
Animate3D* Animate3D::clone() const
{
	auto animate = const_cast<Animate3D*>(this);
	auto copy = Animate3D::create(animate->_animation);

	copy->_absSpeed = _absSpeed;
	copy->_weight = _weight;
	copy->_elapsed = _elapsed;
	copy->_start = _start;
	copy->_last = _last;
	copy->_playReverse = _playReverse;
	copy->setDuration(animate->getDuration());
	copy->setOriginInterval(animate->getOriginInterval());
	return copy;
}

/** returns a new action that performs the exactly the reverse action */
Animate3D* Animate3D::reverse() const
{
	auto animate = clone();
	animate->_playReverse = !animate->_playReverse;
	return animate;
}

Node* findChildByNameRecursively(Node* node, const std::string &childName)
{
	const std::string& name = node->getName();
	if (name == childName)
		return node;

	const Vector<Node*>& children = node->getChildren();
	for (const auto& child : children)
	{
		Node* findNode = findChildByNameRecursively(child, childName);
		if (findNode)
			return findNode;
	}
	return nullptr;
}

//! called before the action start. It will also set the target.
void Animate3D::startWithTarget(Node *target)
{
	if (target == NULL)return;

	bool needReMap = (_target != target);
	ActionInterval::startWithTarget(target);

	if (needReMap)
	{
		_boneCurves.clear();
		_nodeCurves.clear();

		bool hasCurve = false;
		Sprite3D* sprite = dynamic_cast<Sprite3D*>(target);

		if (sprite)
		{
			if (_animation)
			{
				const std::unordered_map<std::string, Animation3D::Curve*>& boneCurves = _animation->getBoneCurves();
				for (const auto& iter : boneCurves)
				{
					const std::string& boneName = iter.first;
					auto skin = sprite->getSkeleton();
					if (skin)
					{
						auto bone = skin->getBoneByName(boneName);
						if (bone)
						{
							auto curve = _animation->getBoneCurveByName(boneName);
							_boneCurves[bone] = curve;
							hasCurve = true;
						}
						else
						{
							Node* node = nullptr;
							if (target->getName() == boneName)
								node = target;
							else
								node = findChildByNameRecursively(target, boneName);

							if (node)
							{
								auto curve = _animation->getBoneCurveByName(boneName);
								if (curve)
								{
									_nodeCurves[node] = curve;
									hasCurve = true;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			const std::unordered_map<std::string, Animation3D::Curve*>& boneCurves = _animation->getBoneCurves();
			for (const auto& iter : boneCurves)
			{
				const std::string& boneName = iter.first;
				Node* node = nullptr;
				if (target->getName() == boneName)
					node = target;
				else
					node = findChildByNameRecursively(target, boneName);

				if (node)
				{
					auto curve = _animation->getBoneCurveByName(boneName);
					if (curve)
					{
						_nodeCurves[node] = curve;
						hasCurve = true;
					}
				}

			}
		}

		if (!hasCurve)
		{
			CCLOG("warning: no animation found for the skeleton");
		}
	}

	auto runningAction = s_runningAnimates.find(target);
	if (runningAction != s_runningAnimates.end() && !(runningAction->second->isDone()))
	{
		//make the running action fade out
		auto action = (*runningAction).second;
		if (action != this)
		{
			if (_transTime < 0.001f)
			{
				s_runningAnimates[target] = this;
				_state = Animate3D::Animate3DState::Running;
				_weight = 1.0f;
				action->stop();
				//target->stopAction(action);
			}
			else
			{
				s_fadeOutAnimates[target] = action;
				action->_state = Animate3D::Animate3DState::FadeOut;
				action->_accTransTime = 0.0f;
				action->_weight = 1.0f;
				action->_lastTime = 0.f;
				s_runningAnimates.erase(target);
				s_fadeInAnimates[target] = this;
				_accTransTime = 0.0f;
				_state = Animate3D::Animate3DState::FadeIn;
				_weight = 0.f;
				_lastTime = 0.f;
			}
		}
	}
	else
	{
		auto it = s_fadeInAnimates.find(target);
		if (it != s_fadeInAnimates.end())
		{
			if (it->second != this)
			{
				it->second->stop();
			}
			s_fadeInAnimates.erase(it);
		}

		auto itFadeout = s_fadeOutAnimates.find(target);
		if (itFadeout != s_fadeOutAnimates.end())
		{
			if (itFadeout->second != this)
			{
				itFadeout->second->stop();
			}
			s_fadeOutAnimates.erase(itFadeout);
		}

		if (s_runningAnimates[target] != NULL)
		{
			if (s_runningAnimates[target] != this)
			{
				s_runningAnimates[target]->stop();
			}
		}

		s_runningAnimates[target] = this;
		_state = Animate3D::Animate3DState::Running;
		_weight = 1.0f;
	}
}

void Animate3D::stop()
{
	ActionInterval::stop();
}

//! called every frame with it's delta time. DON'T override unless you know what you are doing.
void Animate3D::step(float dt)
{
	ActionInterval::step(dt);
}

void Animate3D::update(float t)
{
	if (_target)
	{
		if (_state == Animate3D::Animate3DState::FadeIn && _lastTime > 0.f)
		{
			float timeT = t;
			while (timeT < _lastTime)
			{
				timeT += 1.0f;
			}
			_accTransTime += (timeT - _lastTime) * getDuration();

			_weight = _accTransTime / _transTime;
			if (_weight >= 1.0f)
			{
				_accTransTime = _transTime;
				_weight = 1.0f;
				_state = Animate3D::Animate3DState::Running;
				s_fadeInAnimates.erase(_target);
				s_runningAnimates[_target] = this;
			}
		}
		else if (_state == Animate3D::Animate3DState::FadeOut && _lastTime > 0.f)
		{
			float timeT = t;
			while (timeT < _lastTime)
			{
				timeT += 1.0f;
			}
			_accTransTime += (timeT - _lastTime) * getDuration();

			_weight = 1 - _accTransTime / _transTime;
			//printf("fadeout %f, %f, %f\r\n", _lastTime, t, _accTransTime);
			if (_weight <= 0.0f)
			{
				_accTransTime = _transTime;
				_weight = 0.0f;

				s_fadeOutAnimates.erase(_target);
				//_target->stopAction(this);
				this->stop();
				return;
			}
		}
		float lastTime = _lastTime;
		_lastTime = t;

		if (_quality != Animate3DQuality::QUALITY_NONE)
		{
			if (_weight > 0.0f)
			{
				float transDst[3], rotDst[4], scaleDst[3];
				float* trans = nullptr, *rot = nullptr, *scale = nullptr;
				if (_playReverse) {
					t = 1 - t;
					lastTime = 1.0f - lastTime;
				}

				t = _start + t * _last;
				lastTime = _start + lastTime * _last;

				for (const auto& it : _boneCurves) {
					auto bone = it.first;
					auto curve = it.second;
					if (curve->translateCurve)
					{
						curve->translateCurve->evaluate(t, transDst, _translateEvaluate);
						trans = &transDst[0];
					}
					if (curve->rotCurve)
					{
						curve->rotCurve->evaluate(t, rotDst, _roteEvaluate);
						rot = &rotDst[0];
					}
					if (curve->scaleCurve)
					{
						curve->scaleCurve->evaluate(t, scaleDst, _scaleEvaluate);
						scale = &scaleDst[0];
					}
					bone->setAnimationValue(trans, rot, scale, this, _weight);
				}

				//compute blendweightinfo
				Sprite3D* sprite = dynamic_cast<Sprite3D*>(_target);
				const Animation3D::MeshCurveMap_ &meshBlendCurves = _animation->get_meshBlendCurves();
				if (sprite != NULL && _state == Animate3D::Animate3DState::Running && meshBlendCurves.size() > 0)
				{
					for (const auto& itMeshCuvers : meshBlendCurves)
					{
						int meshIndex = itMeshCuvers.first;

						map<int, float> mapWeights;
						const Animation3D::ChanelCurveMap_ &chanelMaps = itMeshCuvers.second;
						for (const auto& itChanelCurers : chanelMaps)
						{
							int chanelIndex = itChanelCurers.first;
							const Animation3D::CurveBlendShape::AnimationCurveVec1 *curve = itChanelCurers.second->blendCurve;
							float fValue = 0.0f;
							curve->evaluate(t, &fValue, _translateEvaluate);
							mapWeights.insert(std::make_pair(chanelIndex, fValue));
						}

						sprite->updateBlendWeights(meshIndex, mapWeights);
					}
				}
				/*
				for (const auto& it : _nodeCurves)
				{
					auto node = it.first;
					auto curve = it.second;
					Mat4 transform;
					if (curve->translateCurve)
					{
						curve->translateCurve->evaluate(t, transDst, _translateEvaluate);
						transform.translate(transDst[0], transDst[1], transDst[2]);
					}
					if (curve->rotCurve)
					{
						curve->rotCurve->evaluate(t, rotDst, _roteEvaluate);
						Quaternion qua(rotDst[0], rotDst[1], rotDst[2], rotDst[3]);
						transform.rotate(qua);
					}
					if (curve->scaleCurve)
					{
						curve->scaleCurve->evaluate(t, scaleDst, _scaleEvaluate);
						transform.scale(scaleDst[0], scaleDst[1], scaleDst[2]);
					}
					node->setAdditionalTransform(&transform);
				}*//*
				if (!_keyFrameUserInfos.empty()) {
					float prekeyTime = lastTime * getDuration() * _frameRate;
					float keyTime = t * getDuration() * _frameRate;
					std::vector<Animate3DDisplayedEventInfo*> eventInfos;
					for (auto keyFrame : _keyFrameUserInfos)
					{
						if ((!_playReverse && keyFrame.first >= prekeyTime && keyFrame.first < keyTime)
							|| (_playReverse && keyFrame.first >= keyTime && keyFrame.first < prekeyTime))
						{
							auto& frameEvent = _keyFrameEvent[keyFrame.first];
							if (frameEvent == nullptr)
								frameEvent = new (std::nothrow) EventCustom(Animate3DDisplayedNotification);
							auto eventInfo = &_displayedEventInfo[keyFrame.first];
							eventInfo->target = _target;
							eventInfo->frame = keyFrame.first;
							eventInfo->userInfo = &_keyFrameUserInfos[keyFrame.first];
							eventInfos.push_back(eventInfo);
							frameEvent->setUserData((void*)eventInfo);
						}
					}
					std::sort(eventInfos.begin(), eventInfos.end(), _playReverse ? cmpEventInfoDes : cmpEventInfoAsc);
					for (auto eventInfo : eventInfos) {
						Director::getInstance()->getEventDispatcher()->dispatchEvent(_keyFrameEvent[eventInfo->frame]);
					}
				}*/
			}
		}
	}
}

float Animate3D::getSpeed() const
{
	return _playReverse ? -_absSpeed : _absSpeed;
}
void Animate3D::setSpeed(float speed)
{
	_absSpeed = fabsf(speed);
	_playReverse = speed < 0;
	_duration = _originInterval / _absSpeed;
}

void Animate3D::setWeight(float weight)
{
	CCASSERT(weight >= 0.0f, "invalid weight");
	_weight = fabsf(weight);
}

void Animate3D::setOriginInterval(float interval)
{
	_originInterval = interval;
}

void Animate3D::setQuality(Animate3DQuality quality)
{
	if (quality == Animate3DQuality::QUALITY_HIGH)
	{
		_translateEvaluate = EvaluateType::INT_LINEAR;
		_roteEvaluate = EvaluateType::INT_QUAT_SLERP;
		_scaleEvaluate = EvaluateType::INT_LINEAR;
	}
	else if (quality == Animate3DQuality::QUALITY_LOW)
	{
		_translateEvaluate = EvaluateType::INT_NEAR;
		_roteEvaluate = EvaluateType::INT_NEAR;
		_scaleEvaluate = EvaluateType::INT_NEAR;
	}
	_quality = quality;
}

Animate3DQuality Animate3D::getQuality() const
{
	return _quality;
}
/*
const ValueMap* Animate3D::getKeyFrameUserInfo(int keyFrame) const
{
	auto iter = _keyFrameUserInfos.find(keyFrame);
	if (iter != _keyFrameUserInfos.end())
		return &iter->second;

	return nullptr;
}

ValueMap* Animate3D::getKeyFrameUserInfo(int keyFrame)
{
	auto iter = _keyFrameUserInfos.find(keyFrame);
	if (iter != _keyFrameUserInfos.end())
		return &iter->second;

	return nullptr;
}

void Animate3D::setKeyFrameUserInfo(int keyFrame, const ValueMap &userInfo)
{
	_keyFrameUserInfos[keyFrame] = userInfo;
}
*/
Animate3D::Animate3D()
	: _state(Animate3D::Animate3DState::Running)
	, _animation(nullptr)
	, _absSpeed(1.f)
	, _weight(1.f)
	, _start(0.f)
	, _last(1.f)
	, _playReverse(false)
	, _accTransTime(0.0f)
	, _lastTime(0.0f)
	, _originInterval(0.0f)
	, _frameRate(30.0f)
{
	setQuality(Animate3DQuality::QUALITY_HIGH);
}
Animate3D::~Animate3D()
{
	CC_SAFE_RELEASE(_animation);

	std::unordered_map<Node*, Animate3D*>::iterator it = s_fadeInAnimates.begin();
	while (it != s_fadeInAnimates.end())
	{
		if (it->second == this)
		{
			it = s_fadeInAnimates.erase(it);
		}
		else
		{
			++it;
		}
	}

	it = s_fadeOutAnimates.begin();
	while (it != s_fadeOutAnimates.end())
	{
		if (it->second == this)
		{
			it = s_fadeOutAnimates.erase(it);
		}
		else
		{
			++it;
		}
	}

	it = s_runningAnimates.begin();
	while (it != s_runningAnimates.end())
	{
		if (it->second == this)
		{
			it = s_runningAnimates.erase(it);
		}
		else
		{
			++it;
		}
	}
}

NS_CC_END
