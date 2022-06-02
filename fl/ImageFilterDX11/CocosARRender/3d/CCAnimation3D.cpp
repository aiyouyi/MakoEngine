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

#include "3d/CCAnimation3D.h"
#include "3d/CCBundle3D.h"
#include "platform/CCFileUtils.h"

NS_CC_BEGIN

Animation3D* Animation3D::create(const std::string& fileName, const std::string& animationName)
{
    std::string fullPath = fileName;
    std::string key = fullPath + "#" + animationName;
    //auto animation = Animation3DCache::getInstance()->getAnimation(key);
    //if (animation != nullptr)
        //return animation;
    
	Sprite3DResourceLoaderFromFile fileLoader;

	auto animation = new (std::nothrow) Animation3D();
    if(animation->initWithFile(fileName, animationName, &fileLoader))
    {
        animation->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(animation);
    }
    
    return animation;
}

Animation3D* Animation3D::createFromZip(const std::string& filename, HZIP hZip, const std::string &zip, const std::string& animationName)
{
	std::string fullPath = zip + "/" + filename;
	std::string key = fullPath + "#" + animationName;
	//auto animation = Animation3DCache::getInstance()->getAnimation(key);
	//if (animation != nullptr)
		//return animation;

	Sprite3DResourceLoaderFromZip zipLoader(hZip, zip);

	auto animation = new (std::nothrow) Animation3D();
	if (animation->initWithFile(filename, animationName, &zipLoader))
	{
		animation->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(animation);
	}

	return animation;
}

bool Animation3D::initWithFile(const std::string& filename, const std::string& animationName, Sprite3DResourceLoader *pLoader)
{
    std::string fullPath = filename;
    
    //load animation here
    auto bundle = Bundle3D::createBundle();
    Animation3DData animationdata;
    if (bundle->load(fullPath, pLoader) && bundle->loadAnimationData(animationName, &animationdata) && init(animationdata))
    {
        //std::string key = fullPath + "#" + animationName;
        //Animation3DCache::getInstance()->addAnimation(key, this);
        Bundle3D::destroyBundle(bundle);
        return true;
    }
    
    Bundle3D::destroyBundle(bundle);
    
    return false;
}

Animation3D::Curve* Animation3D::getBoneCurveByName(const std::string& name) const
{
    auto it = _boneCurves.find(name);
    if (it != _boneCurves.end())
        return it->second;
    
    return nullptr;
}

Animation3D::Animation3D()
: _duration(0)
{
    
}

Animation3D::~Animation3D()
{
    for (auto itor : _boneCurves) {
        CC_SAFE_DELETE(itor.second);
    }

	MeshCurveMap_::iterator it = _meshBlendCurves.begin();
	while (it != _meshBlendCurves.end())
	{
		ChanelCurveMap_ &ccm = it->second;
		for (auto itCCM : ccm)
		{
			CurveBlendShape* pCBS = itCCM.second;
			if (pCBS != NULL)
			{
				delete pCBS;
			}
		}

		++it;
	}

	_meshBlendCurves.clear();
}

Animation3D::Curve::Curve()
: translateCurve(nullptr)
, rotCurve(nullptr)
, scaleCurve(nullptr)
{
    
}
Animation3D::Curve::~Curve()
{
    CC_SAFE_RELEASE_NULL(translateCurve);
    CC_SAFE_RELEASE_NULL(rotCurve);
    CC_SAFE_RELEASE_NULL(scaleCurve);
}


//AnimationCurveVec1 *blendCurve;
Animation3D::CurveBlendShape::CurveBlendShape()
{
	blendCurve = NULL;
}
Animation3D::CurveBlendShape::~CurveBlendShape()
{
	CC_SAFE_RELEASE_NULL(blendCurve);
}

bool Animation3D::init(const Animation3DData &data)
{
    _duration = data._totalTime;

    for(const auto& iter : data._translationKeys)
    {
        Curve* curve = _boneCurves[iter.first];
        if( curve == nullptr)
        {
            curve = new (std::nothrow) Curve();
            _boneCurves[iter.first] = curve;
        }
        
        if(iter.second.size() == 0) continue;
        std::vector<float> keys;
        std::vector<float> values;
        for(const auto& keyIter : iter.second)
        {
            keys.push_back(keyIter._time);
            values.push_back(keyIter._key.x);
            values.push_back(keyIter._key.y);
            values.push_back(keyIter._key.z);
        }
        
        curve->translateCurve = Curve::AnimationCurveVec3::create(&keys[0], &values[0], (int)keys.size());
        if(curve->translateCurve) curve->translateCurve->retain();
    }
    
    for(const auto& iter : data._rotationKeys)
    {
        Curve* curve = _boneCurves[iter.first];
        if( curve == nullptr)
        {
            curve = new (std::nothrow) Curve();
            _boneCurves[iter.first] = curve;
        }
        
        if(iter.second.size() == 0) continue;
        std::vector<float> keys;
        std::vector<float> values;
        for(const auto& keyIter : iter.second)
        {
            keys.push_back(keyIter._time);
            values.push_back(keyIter._key.x);
            values.push_back(keyIter._key.y);
            values.push_back(keyIter._key.z);
            values.push_back(keyIter._key.w);
        }
        
        curve->rotCurve = Curve::AnimationCurveQuat::create(&keys[0], &values[0], (int)keys.size());
        if(curve->rotCurve) curve->rotCurve->retain();
    }
    
    for(const auto& iter : data._scaleKeys)
    {
        Curve* curve = _boneCurves[iter.first];
        if( curve == nullptr)
        {
            curve = new (std::nothrow) Curve();
            _boneCurves[iter.first] = curve;
        }
        
        if(iter.second.size() == 0) continue;
        std::vector<float> keys;
        std::vector<float> values;
        for(const auto& keyIter : iter.second)
        {
            keys.push_back(keyIter._time);
            values.push_back(keyIter._key.x);
            values.push_back(keyIter._key.y);
            values.push_back(keyIter._key.z);
        }
        
        curve->scaleCurve = Curve::AnimationCurveVec3::create(&keys[0], &values[0], (int)keys.size());
        if(curve->scaleCurve) curve->scaleCurve->retain();
    }

	for (const auto& iter : data._blendshapeKeys)
	{
		const int &meshIndex = iter.first;

		ChanelCurveMap_ chanelMap;
		for (const auto &iterFrameKeys : iter.second)
		{
			const int &chanelIndex = iterFrameKeys.first;
			const std::vector<Animation3DData::Vec1Key> &keyFrames = iterFrameKeys.second;

			std::vector<float> keys;
			std::vector<float> values;
			for (int iKey = 0; iKey < keyFrames.size(); ++iKey)
			{
				keys.push_back(keyFrames[iKey]._time);
				values.push_back(keyFrames[iKey]._key);
			}

			CurveBlendShape*pCBS = chanelMap[chanelIndex];
			if (pCBS == nullptr)
			{
				pCBS = new (std::nothrow) CurveBlendShape();
				chanelMap[chanelIndex] = pCBS;
			}
			pCBS->blendCurve = CurveBlendShape::AnimationCurveVec1::create(&keys[0], &values[0], keys.size());
			if (pCBS->blendCurve != NULL)
			{
				pCBS->blendCurve->retain();
			}
		}

		if (chanelMap.size() > 0)
		{
			_meshBlendCurves.insert(std::make_pair(meshIndex, chanelMap));
		}
	}
    return true;
}

////////////////////////////////////////////////////////////////
Animation3DCache* Animation3DCache::_cacheInstance = nullptr;

Animation3DCache* Animation3DCache::getInstance()
{
    if (_cacheInstance == nullptr)
        _cacheInstance = new (std::nothrow) Animation3DCache();
    
    return _cacheInstance;
}
void Animation3DCache::destroyInstance()
{
    CC_SAFE_DELETE(_cacheInstance);
}

Animation3D* Animation3DCache::getAnimation(const std::string& key)
{
    auto it = _animations.find(key);
    if (it != _animations.end())
        return it->second;
    return nullptr;
}

void Animation3DCache::addAnimation(const std::string& key, Animation3D* animation)
{
    const auto& it = _animations.find(key);
    if (it != _animations.end())
    {
        return; // already have this key
    }
    _animations[key] = animation;
    animation->retain();
}

void Animation3DCache::removeAllAnimations()
{
    for (auto itor : _animations) {
        CC_SAFE_RELEASE(itor.second);
    }
    _animations.clear();
}
void Animation3DCache::removeUnusedAnimation()
{
    for (auto itor = _animations.begin(); itor != _animations.end(); ) {
        if (itor->second->getReferenceCount() == 1)
        {
            itor->second->release();
            itor = _animations.erase(itor);
        }
        else
            ++itor;
    }
}

Animation3DCache::Animation3DCache()
{
    
}
Animation3DCache::~Animation3DCache()
{
    removeAllAnimations();
}

NS_CC_END
