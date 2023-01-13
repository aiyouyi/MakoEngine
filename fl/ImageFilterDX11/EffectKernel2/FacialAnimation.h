#pragma once

#include "3d/CCSprite3D.h"
#include "3d/CCAnimate3D.h"
#include "2d/CCActionInterval.h"
#include "base/CCAutoreleasePool.h"
#include "Toolbox/FSObject.h"
#include "Toolbox/FacialObject.h"
#include "Toolbox/EffectModel.hpp"
#include "Toolbox/DXUtils/DX11Shader.h"
#include "Toolbox/DXUtils/DX11FBO.h"
#include "Toolbox/RectDraw.h"
#include <xnamath.h>
#include "FaceDetectorInterface.h"
#include "Algorithm/Face3DReconstruct/MultiLinearVideoReconstructor.h"

class SceneInfo
{
public:
	bool loadScene(const char *szFile);
	void destory();
	bool needRender();

	void setBackGround(const char *szZipFile, const char *szImg);

	cocos2d::Action* addAnimation(const string id, const string mode, FacialObject* obj);
public:
	//场景物体
	vector<FSObject *> m_vRootObjects;
	FacialObject *m_pFacialObj;

	//灯光
	vector<BaseLight> m_vLights;
	//2D背景
	vector<Effect2DRect> m_vBackGround;

	NPRInfo m_NPRInfo;

	vec3 m_CameraParam;

	string m_zip_file;
	string m_animate_file;
	string m_id;
};

class DX11IMAGEFILTER_EXPORTS_CLASS FacialAnimation
{
public:
	FacialAnimation();
	virtual ~FacialAnimation();

	bool loadScene(const char *szFile);

	bool loadSceneAsyn(const char *szFile);

	void setBackGround(const char *szZipFile, const char *szImg);

	void SetCameraArea(float x, float y, float w, float h);

	void SetResourcePath(const char *path);

	void updateAvantarParam(float *arr);

	void play(const char *szAnimation, ActionUserCallback pCallBack=NULL, void *pUserInfo=NULL);

	bool renderEffectToTexture(ID3D11ShaderResourceView *pInputTexture, ID3D11Texture2D *pTargetTexture, int width, int height, const ccFDFaceRes *faceRes);

	void destory();

	vector<BaseLight>& getLight() { return m_sceneInfo.m_vLights; }
	NPRInfo& getNPR() { return m_sceneInfo.m_NPRInfo; }
	vector<Effect2DRect>& getEffect2DRectVec() { return m_sceneInfo.m_vBackGround; }
	void resetAnidrawable(const std::vector<std::string> & file_list);
	FacialObject* getFacialObj() { return m_sceneInfo.m_pFacialObj; }
	cocos2d::Action* addAnimation(const string id, const string mode, FacialObject* obj) {
		return m_sceneInfo.addAnimation(id, mode, obj);
	}

protected:
	bool prepare();
	void renderEffect(int width, int height, const ccFDFaceRes *faceRes, int *arrRenderView=NULL);
private:
	SceneInfo m_sceneInfo;

	vec4 m_arrCameraView;
	bool m_bNeedUpdateCameraDraw;

	DX11FBO *m_pFBO;
	RectDraw *m_rectDraw;

	DX11Texture *m_pTargetTexture;

	__int64 m_nLastTime;
	__int64 m_nStartTime;

	//last人脸
	ccFDFaceRes m_lastFaceRes;

	mt3dface::MultiLinearVideoReconstructor *m_Reconstruct3D;

	bool m_FirstReconstruct = true;

	std::string m_resourcePath;
	std::shared_ptr<CC3DTextureRHI> m_InputSRV;

};