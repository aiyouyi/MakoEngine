#ifndef MESHRENDERCMD

#include "../3d/CCMesh.h"
#include "../3d/BaseLight.h"
#include "../math/Mat4.h"
#include "CCVertexIndexBuffer.h"
#include "CCVertexIndexData.h"
#include "Toolbox/DXUtils/DX11Context.h"
#include <xnamath.h>
#include <vector>
using namespace std;

NS_CC_BEGIN

#define SKINNING_JOINT_COUNT 128
class CC_DLL MeshRenderCmd: public Ref
{
public:
	static MeshRenderCmd *create(Mesh *pMesh, Mat4 matTransform, bool bTransparent, bool bLighting, bool bNPR = false)
	{
		MeshRenderCmd *cmd = new MeshRenderCmd();
		cmd->init(pMesh, matTransform, bTransparent, bLighting,bNPR);
		cmd->autorelease();
		return cmd;
	}

	const Mat4 getInitWTransform();

	bool init(Mesh *pMesh, Mat4 matTrans, bool bTransparent, bool bLighting,bool bNPR=false);

	bool isTransparent();

	void updateTransform(const Mat4 &matTransform);

	//render sort
	float getZOrder();

	bool getNPR() const { return m_bNPR; }

	virtual void render(const Mat4 &matView, const Mat4 &matProj, vector<BaseLight> &vLights,NPRInfo *npr = NULL);
protected:
	MeshRenderCmd();
	~MeshRenderCmd();

	void prepare();
	void destory();

	struct BaseLightInfo
	{
		Vec4 vAmbient;
		Vec4 vDiffuse;
		Vec4 vSpecular;
		Vec4 vOrient;
	};

	struct BaseConstantBuffer
	{
		Mat4 matWorld;
		Mat4 matWorldR;
		Mat4 matVP;
	};
	struct SkinConstantBuffer
	{
		Mat4 matWorld;
		Mat4 matWorldR;
		Mat4 matVP;
		Vec4 matPalette[SKINNING_JOINT_COUNT * 3];
	};
	struct BaseLightingCB
	{
		Mat4 matWorld;
		Mat4 matWorldR;
		Mat4 matVP;
		BaseLightInfo arrLight[4];
	};
	struct SkinLightingCB
	{
		Mat4 matWorld;
		Mat4 matWorldR;
		Mat4 matVP;
		Vec4 matPalette[SKINNING_JOINT_COUNT * 3];
		BaseLightInfo arrLight[4];
	};
	struct SkinLightingNPRCB
	{
		Mat4 matWorld;
		Mat4 matWorldR;
		Mat4 matVP;
		Vec4 matPalette[SKINNING_JOINT_COUNT * 3];
		BaseLightInfo arrLight[4];
		Vec4 u_NPRInfo;
		Vec4 u_NPRLineColor;
	};

private:
	bool m_bNeedUpdate;
	bool m_bTransparent;
	bool m_bLighting;
	bool m_bNPR;
	Mesh *m_pMesh;
	Mat4 m_matTrans;
	Mat4 m_matTransInit;
	//渲染模型信息
	DX11Shader *m_pShader;
	//ID3D11Buffer*       m_pConstantBuffer;  //转换矩阵

	//渲染骨骼动画混合
	//ID3D11Buffer *m_pSkinConstantBuffer;  //转换矩阵

	ID3D11Buffer *m_pConstantBuffer;

	//采样状态
	ID3D11SamplerState* m_pSamplerLinear;

	//混合以及buffer操作
	ID3D11BlendState *m_pBlendState;
	//ID3D11BlendState *m_pBSEnable;
	//ID3D11BlendState *m_pBSDisable;

	//深度操作
	ID3D11DepthStencilState *m_pDepthState;
	//ID3D11DepthStencilState *m_pDepthStateEnable;
	ID3D11DepthStencilState *m_pDepthStateRestore;
}; 

struct MeshRenderCmdCmp
{
	bool operator()(MeshRenderCmd *modelL, MeshRenderCmd *modelR)
	{
		float zL = modelL->getZOrder();

		float zR = modelR->getZOrder();

		return zL < zR;
	}
};
NS_CC_END
#endif