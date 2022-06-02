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

#include "2d/CCActionInterval.h"
#include "2d/CCActionInstant.h"
#include <stdarg.h>
#include <algorithm>
#include "2d/CCNode.h"

NS_CC_BEGIN


//
// IntervalAction
//

bool ActionInterval::initWithDuration(float d)
{

    _duration = std::abs(d) <= MATH_EPSILON ? MATH_EPSILON : d;
    _elapsed = 0;
    _firstTick = true;
    _done = false;
    
    return true;
}

bool ActionInterval::sendUpdateEventToScript(float dt, Action *actionObject)
{
    return false;
}

bool ActionInterval::isDone() const
{
    return _done;
}

void ActionInterval::step(float dt)
{
    if (_firstTick)
    {
        _firstTick = false;
        _elapsed = 0;
    }
    else
    {
        _elapsed += dt;
    }
    
    float updateDt = max(0.0f,                                  // needed for rewind. elapsed could be negative
                              min(1.0f, _elapsed / _duration)
                              );

    if (sendUpdateEventToScript(updateDt, this)) return;
    
    this->update(updateDt);

	bool done = _done;
    _done = _elapsed >= _duration;

	if (_done && _done != done && _pActionCallback != NULL && _target != NULL)
	{
		(*_pActionCallback)(AE_END, _pUserInfo);
	}
}

void ActionInterval::reset()
{
	_elapsed = 0.0f;
	_firstTick = true;
	_done = false;
}

void ActionInterval::setAmplitudeRate(float /*amp*/)
{
    // Abstract class needs implementation
}

float ActionInterval::getAmplitudeRate()
{
    // Abstract class needs implementation

    return 0;
}

void ActionInterval::startWithTarget(Node *target)
{
    FiniteTimeAction::startWithTarget(target);
    _elapsed = 0.0f;
    _firstTick = true;
    _done = false;
}
//
// Repeat
//

Repeat* Repeat::create(FiniteTimeAction *action, unsigned int times)
{
    Repeat* repeat = new (std::nothrow) Repeat();
    if (repeat && repeat->initWithAction(action, times))
    {
        repeat->autorelease();
        return repeat;
    }

    delete repeat;
    return nullptr;
}

bool Repeat::initWithAction(FiniteTimeAction *action, unsigned int times)
{
    if (action && ActionInterval::initWithDuration(action->getDuration() * times))
    {
        _times = times;
        _innerAction = action;
        action->retain();

        _actionInstant = dynamic_cast<ActionInstant*>(action) ? true : false;
        //an instant action needs to be executed one time less in the update method since it uses startWithTarget to execute the action
        // minggo: instant action doesn't execute action in Repeat::startWithTarget(), so comment it.
//        if (_actionInstant) 
//        {
//            _times -=1;
//        }
        _total = 0;

        return true;
    }

    return false;
}

Repeat* Repeat::clone() const
{
    // no copy constructor
    return Repeat::create(_innerAction->clone(), _times);
}

Repeat::~Repeat()
{
    CC_SAFE_RELEASE(_innerAction);
}

void Repeat::startWithTarget(Node *target)
{
    _total = 0;
    _nextDt = _innerAction->getDuration() / _duration;
    ActionInterval::startWithTarget(target);
    _innerAction->startWithTarget(target);
}

void Repeat::stop()
{
    _innerAction->stop();
    ActionInterval::stop();
}

// issue #80. Instead of hooking step:, hook update: since it can be called by any 
// container action like Repeat, Sequence, Ease, etc..
void Repeat::update(float dt)
{
    if (dt >= _nextDt)
    {
        while (dt >= _nextDt && _total < _times)
        {
            if (!(sendUpdateEventToScript(1.0f, _innerAction)))
                _innerAction->update(1.0f);
            _total++;

            _innerAction->stop();
            _innerAction->startWithTarget(_target);
            _nextDt = _innerAction->getDuration()/_duration * (_total+1);
        }

        // fix for issue #1288, incorrect end value of repeat
        if (std::abs(dt - 1.0f) < FLT_EPSILON && _total < _times)
        {
            if (!(sendUpdateEventToScript(1.0f, _innerAction)))
                _innerAction->update(1.0f);
            
            _total++;
        }

        // don't set an instant action back or update it, it has no use because it has no duration
        if (!_actionInstant)
        {
            if (_total == _times)
            {
                // minggo: inner action update is invoked above, don't have to invoke it here
//                if (!(sendUpdateEventToScript(1, _innerAction)))
//                    _innerAction->update(1);
                _innerAction->stop();
            }
            else
            {
                // issue #390 prevent jerk, use right update
                if (!(sendUpdateEventToScript(dt - (_nextDt - _innerAction->getDuration()/_duration), _innerAction)))
                    _innerAction->update(dt - (_nextDt - _innerAction->getDuration()/_duration));
            }
        }
    }
    else
    {
        if (!(sendUpdateEventToScript(fmodf(dt * _times,1.0f), _innerAction)))
            _innerAction->update(fmodf(dt * _times,1.0f));
    }
}

bool Repeat::isDone() const
{
    return _total == _times;
}

Repeat* Repeat::reverse() const
{
    return Repeat::create(_innerAction->reverse(), _times);
}

//
// RepeatForever
//
RepeatForever::~RepeatForever()
{
    CC_SAFE_RELEASE(_innerAction);
}

RepeatForever *RepeatForever::create(ActionInterval *action)
{
    RepeatForever *ret = new (std::nothrow) RepeatForever();
    if (ret && ret->initWithAction(action))
    {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool RepeatForever::initWithAction(ActionInterval *action)
{
    if (action == nullptr)
    {
        return false;
    }
    
    action->retain();
    _innerAction = action;
    
    return true;
}

RepeatForever *RepeatForever::clone() const
{
    // no copy constructor
    return RepeatForever::create(_innerAction->clone());
}

void RepeatForever::startWithTarget(Node* target)
{
    ActionInterval::startWithTarget(target);
    _innerAction->startWithTarget(target);
}

void RepeatForever::step(float dt)
{
    _innerAction->step(dt);
    // only action interval should prevent jerk, issue #17808
    if (_innerAction->isDone() && _innerAction->getDuration() > 0 && _innerAction->getTarget() != NULL)
    {
        float diff = _innerAction->getElapsed() - _innerAction->getDuration();
        if (diff > _innerAction->getDuration())
            diff = fmodf(diff, _innerAction->getDuration());
        //_innerAction->startWithTarget(_innerAction->getTarget());
		reset();
        // to prevent jerk. cocos2d-iphone issue #390, 1247
        _innerAction->step(0.0f);
        _innerAction->step(diff);

		if (_pActionCallback != NULL)
		{
			(*_pActionCallback)(AE_REPEAT, _pUserInfo);
		}
    }
}

bool RepeatForever::isDone() const
{
    return false;
}

void RepeatForever::reset()
{
	ActionInterval::reset();
	if (_innerAction != NULL)
	{
		_innerAction->reset();
	}
}

RepeatForever *RepeatForever::reverse() const
{
    return RepeatForever::create(_innerAction->reverse());
}

NS_CC_END
