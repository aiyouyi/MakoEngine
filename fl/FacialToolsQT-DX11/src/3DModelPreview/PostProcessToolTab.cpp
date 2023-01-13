#include "PostProcessToolTab.h"
#include "ARModelPreviewWindow.h"
#include "Task/basetask.h"
#include "DataCenter.h"
#include <QtGui>

ProcesProcessToolTab::ProcesProcessToolTab(QWidget* parent, ARModelPreviewWindow* Main)
	:m_ParentWidget(parent)
	, m_MainWindow(Main)
{
	QObject::connect(m_MainWindow->ModelUI.cbEnableBloom, SIGNAL(stateChanged(int)), this, SLOT(OnCBEnableBloomChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderBloomAlpha, SIGNAL(valueChanged(int)), this, SLOT(OnSliderBloomAlphaChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderBloomRadius, SIGNAL(valueChanged(int)), this, SLOT(OnSliderBloomRadiusChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderBloomLoopTime, SIGNAL(valueChanged(int)), this, SLOT(OnSliderBloomLoopTimeChanged(int)));
	QObject::connect(m_MainWindow->ModelUI.SliderBloomStrength, SIGNAL(valueChanged(int)), this, SLOT(OnSliderBloomStrengthChanged(int)));
}

ProcesProcessToolTab::~ProcesProcessToolTab()
{

}

void ProcesProcessToolTab::SetUIValue(QPostProcessEvent* E)
{
	const auto& Config = DataCenter::getInstance().RecordInfo;
	m_MainWindow->ModelUI.SliderBloomAlpha->setSliderPosition(Config.Bloom.BloomAlpha / 0.03);
	m_MainWindow->ModelUI.SliderBloomRadius->setSliderPosition(Config.Bloom.Bloomradius / 0.03);
	m_MainWindow->ModelUI.SliderBloomLoopTime->setSliderPosition(Config.Bloom.Bloomlooptime / 5);
	m_MainWindow->ModelUI.SliderBloomStrength->setSliderPosition(Config.Bloom.BloomStrength / 0.02);

	m_MainWindow->ModelUI.cbEnableBloom->setChecked(Config.Bloom.EnableBloom);
}

void ProcesProcessToolTab::OnCBEnableBloomChanged(int value)
{
	auto& Config = DataCenter::getInstance().RecordInfo;
	Config.Bloom.EnableBloom = value == Qt::CheckState::Checked;
	m_MainWindow->PostEvent(std::make_shared<BloomEvent>());
}

void ProcesProcessToolTab::OnSliderBloomAlphaChanged(int value)
{
	auto& Config = DataCenter::getInstance().RecordInfo;
	Config.Bloom.BloomAlpha = value * 0.03;
	m_MainWindow->ModelUI.EditBloomAlpha->setText(QString("%1").arg(Config.Bloom.BloomAlpha));

	m_MainWindow->PostEvent(std::make_shared<BloomEvent>());
}

void ProcesProcessToolTab::OnSliderBloomRadiusChanged(int value)
{
	auto& Config = DataCenter::getInstance().RecordInfo;
	Config.Bloom.Bloomradius = value * 0.03;
	m_MainWindow->ModelUI.EditBloomRadius->setText(QString("%1").arg(Config.Bloom.Bloomradius));

	m_MainWindow->PostEvent(std::make_shared<BloomEvent>());
}

void ProcesProcessToolTab::OnSliderBloomLoopTimeChanged(int value)
{
	auto& Config = DataCenter::getInstance().RecordInfo;
	Config.Bloom.Bloomlooptime = value * 5;
	m_MainWindow->ModelUI.EditBloomLoopTime->setText(QString("%1").arg(Config.Bloom.Bloomlooptime));

	m_MainWindow->PostEvent(std::make_shared<BloomEvent>());
}


void ProcesProcessToolTab::OnSliderBloomStrengthChanged(int value)
{
	auto& Config = DataCenter::getInstance().RecordInfo;
	Config.Bloom.BloomStrength = value * 0.02;
	m_MainWindow->ModelUI.EditBloomStrength->setText(QString("%1").arg(Config.Bloom.BloomStrength));

	m_MainWindow->PostEvent(std::make_shared<BloomEvent>());
}
