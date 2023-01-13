#pragma once
#include "inc.h"
#include "QUIEvent.h"
#include<vector>


class ARModelPreviewWindow;

namespace CC3DImageFilter
{
	struct dynamicBoneParameter;
}

class DynamicAttrUI
{
public:
	DynamicAttrUI(QWidget* Parent, int Index, QObject* MessageReceiver);
	~DynamicAttrUI();
	class QWidget* ParentUI = nullptr;
	class QLabel* AttrLabel = nullptr;
	class QSlider* AttrSlider = nullptr;
	class QLineEdit* AttrLineEdit = nullptr;
	void* pEffect = nullptr;
};

class BaseToolTab : public QObject
{
	Q_OBJECT
public:
	BaseToolTab(QWidget* parent, ARModelPreviewWindow* Main);
	~BaseToolTab();

	void SetUIValue(QSynchronizeConfigEvent* E);
	void SetUIValue(AddSkeletonLayerEvent* E);

private slots:
	void OnOpenGlbModel();
	void OnOpenARZIP();
	void OnSliderHDRXChanged(int value);
	void OnSliderHDRYChanged(int value);
	void OnSliderHDRScaleChanged(int value);
	void OnSliderHDRContrastChanged(int value);

	void OnSliderLightX(int value);
	void OnSliderLightY(int value);
	void OnSliderLightZ(int value);
	void OnSliderLightR(int value);
	void OnSliderLightG(int value);
	void OnSliderLightB(int value);

	void OnSliderLightGamma(int value);
	void OnSliderLightIntensity(int value);
	void OnSliderLightAmbient(int value);

	void OnSliderAOOffsetChanged(int value);
	void OnCBShadowStateChanged(int value);
	void OnExportSetting();

	void OnAddDynamicBone();
	void OnDeleteDynamicBone();
	void OnClearDynamicBone();
	void OnSliderDampingChanged(int value);
	void OnSliderElasticityChanged(int value);
	void OnSliderStiffnessChanged(int value);
	void OnSliderInertChanged(int value);
	void OnDynamicBoneIndexChanged(int index);

	void OnToneMappingIndexChanged(int value);
	void OnSliderToneMappingContrastChanged(int value);
	void OnSliderToneMappingSaturationChanged(int value);

	void OnCBUseVideoChange(int value);
	void OnChangeVideoFile();
	void OnChangeHDRFile();

	void UpdateTabWidget(int Index);
	void OnAttrValueChanged(int value);
public:
	void OpenVideoFile(const QString& videoFile);
	void OpenARZipFile(const QString& ARZipFile);
	void OpenHDRFile(const QString& HDRFile);
	void OpenXMLFile(const QString& XML);
private:
	void MakeRotateHDREvent();
	void MakeLightInfoEvent();
	void UpdateDynamicBoneConfig(const CC3DImageFilter::dynamicBoneParameter& db_param);
private:
	QPointer<QWidget> m_ParentWidget;
	QPointer<ARModelPreviewWindow> m_MainWindow;
	std::wstring m_FBX2glTF;
	QString m_CurrentVideoFileName;
	std::vector< DynamicAttrUI*> AttrUis;
};
