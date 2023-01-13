#include "BasicToolTab.h"

#include <QFileDialog>
#include "ARModelPreviewWindow.h"
#include "Task/basetask.h"
#include "DataCenter.h"
#include "BasicPreview.h"
#include <filesystem>
#include <QtGui>

BaseToolTab::BaseToolTab(QWidget* parent, ARModelPreviewWindow* Main)
	:m_ParentWidget(parent)
	,m_MainWindow(Main)
{
	QObject::connect(m_MainWindow->ModelUI.BtnOpenGLB, SIGNAL(clicked()), this, SLOT(OnOpenGlbModel()));
	QObject::connect(m_MainWindow->ModelUI.BtnOpenZIP, SIGNAL(clicked()), this, SLOT(OnOpenARZIP()));
	QObject::connect(m_MainWindow->ModelUI.BtnExportXML, SIGNAL(clicked()), this, SLOT(OnExportSetting()));

	QObject::connect(m_MainWindow->ModelUI.SliderHDRX, SIGNAL(valueChanged(int)), this, SLOT(OnSliderHDRXChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderHDRY, SIGNAL(valueChanged(int)), this, SLOT(OnSliderHDRYChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderHDRScale, SIGNAL(valueChanged(int)), this, SLOT(OnSliderHDRScaleChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderHDRContrast, SIGNAL(valueChanged(int)), this, SLOT(OnSliderHDRContrastChanged(int)));

	QObject::connect(m_MainWindow->ModelUI.SliderLightX, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightX(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderLightY, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightY(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderLightZ, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightZ(int)));

	QObject::connect(m_MainWindow->ModelUI.SliderLightR, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightR(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderLightG, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightG(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderLightB, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightB(int)));

	QObject::connect(m_MainWindow->ModelUI.SliderGamma, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightGamma(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderLightIntensity, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLightIntensity(int)));

	QObject::connect(m_MainWindow->ModelUI.SliderAOOffset, SIGNAL(valueChanged(int)), this, SLOT(OnSliderAOOffsetChanged(int)));
	
	QObject::connect(m_MainWindow->ModelUI.chEnableShadow, SIGNAL(stateChanged(int)), this, SLOT(OnCBShadowStateChanged(int)));

	QObject::connect(m_MainWindow->ModelUI.BtnAddDynamicBone, SIGNAL(clicked()), this, SLOT(OnAddDynamicBone()));
	QObject::connect(m_MainWindow->ModelUI.BtnDeleteDynamicBone, SIGNAL(clicked()), this, SLOT(OnDeleteDynamicBone()));
	QObject::connect(m_MainWindow->ModelUI.BtnClearDyanmicBone, SIGNAL(clicked()), this, SLOT(OnClearDynamicBone()));
	QObject::connect(m_MainWindow->ModelUI.SliderDamping, SIGNAL(valueChanged(int)), this, SLOT(OnSliderDampingChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderElasticity, SIGNAL(valueChanged(int)), this, SLOT(OnSliderElasticityChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderStiffness, SIGNAL(valueChanged(int)), this, SLOT(OnSliderStiffnessChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderInert, SIGNAL(valueChanged(int)), this, SLOT(OnSliderInertChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.cbDynamicBone, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDynamicBoneIndexChanged(int)));

	QObject::connect(m_MainWindow->ModelUI.cbPBRTonemapping, SIGNAL(currentIndexChanged(int)), this, SLOT(OnToneMappingIndexChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderToneMappingContrast, SIGNAL(valueChanged(int)), this, SLOT(OnSliderToneMappingContrastChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderToneMappingSaturation, SIGNAL(valueChanged(int)), this, SLOT(OnSliderToneMappingSaturationChanged(int)));

	QObject::connect(m_MainWindow->ModelUI.CBUSEVideo, SIGNAL(stateChanged(int)), this, SLOT(OnCBUseVideoChange(int)));
	QObject::connect(m_MainWindow->ModelUI.BtnChangeVideo, SIGNAL(clicked()), this, SLOT(OnChangeVideoFile()));
	QObject::connect(m_MainWindow->ModelUI.BtnChangeHDR, SIGNAL(clicked()), this, SLOT(OnChangeHDRFile()));
	QObject::connect(m_MainWindow->ModelUI.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(UpdateTabWidget(int)));

	m_MainWindow->ModelUI.cbPBRTonemapping->addItem("ACESTonemapping");
	m_MainWindow->ModelUI.cbPBRTonemapping->addItem("AMDTonemapping");
	m_FBX2glTF = L"../Release/FBX2GlbTools/FBX2glTF.exe";

	m_CurrentVideoFileName = "../Resource/2202241433(kiki).mp4";

	m_MainWindow->ModelUI.CBUSEVideo->setChecked(true);
	//std::shared_ptr< VideoPreviewEvent> ViewPreview = std::make_shared<VideoPreviewEvent>();
	//ViewPreview->FileName = m_CurrentVideoFileName;
	//ViewPreview->UseVideo = true;
	//m_MainWindow->PostEvent(ViewPreview);
}

BaseToolTab::~BaseToolTab()
{

}

void BaseToolTab::SetUIValue(QSynchronizeConfigEvent* E)
{
	const auto& LightDir = DataCenter::getInstance().RecordInfo.LightDir;
	m_MainWindow->ModelUI.SliderLightX->setSliderPosition((LightDir.x + 1) / 0.02);
	m_MainWindow->ModelUI.SliderLightY->setSliderPosition((LightDir.y + 1) / 0.02);
	m_MainWindow->ModelUI.SliderLightZ->setSliderPosition((LightDir.z + 1) / 0.02);

	const auto& LightColor = DataCenter::getInstance().RecordInfo.LightColor;
	m_MainWindow->ModelUI.SliderLightR->setSliderPosition((LightColor.x + 1) / 0.02);
	m_MainWindow->ModelUI.SliderLightG->setSliderPosition((LightColor.y + 1) / 0.02);
	m_MainWindow->ModelUI.SliderLightB->setSliderPosition((LightColor.z + 1) / 0.02);

	m_MainWindow->ModelUI.SliderHDRX->setSliderPosition(DataCenter::getInstance().RecordInfo.hdrRotateX / 3.6f);
	m_MainWindow->ModelUI.SliderHDRY->setSliderPosition(DataCenter::getInstance().RecordInfo.hdrRotateY / 3.6f);
	m_MainWindow->ModelUI.SliderHDRScale->setSliderPosition(DataCenter::getInstance().RecordInfo.HDRScale / 0.02);
	m_MainWindow->ModelUI.SliderHDRContrast->setSliderPosition(DataCenter::getInstance().RecordInfo.HDRContrast / 0.01);

	float gamma = DataCenter::getInstance().RecordInfo.Gamma;
	m_MainWindow->ModelUI.SliderGamma->setSliderPosition(gamma / 0.02);

	float LightStrength = DataCenter::getInstance().RecordInfo.LightStrength;
	m_MainWindow->ModelUI.SliderLightIntensity->setSliderPosition(LightStrength / 0.1);

	float AOOffset = DataCenter::getInstance().RecordInfo.AOOffset;
	m_MainWindow->ModelUI.SliderAOOffset->setSliderPosition(AOOffset / 0.01);

	m_MainWindow->ModelUI.chEnableShadow->setChecked(DataCenter::getInstance().RecordInfo.bEnableShadow);

	m_MainWindow->ModelUI.cbDynamicBone->clear();
	for (auto item : DataCenter::getInstance().RecordInfo.DyBone_array)
	{
		m_MainWindow->ModelUI.cbDynamicBone->addItem(QString::fromStdString(item.bone_name));
	}
	if (m_MainWindow->ModelUI.cbDynamicBone->count() > 0)
	{
		m_MainWindow->ModelUI.cbDynamicBone->setCurrentIndex(0);
	}
	
	m_MainWindow->ModelUI.cbPBRTonemapping->setCurrentIndex(DataCenter::getInstance().RecordInfo.ToneMapping.ToneMappingType);
	m_MainWindow->ModelUI.SliderToneMappingContrast->setSliderPosition(DataCenter::getInstance().RecordInfo.ToneMapping.Contrast / 0.03);
	m_MainWindow->ModelUI.SliderToneMappingSaturation->setSliderPosition(DataCenter::getInstance().RecordInfo.ToneMapping.Saturation / 0.03);

	UpdateTabWidget(3);
}


void BaseToolTab::SetUIValue(AddSkeletonLayerEvent* E)
{
	auto itr = E->bone_name_map.begin();
	int index = 0;
	for (; itr != E->bone_name_map.end(); itr++)
	{
		QString itemName = QString::fromUtf8(itr->first.c_str());
		m_MainWindow->ModelUI.cbSkeleton->insertItem(index, itemName);
	}
}

void BaseToolTab::OnOpenGlbModel()
{
	QFileDialog dialog(m_ParentWidget);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setNameFilter("*.glb *.gltf *.fbx");
	if (dialog.exec())
	{
		QStringList Files = dialog.selectedFiles();
		if (m_MainWindow && Files.size() > 0)
		{
			m_MainWindow->ModelUI.cbSkeleton->clear();
			QString FileName = Files[0];
			if (FileName.endsWith(".glb"))
			{
				std::shared_ptr< OpenGLBTask> Task = std::make_shared<OpenGLBTask>();
				Task->fileName = FileName;
				m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(Task));
				m_MainWindow->ResetModelPostion();
			}
			else if (FileName.endsWith(".fbx"))
			{
				std::wstring TempPath = FileName.toStdWString();
				std::wstring OutputPath = TempPath.substr(0, TempPath.find_last_of(L".")) + L".glb";

				std::wstring Space = L" ";
				std::wstring Command = Space + L"--binary" + Space + L"--input" +
					Space + TempPath + Space + L"--output" + Space + OutputPath;

				wchar_t szPath[1024] = {};
				DWORD dwLength = ::GetCurrentDirectoryW(MAX_PATH, szPath);
				std::wstring ConvertExe = szPath;
				ConvertExe = ConvertExe + L"/" + m_FBX2glTF;

				SHELLEXECUTEINFOW ShellExecuteInfo;
				ZeroMemory(&ShellExecuteInfo, sizeof(ShellExecuteInfo));
				ShellExecuteInfo.cbSize = sizeof(ShellExecuteInfo);
				ShellExecuteInfo.fMask = SEE_MASK_UNICODE | SEE_MASK_NOCLOSEPROCESS;
				ShellExecuteInfo.lpFile = ConvertExe.c_str();
				ShellExecuteInfo.lpVerb = L"runas";
				ShellExecuteInfo.nShow = SW_HIDE;
				ShellExecuteInfo.lpParameters = Command.c_str();

				bool bSuccess = false;
				if (ShellExecuteExW(&ShellExecuteInfo))
				{
					::WaitForSingleObject(ShellExecuteInfo.hProcess, INFINITE);
					bSuccess = true;

					std::shared_ptr< OpenGLBTask> Task = std::make_shared<OpenGLBTask>();
					Task->fileName = QString::fromStdWString(OutputPath);
					m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(Task));
					m_MainWindow->ResetModelPostion();
				}
			}

		}
	}
}

void BaseToolTab::OnOpenARZIP()
{
	QString strCombineFilter = tr("Resource(*.zip)");
	QString filePath = QFileDialog::getOpenFileName(m_ParentWidget, tr("Open EffectResource"), "../Resource", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) 
	{
		OpenARZipFile(filePath);
	}
}

void BaseToolTab::OnSliderHDRXChanged(int value)
{
	DataCenter::getInstance().RecordInfo.hdrRotateX = value * 3.6f;
	m_MainWindow->ModelUI.EditHDRX->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.hdrRotateX));

	MakeRotateHDREvent();
}

void BaseToolTab::OnSliderHDRYChanged(int value)
{
	DataCenter::getInstance().RecordInfo.hdrRotateY = value * 3.6f;
	m_MainWindow->ModelUI.EditHDRY->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.hdrRotateY));

	MakeRotateHDREvent();
}

void BaseToolTab::OnSliderHDRScaleChanged(int value)
{
	DataCenter::getInstance().RecordInfo.HDRScale = value * 0.02f;
	m_MainWindow->ModelUI.EditHDRScale->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.HDRScale));

	MakeRotateHDREvent();
}

void BaseToolTab::OnSliderHDRContrastChanged(int value)
{
	DataCenter::getInstance().RecordInfo.HDRContrast = value * 0.01f;
	m_MainWindow->ModelUI.EditHDRContrast->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.HDRContrast));

	MakeRotateHDREvent();
}

void BaseToolTab::OnSliderLightX(int value)
{
	DataCenter::getInstance().RecordInfo.LightDir.x = value * 0.02 - 1;
	m_MainWindow->ModelUI.EditLightX->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightDir.x));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightY(int value)
{
	DataCenter::getInstance().RecordInfo.LightDir.y = value * 0.02 - 1;
	m_MainWindow->ModelUI.EditLightY->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightDir.y));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightZ(int value)
{
	DataCenter::getInstance().RecordInfo.LightDir.z = value * 0.02 - 1;
	m_MainWindow->ModelUI.EditLightZ->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightDir.z));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightR(int value)
{
	DataCenter::getInstance().RecordInfo.LightColor.x = value * 0.01;
	m_MainWindow->ModelUI.EditLightR->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightColor.x));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightG(int value)
{
	DataCenter::getInstance().RecordInfo.LightColor.y = value * 0.01;
	m_MainWindow->ModelUI.EditLightG->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightColor.y));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightB(int value)
{
	DataCenter::getInstance().RecordInfo.LightColor.z = value * 0.01;
	m_MainWindow->ModelUI.EditLightB->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightColor.z));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightGamma(int value)
{
	DataCenter::getInstance().RecordInfo.Gamma = value * 0.02;
	m_MainWindow->ModelUI.EditGamma->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.Gamma));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightIntensity(int value)
{
	DataCenter::getInstance().RecordInfo.LightStrength = value * 0.1;
	m_MainWindow->ModelUI.EditLightIntensity->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.LightStrength));
	MakeLightInfoEvent();
}

void BaseToolTab::OnSliderLightAmbient(int value)
{
	//DataCenter::getInstance().RecordInfo.AmbientStrength = value * 0.02;
	//m_MainWindow->ModelUI.EditLightAmbient->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.AmbientStrength));
	//MakeLightInfoEvent();
}

void BaseToolTab::OnSliderAOOffsetChanged(int value)
{
	DataCenter::getInstance().RecordInfo.AOOffset = value * 0.01;
	m_MainWindow->ModelUI.EditAOOffset->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.AOOffset));
	MakeLightInfoEvent();
}

void BaseToolTab::OnCBShadowStateChanged(int value)
{
	DataCenter::getInstance().RecordInfo.bEnableShadow = value == Qt::CheckState::Checked;
	MakeLightInfoEvent();
}

void BaseToolTab::OnExportSetting()
{
	QString FileName = QFileDialog::getSaveFileName(m_ParentWidget, QString::fromStdWString(L"保存材质参数"),
		"../resource", tr("XML(*.xml)"));
	if (!FileName.isNull())
	{
		std::shared_ptr<ExportXMLConfigEvent> Event = std::make_shared<ExportXMLConfigEvent>();
		Event->FileName = FileName;
		m_MainWindow->PostEvent(Event);
	}
}

void BaseToolTab::OnAddDynamicBone()
{
	int index = m_MainWindow->ModelUI.cbSkeleton->currentIndex();
	QString qSkeleton = m_MainWindow->ModelUI.cbSkeleton->currentText();
	std::string skeleton_name = qSkeleton.toUtf8().data();
	bool bFind = false;
	for (auto item : DataCenter::getInstance().RecordInfo.DyBone_array)
	{
		if (item.bone_name == skeleton_name)
		{
			bFind = true;
			break;
		}
	}

	if (!bFind)
	{
		CC3DImageFilter::dynamicBoneParameter db_config;
		db_config.bone_name = skeleton_name;
		DataCenter::getInstance().RecordInfo.DyBone_array.push_back(db_config);
		m_MainWindow->ModelUI.cbDynamicBone->addItem(qSkeleton);

		std::shared_ptr<DynamicBoneEvent> boneEvent = std::make_shared<DynamicBoneEvent>();
		boneEvent->dbType = DynamicBoneOperateType::ADD_DB;
		boneEvent->dbName = qSkeleton;
		m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(boneEvent));
	}
}

void BaseToolTab::OnDeleteDynamicBone()
{
	int index = m_MainWindow->ModelUI.cbDynamicBone->currentIndex();
	if (index != -1)
	{
		m_MainWindow->ModelUI.cbDynamicBone->removeItem(index);
		QString bone_name = QString::fromUtf8(DataCenter::getInstance().RecordInfo.DyBone_array[index].bone_name.c_str());
		DataCenter::getInstance().RecordInfo.DyBone_array.erase(DataCenter::getInstance().RecordInfo.DyBone_array.begin() + index);

		std::shared_ptr<DynamicBoneEvent> boneEvent = std::make_shared<DynamicBoneEvent>();
		boneEvent->dbType = DynamicBoneOperateType::DELETE_DB;
		boneEvent->dbName = bone_name;
		m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(boneEvent));
	}
}

void BaseToolTab::OnClearDynamicBone()
{
	if (m_MainWindow)
	{
		DataCenter::getInstance().RecordInfo.DyBone_array.clear();
		m_MainWindow->ModelUI.cbDynamicBone->clear();
		std::shared_ptr<DynamicBoneEvent> boneEvent = std::make_shared<DynamicBoneEvent>();
		boneEvent->dbType = DynamicBoneOperateType::CLEAR_DB;
		//boneEvent->dbName = bone_name;
		m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(boneEvent));
	}
}

void BaseToolTab::OnSliderDampingChanged(int value)
{
	auto& DyBoneArray = DataCenter::getInstance().RecordInfo.DyBone_array;
	if (!DyBoneArray.empty())
	{
		int index = m_MainWindow->ModelUI.cbDynamicBone->currentIndex();
		if (index > -1)
		{
			CC3DImageFilter::dynamicBoneParameter& db_param = DyBoneArray[index];
			db_param._fDamping = value * 0.01;
			m_MainWindow->ModelUI.lineEditDamping->setText(QString("%1").arg(db_param._fDamping));
			UpdateDynamicBoneConfig(db_param);
		}

	}
}

void BaseToolTab::OnSliderElasticityChanged(int value)
{
	auto& DyBoneArray = DataCenter::getInstance().RecordInfo.DyBone_array;
	if (!DyBoneArray.empty())
	{
		int index = m_MainWindow->ModelUI.cbDynamicBone->currentIndex();
		if (index > -1)
		{
			CC3DImageFilter::dynamicBoneParameter& db_param = DyBoneArray[index];
			db_param._fElasticity = value * 0.01;
			m_MainWindow->ModelUI.lineEditElasticity->setText(QString("%1").arg(db_param._fElasticity));
			UpdateDynamicBoneConfig(db_param);
		}
	}
}

void BaseToolTab::OnSliderStiffnessChanged(int value)
{
	auto& DyBoneArray = DataCenter::getInstance().RecordInfo.DyBone_array;
	if (!DyBoneArray.empty())
	{
		int index = m_MainWindow->ModelUI.cbDynamicBone->currentIndex();
		if (index > -1)
		{
			CC3DImageFilter::dynamicBoneParameter& db_param = DyBoneArray[index];
			db_param._fStiffness = value * 0.01;
			m_MainWindow->ModelUI.lineEditStiffness->setText(QString("%1").arg(db_param._fStiffness));
			UpdateDynamicBoneConfig(db_param);
		}

	}
}

void BaseToolTab::OnSliderInertChanged(int value)
{
	auto& DyBoneArray = DataCenter::getInstance().RecordInfo.DyBone_array;
	if (!DyBoneArray.empty())
	{
		int index = m_MainWindow->ModelUI.cbDynamicBone->currentIndex();
		if (index > -1)
		{
			CC3DImageFilter::dynamicBoneParameter& db_param = DyBoneArray[index];
			db_param._fInert = value * 0.01;
			m_MainWindow->ModelUI.lineEditInert->setText(QString("%1").arg(db_param._fInert));
			UpdateDynamicBoneConfig(db_param);
		}

	}
}

void BaseToolTab::OnDynamicBoneIndexChanged(int index)
{
	if (index == -1)
	{
		m_MainWindow->ModelUI.SliderDamping->setValue(0);
		m_MainWindow->ModelUI.SliderElasticity->setValue(0);
		m_MainWindow->ModelUI.SliderStiffness->setValue(0);
		m_MainWindow->ModelUI.SliderInert->setValue(0);
		m_MainWindow->ModelUI.lineEditDamping->setText("0");
		m_MainWindow->ModelUI.lineEditElasticity->setText("0");
		m_MainWindow->ModelUI.lineEditStiffness->setText("0");
		m_MainWindow->ModelUI.lineEditInert->setText("0");
	}
	else
	{
		CC3DImageFilter::dynamicBoneParameter db_param = DataCenter::getInstance().RecordInfo.DyBone_array[index];
		m_MainWindow->ModelUI.SliderDamping->setValue((int)(db_param._fDamping * 100));
		m_MainWindow->ModelUI.SliderElasticity->setValue((int)(db_param._fElasticity * 100));
		m_MainWindow->ModelUI.SliderStiffness->setValue((int)(db_param._fStiffness * 100));
		m_MainWindow->ModelUI.SliderInert->setValue((int)(db_param._fInert * 100));
	}
}

void BaseToolTab::OnToneMappingIndexChanged(int value)
{
	DataCenter::getInstance().RecordInfo.ToneMapping.ToneMappingType = value;
	m_MainWindow->ModelUI.SliderToneMappingContrast->setEnabled(value == 1);
	m_MainWindow->ModelUI.SliderToneMappingSaturation->setEnabled(value == 1);

	m_MainWindow->PostEvent(std::make_shared<ChangeToneMappingEvent>());
}

void BaseToolTab::OnSliderToneMappingContrastChanged(int value)
{
	DataCenter::getInstance().RecordInfo.ToneMapping.Contrast = (double)value * 0.03;
	m_MainWindow->ModelUI.lineEditToneMappingContrast->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.ToneMapping.Contrast));

	m_MainWindow->PostEvent(std::make_shared<ChangeToneMappingEvent>());
}

void BaseToolTab::OnSliderToneMappingSaturationChanged(int value)
{
	DataCenter::getInstance().RecordInfo.ToneMapping.Saturation = (double)value * 0.03;
	m_MainWindow->ModelUI.lineEditToneMappingSaturation->setText(QString("%1").arg(DataCenter::getInstance().RecordInfo.ToneMapping.Saturation));

	m_MainWindow->PostEvent(std::make_shared<ChangeToneMappingEvent>());
}

void BaseToolTab::OnCBUseVideoChange(int value)
{
	std::shared_ptr< VideoPreviewEvent> ViewPreview = std::make_shared<VideoPreviewEvent>();
	ViewPreview->FileName = m_CurrentVideoFileName;
	ViewPreview->UseVideo = value == Qt::CheckState::Checked;
	m_MainWindow->PostEvent(ViewPreview);

	m_MainWindow->GetPreview()->ResetMousePos();

	m_MainWindow->ModelUI.BtnChangeVideo->setEnabled(ViewPreview->UseVideo);
}

void BaseToolTab::OnChangeVideoFile()
{
	QString strCombineFilter = tr("Video(*.mp4 *.flv *.avi *.mkv *.mov)");
	QString filePath = QFileDialog::getOpenFileName(m_MainWindow, tr("Open Video"), "../resource", strCombineFilter, nullptr);
	if (!filePath.isEmpty()){
		OpenVideoFile(filePath);
	}
}

void BaseToolTab::OnChangeHDRFile()
{
	QString strCombineFilter = tr("Hdr(*.hdr)");
	QString filePath = QFileDialog::getOpenFileName(m_MainWindow, tr("Open HDR"), "../resource", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) {
		OpenHDRFile(filePath);
	}
}

void BaseToolTab::OpenVideoFile(const QString& videoFile)
{
	m_CurrentVideoFileName = videoFile;
	std::shared_ptr< VideoPreviewEvent> ViewPreview = std::make_shared<VideoPreviewEvent>();
	ViewPreview->FileName = m_CurrentVideoFileName;
	ViewPreview->UseVideo = true;
	m_MainWindow->PostEvent(ViewPreview);
}

void BaseToolTab::OpenARZipFile(const QString& ARZipFile)
{
	m_MainWindow->ModelUI.cbSkeleton->clear();

	std::shared_ptr<ChangeAREffect> Task = std::make_shared<ChangeAREffect>();
	Task->fileName = ARZipFile;
	m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(Task));
	m_MainWindow->ResetModelPostion();
}

void BaseToolTab::OpenHDRFile(const QString& HDRFile)
{
	std::filesystem::path  path = HDRFile.toUtf8().data();

	DataCenter::getInstance().RecordInfo.HDRFileName = path.filename().string();
	std::shared_ptr< ChangeHDRFileEvent> ViewPreview = std::make_shared<ChangeHDRFileEvent>();
	ViewPreview->FileName = HDRFile;
	m_MainWindow->PostEvent(ViewPreview);
}

void BaseToolTab::OpenXMLFile(const QString& XML)
{

	std::shared_ptr< OepnXMLFileEvent> ViewPreview = std::make_shared<OepnXMLFileEvent>();
	ViewPreview->FileName = XML;
	m_MainWindow->PostEvent(ViewPreview);
}

void BaseToolTab::MakeRotateHDREvent()
{
	std::shared_ptr< RotateHDREvent> RotateEvent = std::make_shared<RotateHDREvent>();
	RotateEvent->hdrRotateX = DataCenter::getInstance().RecordInfo.hdrRotateX;
	RotateEvent->hdrRotateY = DataCenter::getInstance().RecordInfo.hdrRotateY;
	RotateEvent->hdrScale = DataCenter::getInstance().RecordInfo.HDRScale;
	RotateEvent->hdrContrast = DataCenter::getInstance().RecordInfo.HDRContrast;
	m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(RotateEvent));
}

void BaseToolTab::MakeLightInfoEvent()
{
	std::shared_ptr< LightInfoEvent> LightEvent = std::make_shared<LightInfoEvent>();
	m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(LightEvent));
}

void BaseToolTab::UpdateDynamicBoneConfig(const CC3DImageFilter::dynamicBoneParameter& db_param)
{
	std::shared_ptr<DynamicBoneEvent> boneEvent = std::make_shared<DynamicBoneEvent>();
	boneEvent->dbType = DynamicBoneOperateType::Update_DB;
	boneEvent->dbName = QString::fromStdString(db_param.bone_name);
	m_MainWindow->PostEvent(std::static_pointer_cast<TaskEvent>(boneEvent));
}

void BaseToolTab::UpdateTabWidget(int Index)
{
	if (Index == 3)
	{
		for (auto& Item : AttrUis)
		{
			delete Item;
		}
		AttrUis.clear();

		bool bFinished = false;
		std::shared_ptr< GetCurrentEffectAttrEvent> AttrEvent = std::make_shared<GetCurrentEffectAttrEvent>();
		AttrEvent->FinishCallBack = [&bFinished]()
		{
			bFinished = true;
		};
		m_MainWindow->PostEvent(AttrEvent);
		while (!bFinished);
		int UIIndex = 0;
		for (auto Effect : AttrEvent->Effects)
		{
			DynamicAttrUI* AttrUI = new DynamicAttrUI(m_MainWindow->ModelUI.PartAttr, UIIndex++, this);
			AttrUI->AttrLabel->setText(Effect.ShowName);
			AttrUI->AttrSlider->setValue(Effect.Alpha * 100);
			AttrUI->AttrLineEdit->setText(QString::number(AttrUI->AttrSlider->value() / 100.0f, 'f', 2));
			AttrUI->pEffect = Effect.pEffect;
			AttrUis.push_back(AttrUI);
		}
	}
}

void BaseToolTab::OnAttrValueChanged(int value)
{
	QSlider* AttrSlider = qobject_cast<QSlider*>(sender());
	for (auto AttrUI : AttrUis)
	{
		if (AttrUI->AttrSlider == AttrSlider)
		{
			AttrUI->AttrLineEdit->setText(QString::number(AttrUI->AttrSlider->value() / 100.0f, 'f', 2));
			std::shared_ptr< SetEffectAttrEvent> AttrEvent = std::make_shared<SetEffectAttrEvent>();
			AttrEvent->AttrData.ShowName = AttrUI->AttrLabel->text();
			AttrEvent->AttrData.Alpha = AttrUI->AttrSlider->value() / 100.0f;
			AttrEvent->AttrData.pEffect = AttrUI->pEffect;
			m_MainWindow->PostEvent(AttrEvent);
		}
	}
}

DynamicAttrUI::DynamicAttrUI(QWidget* Parent, int Index, QObject* MessageReceiver)
{
	ParentUI = Parent;
	AttrLabel = new QLabel(ParentUI);
	AttrLabel->setGeometry(QRect(20, 20 + Index * 30, 100, 40));
	AttrLabel->show();

	AttrSlider = new QSlider(ParentUI);
	AttrSlider->setGeometry(QRect(140, 30 + Index * 30, 110, 20));
	AttrSlider->setMaximum(100);
	AttrSlider->setOrientation(Qt::Horizontal);
	AttrSlider->show();
	AttrSlider->connect(AttrSlider, SIGNAL(valueChanged(int)), MessageReceiver, SLOT(OnAttrValueChanged(int)));

	AttrLineEdit = new QLineEdit(ParentUI);
	AttrLineEdit->setGeometry(QRect(260, 30 + Index * 30, 50, 20));
	AttrLineEdit->setReadOnly(true);
	AttrLineEdit->show();
}

DynamicAttrUI::~DynamicAttrUI()
{
	if (AttrLabel)
	{
		delete AttrLabel;
		AttrLabel = nullptr;
	}
	if (AttrSlider)
	{
		delete AttrSlider;
		AttrSlider = nullptr;
	}
	if (AttrLineEdit)
	{
		delete AttrLineEdit;
		AttrLineEdit = nullptr;
	}
}
