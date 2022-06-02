/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#include "2d/CCAction.h"
#include "2d/CCActionInterval.h"
#include "2d/CCNode.h"

NS_CC_BEGIN
//
// Action Base Class
//

Action::Action()
:_originalTarget(nullptr)
,_target(nullptr)
,_tag(Action::INVALID_TAG)
,_flags(0)
{
	_pActionCallback = NULL;
	_pUserInfo = NULL;
}

Action::~Action()
{
	if (_pUserInfo != NULL)
	{
		_pUserInfo->release();
	}
}

void Action::setActionCallback(ActionCallback pCallBack, Ref *pUserInfo)
{
	_pActionCallback = pCallBack;

	if (_pUserInfo != NULL)
	{
		_pUserInfo->release();
	}
	_pUserInfo = pUserInfo;
	if (_pUserInfo != NULL)
	{
		_pUserInfo->retain();
	}
}

std::string Action::description() const
{
	char szInfo[256];
	sprintf(szInfo, "<Action | Tag = %d", _tag);
    return szInfo;
}

void Action::startWithTarget(Node *aTarget)
{
	if (_pActionCallback != NULL)
	{
		(*_pActionCallback)(AE_START, _pUserInfo);
	}
    _originalTarget = _target = aTarget;
}

void Action::stop()
{
    _target = nullptr;
}

bool Action::isDone() const
{
    return true;
}

void Action::step(float /*dt*/)
{
}

void Action::update(float /*time*/)
{
}

//
// Speed
//
Speed::Speed()
: _speed(0.0)
, _innerAction(nullptr)
{
}

Speed::~Speed()
{
    CC_SAFE_RELEASE(_innerAction);
}

Speed* Speed::create(ActionInterval* action, float speed)
{
    Speed *ret = new (std::nothrow) Speed();
    if (ret && ret->initWithAction(action, speed))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Speed::initWithAction(ActionInterval *action, float speed)
{
    if (action == nullptr)
    {
        return false;
    }
    
    action->retain();
    _innerAction = action;
    _speed = speed;
    return true;
}

Speed *Speed::clone() const
{
    // no copy constructor
    if (_innerAction)
        return Speed::create(_innerAction->clone(), _speed);
    
    return nullptr;
}

void Speed::startWithTarget(Node* target)
{
    if (target && _innerAction)
    {
        Action::startWithTarget(target);
        _innerAction->startWithTarget(target);
    }
}

void Speed::reset()
{
	if (_innerAction != NULL)
	{
		_innerAction->reset();
	}
}

void Speed::stop()
{
    if (_innerAction)
        _innerAction->stop();
    
    Action::stop();
}

void Speed::step(float dt)
{
    _innerAction->step(dt * _speed);
}

bool Speed::isDone() const
{
    return _innerAction->isDone();
}

Speed *Speed::reverse() const
{
    if (_innerAction)
        return Speed::create(_innerAction->reverse(), _speed);
    
    return nullptr;
}

void Speed::setInnerAction(ActionInterval *action)
{
    if (_innerAction != action)
    {
        CC_SAFE_RELEASE(_innerAction);
        _innerAction = action;
        CC_SAFE_RETAIN(_innerAction);
    }
}

NS_CC_END


