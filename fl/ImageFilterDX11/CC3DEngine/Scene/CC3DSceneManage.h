#pragma once
#ifndef _H_CC3D_SCENEMANAGE_H_
#define _H_CC3D_SCENEMANAGE_H_
#include "CC3DCamera.h"
#include "CC3DLight.h"
#include "CC3DProject.h"
#include "CC3DModelControl.h"
#include "Model/CC3DModel.h"
#include "Animate/CC3DAnimateManage.h"
#include "Skeleton/CC3DSkeleton.h"
#include "Skeleton/CC3DSkeletonManage.h"
#include "Toolbox/event.h"

namespace CC3DImageFilter
{
	class CC3DSceneManage
	{
	public:
		CC3DSceneManage(void);
		virtual ~CC3DSceneManage(void);
		void Release();
		void LoadMainModel(char* pFileName, const uint8* pDataBuffer = NULL, int nLen = 0);
		void AddModel(char* pFileName, char* AttatchBone = NULL, const uint8* pDataBuffer = NULL, int nLen = 0);
		void AddLight(CC3DLight* pLight);
		void LoadAnimate();
		void LoadSkeleton(char* AttatchBone);
		void DeleteLastModel();
		void DeleteModel(int Index);
		void DelayLoadModelResource(CC3DImageFilter::EffectConfig* EffectConfig);

		void play(float fSecond);
		void playOnce(float fSecond);

		void ReadSkeletonAnimate(const char* pAnimateFile);
		void ReadSkeletonAnimateData(const char* pAnimateData);
		void AddSkeletonAnnimateData(const char* pAnimateData, const std::string& animName);
		void AddSkeletonAnnimateDataForBlend(const char* pAnimateData, const std::string& animName, const std::string& boneName, float blendWeight = 1.0f);
		void ReSortAnimateTimeline(float blendTime);

		void LoadAnimationJson(const std::string& fileName);
	public:

		//相机设置
		CC3DCamera m_Camera;
		//投影矩阵设置
		CC3DPerspectiveProject m_Project;
		//model矩阵，这里暂时只用一个model矩阵控制所有model
		CC3DModelControl m_ModelControl;
		//灯光设置，可以多个灯光，多种类型
		std::vector<CC3DLight*> m_Light;


		//gltf加载器
		tinygltf::TinyGLTF gltf_ctx;
		//gltf model
		std::vector<tinygltf::Model*>m_gltfModel;
		//CC model
		std::vector<CC3DModel*>m_Model;

		std::vector<CC3DAnimateManage*>m_Animate;

		std::vector <CC3DSkeleton*>m_Skeleton;

		CC3DSkeletonManage SkeletonManage;

		bool HasLoadModel = false;

		int m_nSelectCamera = 0;

	public:
		core::event<void(const std::map<std::string, int>&) > sigAddSkeletonLayer;

	private:

		void SetDefaultLight();
		void SetDefaultCamera();
		void SetDefaultProject();
		void setDefaultLookAt();

		void SetDefaultModelControl();

	};

}



#endif // _H_CC3D_SCENEMANAGE_H_
