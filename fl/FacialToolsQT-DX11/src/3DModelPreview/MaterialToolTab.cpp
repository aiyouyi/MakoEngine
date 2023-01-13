#include "MaterialToolTab.h"
#include "ARModelPreviewWindow.h"
#include "Task/basetask.h"
#include "DataCenter.h"
#include <QtGui>

MaterialToolTab::MaterialToolTab(QWidget* parent, ARModelPreviewWindow* Main)
	:m_ParentWidget(parent)
	, m_MainWindow(Main)
{
	QObject::connect(m_MainWindow->ModelUI.SliderFurLightExposure, SIGNAL(valueChanged(int)), this, SLOT(OnSliderFurLightExposure(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderFurLightAmbient, SIGNAL(valueChanged(int)), this, SLOT(OnSliderFurAmbientStrength(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderFurLevel, SIGNAL(valueChanged(int)), this, SLOT(OnSliderFurLevelChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderFurLength, SIGNAL(valueChanged(int)), this, SLOT(OnSliderFurLengthChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderFurUVScale, SIGNAL(valueChanged(int)), this, SLOT(OnSliderFurUVScaleChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderFurLightFilter, SIGNAL(valueChanged(int)), this, SLOT(OnSliderFurLightFilterChanged(int)));
}

MaterialToolTab::~MaterialToolTab()
{

}

void MaterialToolTab::SetUIValue(QFurDataEvent* E)
{
	auto& FurData = DataCenter::getInstance().FurData;
	m_MainWindow->ModelUI.SliderFurLightAmbient->setSliderPosition(FurData.FurAmbientStrength / 0.03);
	m_MainWindow->ModelUI.SliderFurLightExposure->setSliderPosition(FurData.FurLightExposure / 0.02);
	m_MainWindow->ModelUI.SliderFurLevel->setSliderPosition(FurData.FurLevel );
	m_MainWindow->ModelUI.SliderFurLength->setSliderPosition((double)FurData.FurLength / 0.03);
	m_MainWindow->ModelUI.SliderFurUVScale->setSliderPosition((double)FurData.UVScale / 0.3);
	m_MainWindow->ModelUI.SliderFurLightFilter->setSliderPosition((double)FurData.LightFilter / 0.02);
}

void MaterialToolTab::OnSliderFurLightExposure(int value)
{
	DataCenter::getInstance().FurData.FurLightExposure = value * 0.02;
	m_MainWindow->ModelUI.EditFurLightExposure->setText(QString("%1").arg(DataCenter::getInstance().FurData.FurLightExposure));
	m_MainWindow->PostEvent(std::make_shared<FurDataEvent>());
}

void MaterialToolTab::OnSliderFurAmbientStrength(int value)
{
	DataCenter::getInstance().FurData.FurAmbientStrength = value * 0.03;
	m_MainWindow->ModelUI.EditFurLightAmbient->setText(QString("%1").arg(DataCenter::getInstance().FurData.FurAmbientStrength));
	m_MainWindow->PostEvent(std::make_shared<FurDataEvent>());
}

void MaterialToolTab::OnSliderFurLevelChanged(int value)
{
	DataCenter::getInstance().FurData.FurLevel = value ;
	m_MainWindow->ModelUI.EditFurLevel->setText(QString("%1").arg(DataCenter::getInstance().FurData.FurLevel));
	m_MainWindow->PostEvent(std::make_shared<FurDataEvent>());
}

void MaterialToolTab::OnSliderFurLengthChanged(int value)
{
	DataCenter::getInstance().FurData.FurLength = value *0.03;
	m_MainWindow->ModelUI.EditFurLength->setText(QString("%1").arg(DataCenter::getInstance().FurData.FurLength));
	m_MainWindow->PostEvent(std::make_shared<FurDataEvent>());
}

void MaterialToolTab::OnSliderFurUVScaleChanged(int value)
{
	DataCenter::getInstance().FurData.UVScale = value * 0.3;
	m_MainWindow->ModelUI.EditFurUVScale->setText(QString("%1").arg(DataCenter::getInstance().FurData.UVScale));
	m_MainWindow->PostEvent(std::make_shared<FurDataEvent>());
}

void MaterialToolTab::OnSliderFurLightFilterChanged(int value)
{
	DataCenter::getInstance().FurData.LightFilter = value * 0.02;
	m_MainWindow->ModelUI.EditFurLightFilter->setText(QString("%1").arg(DataCenter::getInstance().FurData.LightFilter));
	m_MainWindow->PostEvent(std::make_shared<FurDataEvent>());
}
