#include "GPUParticleEffect.h"
#include "Toolbox/Render/DynamicRHI.h"
#include "Toolbox/Render/ShaderRHI.h"
#include "EffectKernel/BaseRenderParam.h"
#include "Toolbox/RenderState/PiplelineState.h"
#include "Toolbox/Render/MaterialTexRHI.h"
#include "CC3DEngine/Scene/CC3DProject.h"
#include "CC3DEngine/Scene/CC3DCamera.h"

struct GPUParticleEffectP
{
	std::shared_ptr<ShaderRHI> RenderShader;
	std::shared_ptr<ShaderRHI> StreamOutShader;

	std::shared_ptr<CC3DVertexBuffer> InitVB;
	std::shared_ptr<CC3DVertexBuffer> DrawVB;
	std::shared_ptr<CC3DVertexBuffer> StreamOutVB;
	std::shared_ptr<CC3DTextureRHI> RandomTex;
	std::shared_ptr<MaterialTexRHI> ParticleTex;
	
	
	struct VertexParticle
	{
		Vector3 initialPos;
		Vector3 initialVel;
		Vector2 size;
		float age;
		uint32_t type;
	};

	std::chrono::high_resolution_clock::time_point tStart;
	float TotalTime;
	bool FirstRun = true;
	bool Age = 0.0f;
	float EmitInterval = 0.0001f;
	float AliveTime = 1.0f;
	Vector3 EmitPos = {0,0,0};
	Vector3 EmitDir = {0,-1,0};
	Vector3 Accel = { 0.0f, -7.8f, 0.0f };
	float Delta = 1000.f / 60.f * 0.001f;
	CC3DCamera Camera;
	CC3DPerspectiveProject Project;
};

GPUParticleEffect::GPUParticleEffect()
	:Data(std::make_unique<GPUParticleEffectP>())
{

}

GPUParticleEffect::~GPUParticleEffect()
{

}

void GPUParticleEffect::SetShaderResource(const std::string& path, std::shared_ptr<MaterialTexRHI> Tex)
{
	if (GetDynamicRHI()->API == CC3DDynamicRHI::DX11)
	{
		CCVetexAttribute pAttribute[] =
		{
			{VERTEX_ATTRIB_POSITION, FLOAT_C3},
			{VERTEX_ATTRIB_VELOCITY, FLOAT_C3},
			{VERTEX_ATTRIB_SIZE, FLOAT_C2},
			{VERTEX_ATTRIB_AGE, FLOAT_C1},
			{VERTEX_ATTRIB_TYPE, FLOAT_C1}
		};

		std::string ShaderPath = path + "/Shader/Particle/Fire.fx";
		Data->StreamOutShader = GetDynamicRHI()->CreateShaderRHI();
		std::vector< D3D11SOLayout> SOLayouts = {
					{0, "POSITION", 0, 0, 3, 0},
					{0, "VELOCITY", 0, 0, 3, 0},
					{0, "SIZE", 0, 0, 2, 0},
					{0, "AGE", 0, 0, 1, 0},
					{0, "TYPE", 0, 0, 1, 0}
		};
		Data->StreamOutShader->initGeometryWithStreamOutput(ShaderPath, pAttribute, 5, "SO_VS", "SO_GS", SOLayouts);

		Data->RenderShader = GetDynamicRHI()->CreateShaderRHI();
		Data->RenderShader->initGeometryWithFile(ShaderPath, pAttribute, 5, "VS", "GS", "PS");

		int MaxParticles = 100;
		// 创建缓冲区用于产生粒子系统
// 初始粒子拥有类型0和存活时间0
		GPUParticleEffectP::VertexParticle p{};
		p.age = 0.0f;
		p.type = 0;
		Data->InitVB = GetDynamicRHI()->CreateVertexBuffer(&p, sizeof(p), 1, false);
		Data->DrawVB = GetDynamicRHI()->CreateVertexBuffer(&p, sizeof(p), MaxParticles, true);
		Data->StreamOutVB = GetDynamicRHI()->CreateVertexBuffer(&p, sizeof(p), MaxParticles, true);

		Data->tStart = std::chrono::high_resolution_clock::now();

		// 创建随机数据
		std::mt19937 randEngine;
		randEngine.seed(std::random_device()());
		std::uniform_real_distribution<float> randF(-1.0f, 1.0f);
		std::vector<float> randomValues(1024 * 16);

		std::generate(randomValues.begin(), randomValues.end(), [&]() { return randF(randEngine); });

		Data->RandomTex = GetDynamicRHI()->CreateTexture();
		Data->RandomTex->InitTexture1D(CC3DTextureRHI::SFT_A32R32G32B32F, 1024, randomValues.data(), 1024 * 16);
		Data->ParticleTex = Tex;
		// 生成1D随机纹理
		//CD3D11_TEXTURE1D_DESC texDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, 1024, 1, 1);
		//D3D11_SUBRESOURCE_DATA initData{ randomValues.data(), 1024 * 4 };
		//ComPtr<ID3D11Texture1D> pRandomTex;
		//ComPtr<ID3D11ShaderResourceView> pRandomTexSRV;


	}
}

void GPUParticleEffect::Render(BaseRenderParam& Scene)
{

	GetDynamicRHI()->SetDepthStencilState(CC3DPiplelineState::DepthStateDisable);
	GetDynamicRHI()->SetRasterizerState(CC3DPiplelineState::RasterizerStateCullBack);

	Scene.GetDoubleBuffer()->ClearDepthA();
	Scene.GetDoubleBuffer()->BindFBOA();


	Data->TotalTime += Data->Delta;

	glm::mat4 ViewProj = Data->Project.GetProjectMatrix() * Data->Camera.GetViewMatrix();
	glm::vec3 CameraPos = Data->Camera.GetCameraPos();

	auto& ChangesEveryFram = GET_SHADER_STRUCT_MEMBER(GPUParticle_ChangesEveryFrame);

	ChangesEveryFram.SetParameter("g_ViewProj", ViewProj);
	ChangesEveryFram.SetParameter("g_EyePosW", Vector3(CameraPos.x, CameraPos.y,CameraPos.z));
	ChangesEveryFram.SetParameter("g_GameTime", Data->TotalTime);
	ChangesEveryFram.SetParameter("g_EmitDirW", Data->EmitDir.normalize());
	ChangesEveryFram.SetParameter("g_EmitPosW", Data->EmitPos);
	ChangesEveryFram.SetParameter("g_EmitInterval", Data->EmitInterval);
	ChangesEveryFram.SetParameter("g_AliveTime", Data->AliveTime);
	ChangesEveryFram.SetParameter("g_AccelW", Data->Accel);
	ChangesEveryFram.SetParameter("g_TimeStep", Data->Delta);
	ChangesEveryFram.ApplyToAllBuffer();

	auto& TextureParam = GET_SHADER_STRUCT_MEMBER(GPUParticle_Texture);
	GetDynamicRHI()->SetGSSamplerState(CC3DPiplelineState::WarpLinerSampler);
	TextureParam.SetGSTexture("g_TextureRandom", Data->RandomTex);
	TextureParam.ApplyToAllBuffer();

	RenderToVertexBuffer(Data->FirstRun ? Data->InitVB : Data->DrawVB, Data->StreamOutVB, Data->FirstRun);
	Data->FirstRun = 0;

	Data->DrawVB.swap(Data->StreamOutVB);

	Data->RenderShader->UseShader();
	ChangesEveryFram.ApplyToAllBuffer();
	GetDynamicRHI()->SetSamplerState(CC3DPiplelineState::ClampLinerSampler);
	TextureParam.SetTexture("g_TextureInput", Data->ParticleTex->GetTextureRHI());

	Scene.GetDoubleBuffer()->BindFBOA();
	GetDynamicRHI()->DrawAuto(Data->StreamOutVB);

}

void GPUParticleEffect::Update(Vector2 EmitPos, BaseRenderParam& Scene)
{
	Data->Project.SetAspect((float)Scene.GetWidth() / (float)Scene.GetHeight());

	auto& ChangesEveryFram = GET_SHADER_STRUCT_MEMBER(GPUParticle_ChangesEveryFrame);
	if ((EmitPos.x- -1.f) < 0.01f && (EmitPos.y - -1.f) < 0.01f)
	{
		ChangesEveryFram.SetParameter("g_IsCanEmit", 0);
	}
	else
	{
		if (GET_CONSTBUFFER(GPUParticle_ChangesEveryFrame).IsCanEmit == 0 )
		{
			Data->FirstRun = true;
		}

		ChangesEveryFram.SetParameter("g_IsCanEmit", 1);
		glm::mat4 ViewProj = Data->Project.GetProjectMatrix() * Data->Camera.GetViewMatrix();
		glm::mat4 InvViewProj = glm::inverse(ViewProj);

		float x = (((2.0f * EmitPos.x) / Scene.GetWidth()) - 1);
		float y = -(((2.0f * EmitPos.y) / Scene.GetHeight()) - 1);

		glm::vec4 EmitPosW = glm::vec4(x, y, 1.0f, 0.0f) * InvViewProj;

		Data->EmitPos = Vector3(EmitPosW.x*4, EmitPosW.y*4, 0);
	}


}

void GPUParticleEffect::RenderToVertexBuffer(std::shared_ptr< CC3DVertexBuffer> input, std::shared_ptr< CC3DVertexBuffer> output, uint32_t vertexCount /*= 0*/)
{
	Data->StreamOutShader->UseShader();
	GetDynamicRHI()->SOSetTargets(1, output, 0);
	if (vertexCount)
	{
		GetDynamicRHI()->DrawPointList(input);
	}
	else
	{
		GetDynamicRHI()->DrawAuto(input);
	}
	GetDynamicRHI()->SOSetTargets(0, nullptr, 0);

}
