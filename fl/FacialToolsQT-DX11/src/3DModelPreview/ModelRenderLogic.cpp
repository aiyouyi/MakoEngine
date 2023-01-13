#include "ModelRenderLogic.h"
#include "CC3DEngine/CC3DInterface.h"
#include "ARModelPreviewWindow.h"
#include "QUIEvent.h"
#include "DataCenter.h"
#include "opencv2/opencv.hpp"
#include "EffectCInterface.h"
#include "EffectKernel/CCEffectInterface.h"
#include "Toolbox/DXUtils/DX11Resource.h"
#include "Toolbox/RectDraw.h"
#include "IFurnaceInterface.h"
#include "SplitScreenDetectInterface.h"
#include "EffectKernel/CCEffectInterface.h"
#include "EffectKernel/ResourceManager.h"
#include "win/win32.h"

ImplementEvent(ModelRenderLogic);

struct ModelRenderLogicP
{
	std::shared_ptr<CC3DImageFilter::CC3DInterface> Interface;
	QPointer<ARModelPreviewWindow> MainWindow;
	std::shared_ptr<CC3DTextureRHI> RenderTargetTex;
	std::shared_ptr<CC3DRenderTargetRHI> RenderTargetRHI;
	std::shared_ptr<CC3DTextureRHI> BGTexture;
	cv::Mat			 Frame;
	cv::VideoCapture Video;
	cc_handle_t		 StickerEffect = nullptr;
	core::vec2i      VideoSize;
	bool UseVideo = false;
	std::string CurrentEffect;
	cc_handle_t   FaceDetectHandle;
	std::shared_ptr<RectDraw>  Draw;

	std::shared_ptr<CC3DRasterizerState> Rasterizer;
	std::shared_ptr<CC3DBlendState> BlendState;
	std::shared_ptr<CC3DDepthStencilState> DepthStencialState;
	std::atomic<float> VideoScale = 1.0;
	core::vec2f VideoTrans;
	bool ShowSegBody = true;
	IFurnaceInterface* SegmentBodyNet = nullptr;
	core::vec2i SegmentVideoSize;
};

ModelRenderLogic::ModelRenderLogic(ARModelPreviewWindow* Main)
{
	Data = std::make_shared<ModelRenderLogicP>();
	Data->MainWindow = Main;
}

ModelRenderLogic::~ModelRenderLogic()
{
	if (Data->StickerEffect)
	{
		ccEffectRelease(Data->StickerEffect);
	}

	if (Data->FaceDetectHandle)
	{
		ccFDUnInit(Data->FaceDetectHandle);
	}

	if (Data->SegmentBodyNet)
	{
		Data->SegmentBodyNet->release();
	}
}

void ModelRenderLogic::Prepare()
{
	Data->Interface = std::make_shared<CC3DImageFilter::CC3DInterface>();
	Data->Interface->sigAddSkeletonLayer.bind(std::bind(&ModelRenderLogic::OnAddSkeletonLayer, this, std::placeholders::_1), this);
	Data->Interface->SetResourcePath("../BuiltInResource");
	Data->CurrentEffect = "../Resource/YiYuFengQing_dt_fc1bd0sx0sg1_AR.zip";
	Data->Interface->LoadZIP("../Resource/YiYuFengQing_dt_fc1bd0sx0sg1_AR.zip");

	SynchronizeConfig();
}

void ModelRenderLogic::DoRender(QSize CanvasSize, std::shared_ptr<CC3DTextureRHI> RenderTargetTex)
{
	if (Data->UseVideo && Data->Draw)
	{
		Data->Video >> Data->Frame;

		if (Data->Frame.empty()) {
			Data->Video.set(CV_CAP_PROP_POS_FRAMES, 0);
			return;
		}

		auto Buffer = Data->Frame.ptr();
		int nChannel = Data->Frame.channels();

		ccFDFaceRes_t faceRes;
		faceRes.numFace = 0;

		ccFDDetect(Data->FaceDetectHandle, Buffer, Data->VideoSize.cx, Data->VideoSize.cy, Data->VideoSize.cx * nChannel, CT_BGR);
		ccFDGetFaceRes(Data->FaceDetectHandle, &faceRes);

		cv::Mat frame_BGRA;
		cvtColor(Data->Frame, frame_BGRA, cv::COLOR_BGR2BGRA);

		if (Data->StickerEffect)
		{
			if (Data->ShowSegBody && Data->SegmentBodyNet)
			{
				int splitType = (int)ccScreenSplitDetect(Data->Frame.data, Data->Frame.cols, Data->Frame.rows, Data->Frame.cols * nChannel, SSD_CT_BGR);
				ccEffectSetSplitScreen(Data->StickerEffect, splitType, CCEffectType::BODY_TRACK2D_EFFECT);

				unsigned char* mask = Data->SegmentBodyNet->detect_segment_screen(Data->Frame.cols, Data->Frame.rows, Data->Frame.ptr(), BGR888, 1, splitType);
				ccEffectSetMask(Data->StickerEffect, mask, Data->SegmentVideoSize.cx, Data->SegmentVideoSize.cy);
			}

			ccEffectSetBGRA(Data->StickerEffect, (unsigned char*)frame_BGRA.ptr());
		}

		Data->BGTexture->InitTexture(CC3DTextureRHI::SFT_A8B8G8R8, CC3DTextureRHI::OT_NONE, Data->VideoSize.cx, Data->VideoSize.cy, frame_BGRA.ptr(), Data->VideoSize.cx * 4);
		Data->Draw->setShaderTextureView(Data->BGTexture);
		if (ccEffectProcessTexture(Data->StickerEffect, RHIResourceCast(Data->BGTexture.get())->GetSRV(), RHIResourceCast(Data->RenderTargetTex.get())->GetNativeTex(),
			Data->BGTexture->GetWidth(), Data->BGTexture->GetHeight(), &faceRes))
		{
			Data->Draw->setShaderTextureView(Data->RenderTargetTex);
		}

		if (!Data->RenderTargetRHI)
		{
			Data->RenderTargetRHI = GetDynamicRHI()->CreateRenderTarget(RenderTargetTex->GetWidth(), RenderTargetTex->GetHeight(), false, RenderTargetTex, CC3DTextureRHI::SFT_A8R8G8B8);
		}
		else if (Data->RenderTargetRHI->GetWidth() != RenderTargetTex->GetWidth() || Data->RenderTargetRHI->GetHeight() != RenderTargetTex->GetHeight())
		{
			Data->RenderTargetRHI = GetDynamicRHI()->CreateRenderTarget(RenderTargetTex->GetWidth(), RenderTargetTex->GetHeight(), false, RenderTargetTex, CC3DTextureRHI::SFT_A8R8G8B8);
		}
		

		Data->RenderTargetRHI->Bind();
		Data->RenderTargetRHI->Clear(0, 0, 0, 1);

		core::vec2f Scale = getScale(core::vec2f(CanvasSize.width(), CanvasSize.height()),core::vec2f(Data->VideoSize.cx, Data->VideoSize.cy));
		//auto blender = DXUtils::CreateBlendState();
		float blendFactor[] = { 0.f,0.f,0.f,0.f };
		//DeviceContextPtr->OMSetBlendState(blender, blendFactor, 0xffffffff);
		GetDynamicRHI()->SetBlendState(Data->BlendState, blendFactor, 0xffffffff);
		GetDynamicRHI()->SetRasterizerState(Data->Rasterizer);
		GetDynamicRHI()->SetDepthStencilState(Data->DepthStencialState);

		Data->Draw->render(Vector2(Data->VideoTrans.cx, Data->VideoTrans.cy), Vector2(Data->VideoScale * Scale.cx, Data->VideoScale *Scale.cy), 0, CanvasSize.width(), CanvasSize.height());

		Data->RenderTargetRHI->UnBind();
	}
	else
	{
		Data->Interface->Render(CanvasSize.width(), CanvasSize.height(), RenderTargetTex);
	}
	
}

void ModelRenderLogic::ShutDown()
{
	Data->Interface = {};
}

void ModelRenderLogic::SetVideoScale(float Scale)
{
	if (!Data->UseVideo)
	{
		return;
	}
	Data->VideoScale = Scale;
}

void ModelRenderLogic::SetVideoTranslate(core::vec2f Tans)
{
	//Data->VideoTrans = Tans;
}


void ModelRenderLogic::SynchronizeConfig()
{
	{
		QSynchronizeConfigEvent* InfoE = new QSynchronizeConfigEvent();
		DataCenter::getInstance().RecordInfo = Data->Interface->GetModeConfig();
		qApp->postEvent(Data->MainWindow, InfoE);
	}

	{
		DataCenter::getInstance().FurData = Data->Interface->GetFurData();
		QFurDataEvent* InfoE = new QFurDataEvent();
		qApp->postEvent(Data->MainWindow, InfoE);
	}

	{
		QPostProcessEvent* InfoE = new QPostProcessEvent();
		qApp->postEvent(Data->MainWindow, InfoE);
	}
}

void ModelRenderLogic::OnAddSkeletonLayer(const std::map<std::string, int>& bone)
{
	AddSkeletonLayerEvent* InfoE = new AddSkeletonLayerEvent();
	InfoE->bone_name_map = bone;
	qApp->postEvent(Data->MainWindow, InfoE);
}

void ModelRenderLogic::InitEffect()
{
	if (!Data->StickerEffect)
	{
		Data->Draw = std::make_shared<RectDraw>();
		Data->Draw->init(1, 1, "");

		Data->Rasterizer = GetDynamicRHI()->CreateRasterizerState(CC3DRasterizerState::CT_NONE);
		Data->BlendState = GetDynamicRHI()->CreateBlendState(true, false, true, false);
		Data->DepthStencialState = GetDynamicRHI()->CreateDefaultStencilState(false, true);

		Data->StickerEffect = ccEffectCreate();
		const char* Rpath = "../BuiltInResource";
		ccEffectSetPath(Data->StickerEffect, Rpath);
		ccEffectSetEffectZipAsyn(Data->StickerEffect, Data->CurrentEffect.c_str(), "test.xml", nullptr, nullptr);

		Data->FaceDetectHandle = ccFDInit("../BuiltInResource/faceModel/");
		//	ccFDSetFilterType(1);
		//	ccFD(0.8f);
		ccFDSetLandmarkFilter(Data->FaceDetectHandle, 0.5);
		//face expression control
		ccFDActivateEyeExpression(Data->FaceDetectHandle);
		ccActivateStageTwo(Data->FaceDetectHandle, "../BuiltInResource/faceModel/");
		ccActivateStageTwoEye(Data->FaceDetectHandle, "../BuiltInResource/faceModel/");
	}

	Data->VideoSize.cx = Data->Frame.cols;
	Data->VideoSize.cy = Data->Frame.rows;
	Data->RenderTargetTex = GetDynamicRHI()->CreateTexture();
	Data->BGTexture = GetDynamicRHI()->CreateTexture();
	Data->RenderTargetTex->InitTexture(CC3DTextureRHI::SFT_A8R8G8B8, CC3DTextureRHI::OT_RENDER_TARGET, Data->VideoSize.cx, Data->VideoSize.cy);

	if (Data->ShowSegBody)
	{
		if (Data->SegmentBodyNet)
		{
			Data->SegmentBodyNet->release();
		}
		Data->SegmentBodyNet = CreateFurnaceInterface();

		if (-1 == Data->SegmentBodyNet->load_segment_model("../BuiltInResource/BodyMask/mnnmodel_singo_x128.mnn", "../BuiltInResource/BodyMask/mnnmodel_singo.mnn", Data->VideoSize.cx, Data->VideoSize.cy, &Data->SegmentVideoSize.cy, &Data->SegmentVideoSize.cx))
		{
			//cout << "load_segment_model error!" << endl;
			return;
		}
		Data->SegmentBodyNet->set_frame_interval(60);
	}

}

void ModelRenderLogic::LoadModelResource(const std::string& FileName)
{
	Data->CurrentEffect = FileName;
	Data->Interface = std::make_shared<CC3DImageFilter::CC3DInterface>();
	Data->Interface->sigAddSkeletonLayer.bind(std::bind(&ModelRenderLogic::OnAddSkeletonLayer, this, std::placeholders::_1), this);
	Data->Interface->SetResourcePath("../BuiltInResource");

	Data->Interface->LoadZIP(FileName);

	SynchronizeConfig();

	if (Data->StickerEffect)
	{
		ccEffectSetEffectZipSync(Data->StickerEffect, Data->CurrentEffect.c_str(), "test.xml", nullptr, nullptr);
	}
}

template<>
void ModelRenderLogic::OnEvent<EventType::OpenGLBTask>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< OpenGLBTask> OpenModelTask = std::static_pointer_cast<OpenGLBTask>(E);
	std::string Utf8FileName = OpenModelTask->fileName.toUtf8().data();

	Data->Interface = std::make_shared<CC3DImageFilter::CC3DInterface>();
	Data->Interface->sigAddSkeletonLayer.bind(std::bind(&ModelRenderLogic::OnAddSkeletonLayer, this, std::placeholders::_1), this);
	Data->Interface->SetResourcePath("../BuiltInResource");
	Data->Interface->SetHDRPath("../BuiltInResource/HDR/venice_sunset_1k.hdr");

	Data->Interface->LoadModel(Utf8FileName);
}

template<> 
void ModelRenderLogic::OnEvent<EventType::ChangeAREffect>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeAREffect> ChangeAREffectTask = std::static_pointer_cast<ChangeAREffect>(E);
	std::string Utf8FileName = ChangeAREffectTask->fileName.toUtf8().data();

	LoadModelResource(Utf8FileName);

}

template<> 
void ModelRenderLogic::OnEvent<EventType::ReSize>(std::shared_ptr<TaskEvent> E)
{

}

template<>
void ModelRenderLogic::OnEvent<EventType::MouseEvent>(std::shared_ptr<TaskEvent> E)
{
	if (Data->UseVideo)
	{
		return;
	}
	std::shared_ptr< QTMouseEvent> Event = std::static_pointer_cast<QTMouseEvent>(E);
	if (Event)
	{
		Data->Interface->SetScale(Event->Scale);
		Data->Interface->SetTranslate(Event->Translate.x, Event->Translate.y);
		Data->Interface->SetRotate(Event->Rotate.x,Event->Rotate.y);
	}
}

template<> 
void ModelRenderLogic::OnEvent<EventType::OpenAnimateJson>(std::shared_ptr<TaskEvent> E)
{

}

template<> 
void ModelRenderLogic::OnEvent<EventType::RotateHDR>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr<RotateHDREvent> RotateEvent = std::static_pointer_cast<RotateHDREvent>(E);
	if (RotateEvent)
	{
		Data->Interface->SetRotateIBL(RotateEvent->hdrRotateX,RotateEvent->hdrRotateY);
		Data->Interface->SetHDRScale(RotateEvent->hdrScale);
		Data->Interface->SetHDRContrast(RotateEvent->hdrContrast);
	}
}

template<> 
void ModelRenderLogic::OnEvent<EventType::LightInfo>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr<LightInfoEvent> LightEvent = std::static_pointer_cast<LightInfoEvent>(E);

	const auto& RecordInfo = DataCenter::getInstance().RecordInfo;

	Data->Interface->SetMainLightInfo(RecordInfo.LightDir, RecordInfo.LightColor, RecordInfo.LightStrength, RecordInfo.AmbientStrength);
	Data->Interface->SetGamma(RecordInfo.Gamma);
	Data->Interface->SetEnableDiffuseSRGB(RecordInfo.bEnableSRGB);
	Data->Interface->SetAOOffset(RecordInfo.AOOffset);
	Data->Interface->SetEnableShadow(RecordInfo.bEnableShadow);
}

template<> 
void ModelRenderLogic::OnEvent<EventType::ChangeFurData>(std::shared_ptr<TaskEvent> E)
{
	Data->Interface->SetFurData(DataCenter::getInstance().FurData);
}

template<>
void ModelRenderLogic::OnEvent<EventType::DynamicBone>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr<DynamicBoneEvent> dbEvent = std::static_pointer_cast<DynamicBoneEvent>(E);
	switch (dbEvent->dbType)
	{
	case DynamicBoneOperateType::ADD_DB:
		Data->Interface->AddDynamicBone(dbEvent->dbName.toUtf8().data());
		break;
	case DynamicBoneOperateType::DELETE_DB:
		Data->Interface->DeleteDynamicBone(dbEvent->dbName.toUtf8().data());
		break;
	case DynamicBoneOperateType::CLEAR_DB:
		Data->Interface->ResetDynamicBone();
		break;
	case DynamicBoneOperateType::Update_DB:
	{
		std::string Name = dbEvent->dbName.toStdString();
		auto& DyBoneArray = DataCenter::getInstance().RecordInfo.DyBone_array;
		auto it = std::find_if(DyBoneArray.begin(), DyBoneArray.end(), [&Name](const CC3DImageFilter::dynamicBoneParameter& item)
			{return item.bone_name == Name; });
		if (it != DyBoneArray.end())
		{
			Data->Interface->UpdateDynamicParam(*it);
		}
	}
	default:
		break;
	}
}

template<> 
void ModelRenderLogic::OnEvent<EventType::ChangeTonemapping>(std::shared_ptr<TaskEvent> E)
{
	Data->Interface->SetTonemappingType(DataCenter::getInstance().RecordInfo.ToneMapping);
}

template<> 
void ModelRenderLogic::OnEvent<EventType::Bloom>(std::shared_ptr<TaskEvent> E)
{
	Data->Interface->SetBloomInfo(DataCenter::getInstance().RecordInfo.Bloom);
}

template<> 
void ModelRenderLogic::OnEvent<EventType::ChangePreviewVideo>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr<VideoPreviewEvent> Event = std::static_pointer_cast<VideoPreviewEvent>(E);
	if (Event)
	{
		Data->UseVideo = Event->UseVideo;
		if (Data->UseVideo)
		{
			auto utf8 = Event->FileName.toUtf8();
			std::string fileName;
			fileName.append(utf8.data(), utf8.size());

			Data->Video.open(fileName.c_str());
			if (Data->Video.isOpened() == false)
			{
				std::cerr << "Failed opening camera" << std::endl;
				return;
			}
			Data->Video >> Data->Frame;
			InitEffect();
		}

	}
}

template<> 
void ModelRenderLogic::OnEvent<EventType::ChangeHDRFile>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ChangeHDRFileEvent> Event = std::static_pointer_cast<ChangeHDRFileEvent>(E);
	if (Event)
	{
		Data->Interface->SetHDRPath(Event->FileName.toUtf8().data());
	}
}

template<>
void ModelRenderLogic::OnEvent<EventType::GetCurrentEffectAttr>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< GetCurrentEffectAttrEvent> Event = std::static_pointer_cast<GetCurrentEffectAttrEvent>(E);
	if (Event&& Data->StickerEffect)
	{
		for (auto EffectPart : ((CCEffectInterface*)Data->StickerEffect)->m_AllEffectPart)
		{
			if (EffectPart->m_EffectPart== FACE_EYESTRICK)
			{
				EffectAttrData Attr;
				Attr.ShowName = QString::fromStdString(EffectPart->m_showname);
				Attr.Alpha = EffectPart->m_alpha;
				Attr.pEffect = (void*)EffectPart;
				Event->Effects.push_back(Attr);
			}
		}
		Event->FinishCallBack();
	}
}


template<>
void ModelRenderLogic::OnEvent<EventType::SetEffectAttr>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< SetEffectAttrEvent> Event = std::static_pointer_cast<SetEffectAttrEvent>(E);
	if (Event)
	{
		for (CEffectPart* EffectPart : ((CCEffectInterface*)Data->StickerEffect)->m_AllEffectPart)
		{
			if (Event->AttrData.pEffect==EffectPart)
			{
				EffectPart->SetAlpha(Event->AttrData.Alpha);
			}
		}
	}
}

template<>
void ModelRenderLogic::OnEvent<EventType::OpenXMLFile>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< OepnXMLFileEvent> Event = std::static_pointer_cast<OepnXMLFileEvent>(E);
	if (Event && Data->StickerEffect)
	{
		CCEffectInterface* EffectInterface = (CCEffectInterface*)Data->StickerEffect;

		std::filesystem::path Path(Event->FileName.toUtf8().data());

		std::string ContainPath = Path.parent_path().string() + "/";
		std::string TempZip = win32::temp_path().string() + "Temp.zip";
		EffectInterface->ZipAllResourceOnly(TempZip, ContainPath);

		LoadModelResource(TempZip);

		::DeleteFileW(core::u8_ucs2(TempZip).c_str());
	}
}

template<> 
void ModelRenderLogic::OnEvent<EventType::ExportXMLConfig>(std::shared_ptr<TaskEvent> E)
{
	std::shared_ptr< ExportXMLConfigEvent> Event = std::static_pointer_cast<ExportXMLConfigEvent>(E);
	CCEffectInterface* EffectInterface = (CCEffectInterface*)Data->StickerEffect;
	if (Event && EffectInterface)
	{
		XMLNode RootNode;
		EffectInterface->UpdateTestXmlWithNewAttr(RootNode);
		XMLNode nodeEffect;
		int i = -1;
		while (!(nodeEffect = RootNode.getChildNode("typeeffect", ++i)).isEmpty())
		{
			std::string typestr(nodeEffect.getAttribute("type"));
			if (typestr == "FacePBRModel")
			{
				nodeEffect.deleteNodeContent();
			}			
		}
		const std::string FileName = Event->FileName.toUtf8().data();
		DataCenter::getInstance().Config_stream.SaveModelXML(RootNode, DataCenter::getInstance().RecordInfo, DataCenter::getInstance().FurData);
		RootNode.writeToFile(FileName.c_str());
	}
}