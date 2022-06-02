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

#ifndef __ACTIONS_CCACTION_H__
#define __ACTIONS_CCACTION_H__

#include "base/CCRef.h"
#include "math/CCGeometry.h"
#include "../common.h"

NS_CC_BEGIN

class Node;

enum {
    kActionUpdate
};

/**
 * @addtogroup actions
 * @{
 */

/** 
 * @brief Base class for Action objects.
 */

class CC_DLL Action : public Ref, public Clonable
{
public:
    /** Default tag used for all the actions. */
    static const int INVALID_TAG = -1;
    /**
     * @js NA
     * @lua NA
     */
    virtual std::string description() const;

    /** Returns a clone of action.
     *
     * @return A clone action.
     */
    virtual Action* clone() const
    {
        return nullptr;
    }

    /** Returns a new action that performs the exact reverse of the action. 
     *
     * @return A new action that performs the exact reverse of the action.
     * @js NA
     */
    virtual Action* reverse() const
    {
        return nullptr;
    }

    /** Return true if the action has finished. 
     * 
     * @return Is true if the action has finished.
     */
    virtual bool isDone() const;

    /** Called before the action start. It will also set the target. 
     *
     * @param target A certain target.
     */
    virtual void startWithTarget(Node *target);

	/** reset animation..
	*
	* @param target A certain target.
	*/
	virtual void reset() {}

	virtual void setActionCallback(ActionCallback pCallBack, Ref *_pUserInfo);

    /** 
     * Called after the action has finished. It will set the 'target' to nil.
     * IMPORTANT: You should never call "Action::stop()" manually. Instead, use: "target->stopAction(action);".
     */
    virtual void stop();

    /** Called every frame with it's delta time, dt in seconds. DON'T override unless you know what you are doing. 
     *
     * @param dt In seconds.
     */
    virtual void step(float dt);

    /** 
     * Called once per frame. time a value between 0 and 1.

     * For example:
     * - 0 Means that the action just started.
     * - 0.5 Means that the action is in the middle.
     * - 1 Means that the action is over.
     *
     * @param time A value between 0 and 1.
     */
    virtual void update(float time);
    /** Return certain target.
     *
     * @return A certain target.
     */
    Node* getTarget() const { return _target; }
    /** The action will modify the target properties. 
     *
     * @param target A certain target.
     */
    void setTarget(Node *target) { _target = target; }
    /** Return a original Target. 
     *
     * @return A original Target.
     */
    Node* getOriginalTarget() const { return _originalTarget; }
    /** 
     * Set the original target, since target can be nil.
     * Is the target that were used to run the action. Unless you are doing something complex, like ActionManager, you should NOT call this method.
     * The target is 'assigned', it is not 'retained'.
     * @since v0.8.2
     *
     * @param originalTarget Is 'assigned', it is not 'retained'.
     */
    void setOriginalTarget(Node *originalTarget) { _originalTarget = originalTarget; }
    /** Returns a tag that is used to identify the action easily. 
     *
     * @return A tag.
     */
    int getTag() const { return _tag; }
    /** Changes the tag that is used to identify the action easily. 
     *
     * @param tag Used to identify the action easily.
     */
    void setTag(int tag) { _tag = tag; }
    /** Returns a flag field that is used to group the actions easily.
     *
     * @return A tag.
     */
    unsigned int getFlags() const { return _flags; }
    /** Changes the flag field that is used to group the actions easily.
     *
     * @param flags Used to group the actions easily.
     */
    void setFlags(unsigned int flags) { _flags = flags; }

CC_CONSTRUCTOR_ACCESS:
    Action();
    virtual ~Action();

protected:
    Node    *_originalTarget;
    /** 
     * The "target".
     * The target will be set with the 'startWithTarget' method.
     * When the 'stop' method is called, target will be set to nil.
     * The target is 'assigned', it is not 'retained'.
     */
    Node    *_target;
    /** The action tag. An identifier of the action. */
    int     _tag;
    /** The action flag field. To categorize action into certain groups.*/
    unsigned int _flags;

	ActionCallback _pActionCallback;
	Ref *_pUserInfo;
private:
};

/** @class FiniteTimeAction
 * @brief
 * Base class actions that do have a finite time duration.
 * Possible actions:
 * - An action with a duration of 0 seconds.
 * - An action with a duration of 35.5 seconds.
 * Infinite time actions are valid.
 */
class CC_DLL FiniteTimeAction : public Action
{
public:
    /** Get duration in seconds of the action. 
     *
     * @return The duration in seconds of the action.
     */
    float getDuration() const { return _duration; }
    /** Set duration in seconds of the action. 
     *
     * @param duration In seconds of the action.
     */
    void setDuration(float duration) { _duration = duration; }

    //
    // Overrides
    //
    virtual FiniteTimeAction* reverse() const override
    {
        return nullptr;
    }
    virtual FiniteTimeAction* clone() const override
    {
        return nullptr;
    }

CC_CONSTRUCTOR_ACCESS:
    FiniteTimeAction()
    : _duration(0)
    {}
    virtual ~FiniteTimeAction(){}

protected:
    //! Duration in seconds.
    float _duration;
private:
};

class ActionInterval;
class RepeatForever;

/** @class Speed
 * @brief Changes the speed of an action, making it take longer (speed>1)
 * or shorter (speed<1) time.
 * Useful to simulate 'slow motion' or 'fast forward' effect.
 * @warning This action can't be Sequenceable because it is not an IntervalAction.
 */
class CC_DLL Speed : public Action
{
public:
    /** Create the action and set the speed.
     *
     * @param action An action.
     * @param speed The action speed.
     */
    static Speed* create(ActionInterval* action, float speed);
    /** Return the speed.
     *
     * @return The action speed.
     */
    float getSpeed() const { return _speed; }
    /** Alter the speed of the inner function in runtime. 
     *
     * @param speed Alter the speed of the inner function in runtime.
     */
    void setSpeed(float speed) { _speed = speed; }

    /** Replace the interior action.
     *
     * @param action The new action, it will replace the running action.
     */
    void setInnerAction(ActionInterval *action);
    /** Return the interior action.
     *
     * @return The interior action.
     */
    ActionInterval* getInnerAction() const { return _innerAction; }

	virtual void reset();

    //
    // Override
    //
    virtual Speed* clone() const override;
    virtual Speed* reverse() const override;
    virtual void startWithTarget(Node* target) override;
    virtual void stop() override;
    /**
     * @param dt in seconds.
     */
    virtual void step(float dt) override;
    /** Return true if the action has finished.
     *
     * @return Is true if the action has finished.
     */
    virtual bool isDone() const  override;
    
CC_CONSTRUCTOR_ACCESS:
    Speed();
    virtual ~Speed(void);
    /** Initializes the action. */
    bool initWithAction(ActionInterval *action, float speed);

protected:
    float _speed;
    ActionInterval *_innerAction;

private:
};

/** @class Follow
 * @brief Follow is an action that "follows" a node.
 * Eg:
 * @code
 * layer->runAction(Follow::create(hero));
 * @endcode
 * Instead of using Camera as a "follower", use this action instead.
 * @since v0.99.2
 */

// end of actions group
/// @}

NS_CC_END

#endif // __ACTIONS_CCACTION_H__
