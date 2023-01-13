#pragma once
#include <QtCore>
#include "Task/basetask.h"

class ARModelPreviewWindow;
class CC3DTextureRHI;
class CC3DInterface;

struct ModelRenderLogicP;
struct TaskEvent;

class ModelRenderLogic
{
public:
	ModelRenderLogic(ARModelPreviewWindow* Main);
	~ModelRenderLogic();

	void Prepare() ;
	void DoRender(QSize CanvasSize, std::shared_ptr<CC3DTextureRHI> RenderTargetTex);
	void ShutDown() ;
	void SetVideoScale(float Scale);
	void SetVideoTranslate(core::vec2f Tans);

	DeclarationOnEvent(ModelRenderLogic);
	template<> void OnEvent<EventType::OpenGLBTask>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeAREffect>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ReSize>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::MouseEvent>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::OpenAnimateJson>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::RotateHDR>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::LightInfo>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeFurData>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::DynamicBone>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeTonemapping>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::Bloom>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangePreviewVideo>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ChangeHDRFile>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::GetCurrentEffectAttr>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::SetEffectAttr>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::OpenXMLFile>(std::shared_ptr<TaskEvent> E);
	template<> void OnEvent<EventType::ExportXMLConfig>(std::shared_ptr<TaskEvent> E);

	static core::vec2f getScale(core::vec2f PreivewSize, core::vec2f VideoSize) {
		core::vec2f Scale{ 1,1 };
		int w_mh = PreivewSize.cx * VideoSize.cy;
		int h_mw = PreivewSize.cy * VideoSize.cx;

		if (0 == w_mh || 0 == h_mw)
			return Scale;
		if (w_mh > h_mw) {
			Scale.cx = float(h_mw) / float(w_mh);
		}
		else {
			Scale.cy = float(w_mh) / float(h_mw);
		}
		return Scale;
	}
private:
	void SynchronizeConfig();
	void OnAddSkeletonLayer(const std::map<std::string, int>& bone);
	void InitEffect();
	void LoadModelResource(const std::string& FileName);
private:
	std::shared_ptr< ModelRenderLogicP> Data;

};