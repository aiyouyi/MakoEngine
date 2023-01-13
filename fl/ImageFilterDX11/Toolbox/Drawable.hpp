//
//  Drawable.hpp
//  FaceDetector
//
//  Created by cc on 2017/6/28.
//  Copyright © 2017年 cc. All rights reserved.
//

#ifndef Drawable_hpp
#define Drawable_hpp
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include <vector>

enum en_EffectLoop_Mode
{
	ELM_ONESHOT,
	ELM_REPEAT
};


class Drawable
{
public:
    Drawable(){}
    virtual ~Drawable(){};
    
    virtual Drawable *Clone() = 0;
	virtual void getSize(int &w, int &h) { w = 0; h = 0; };
	virtual long getDuring() = 0;
    virtual std::shared_ptr<MaterialTexRHI> GetTex(long during) = 0;
    virtual std::shared_ptr<CC3DTextureRHI> GetTexRHI(long during) { return nullptr; }

    struct AniPair;
};

class CC3DTextureRHI;

class BitmapDrawable: public Drawable
{
public:
    BitmapDrawable(std::shared_ptr<MaterialTexRHI> pTex);
    BitmapDrawable(std::shared_ptr<CC3DTextureRHI> pTex);
    virtual ~BitmapDrawable();
   
    
    virtual Drawable *Clone();
	virtual void getSize(int &w, int &h);
	virtual long getDuring() { return -1; }

	virtual std::shared_ptr<MaterialTexRHI> GetTex(long during) override {
        return m_MaterialTexRHI;
	}

    virtual std::shared_ptr<CC3DTextureRHI> GetTexRHI(long during)
    {
        return m_TexRHI;
    }

public:
    std::shared_ptr<MaterialTexRHI> m_MaterialTexRHI;
    std::shared_ptr<CC3DTextureRHI> m_TexRHI;
};

class AnimationDrawable: public Drawable
{
public:
    AnimationDrawable();
    virtual ~AnimationDrawable();
    
    void start();
    void stop();
    
    void setOffset(long offset);
    
    void appandTex(long during, std::shared_ptr<MaterialTexRHI> pTex);
    void appandTex(long during, std::shared_ptr<CC3DTextureRHI> pTex);
    
	void getSize(int &w, int &h);

	void setLoopMode(en_EffectLoop_Mode effectLoopMode);

	virtual long getDuring();

    virtual Drawable *Clone();

    virtual std::shared_ptr<MaterialTexRHI> GetTex(long during) override;
    virtual std::shared_ptr<CC3DTextureRHI> GetTexRHI(long during) override;

private:
    struct AniPair
    {
		std::shared_ptr<MaterialTexRHI> MatTexRHI;
        std::shared_ptr<CC3DTextureRHI> TexRHI;
        long during;
    };
    std::vector<AniPair> m_aniInfo;
    long m_offset;
    bool m_bRunning;

	en_EffectLoop_Mode m_effectLoopMode;
};
#endif /* Drawable_hpp */
