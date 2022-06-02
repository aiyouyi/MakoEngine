#pragma once
#include "SimpleFaceBackMesh.h"
namespace mt3dface
{
	class MultiSimpleFaceBackMesh :public SimpleFaceBackMesh
	{
	public:
		MultiSimpleFaceBackMesh() = default;
		~MultiSimpleFaceBackMesh() override = default;
		/**
		* @brief 初始化相关multi simple face and back mesh的数据
		*/
		virtual void setMemberInit() override;
		/**
		* @brief 创建simple face mesh
		* @param [in] pImagePoint 106个landmark点
		* @return true表示成功，false表示失败
		*/
		virtual bool UpdateSimpleFaceMesh(const float* pImagePoint) override;
	

	};
}


