//
//  Drawable.hpp
//  FaceDetector
//
//  Created by cc on 2017/6/28.
//  Copyright © 2017年 cc. All rights reserved.
//

#ifndef Drawable_hpp
#define Drawable_hpp
#include "Toolbox/DXUtils/DX11Texture.h"
#include "Toolbox/Render/DynamicRHI.h"
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
    virtual std::shared_ptr<CC3DTextureRHI> GetTex(long during) = 0;
    virtual ID3D11ShaderResourceView* GetSRV(long during) = 0;
};

class BitmapDrawable: public Drawable
{
public:
    BitmapDrawable();
    BitmapDrawable(std::shared_ptr<CC3DTextureRHI> pTex);
    virtual ~BitmapDrawable();
    
    void setTex(std::shared_ptr<CC3DTextureRHI> pTex);
    
    virtual Drawable *Clone();
	virtual void getSize(int &w, int &h);
	virtual long getDuring() { return -1; }

    virtual std::shared_ptr<CC3DTextureRHI> GetTex(long during) { return m_pTex; }
    virtual ID3D11ShaderResourceView* GetSRV(long during);

public:
    std::shared_ptr<CC3DTextureRHI> m_pTex;
};

class AnimationDrawable: public Drawable
{
public:
    AnimationDrawable();
    virtual ~AnimationDrawable();
    
    void start();
    void stop();
    
    void setOffset(long offset);
    
    void appandTex(long during, std::shared_ptr<CC3DTextureRHI> pTex);
    
	void getSize(int &w, int &h);

	void setLoopMode(en_EffectLoop_Mode effectLoopMode);

	virtual long getDuring();

    virtual Drawable *Clone();

    virtual std::shared_ptr<CC3DTextureRHI> GetTex(long during);
    virtual ID3D11ShaderResourceView* GetSRV(long during);

private:
    struct AniPair
    {
		std::shared_ptr<CC3DTextureRHI> pTex;
        long during;
    };
    std::vector<AniPair> m_aniInfo;
    long m_offset;
    bool m_bRunning;

	en_EffectLoop_Mode m_effectLoopMode;
};
#endif /* Drawable_hpp */
