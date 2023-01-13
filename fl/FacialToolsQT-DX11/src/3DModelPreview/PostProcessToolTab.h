#pragma once

#pragma once
#include "inc.h"
#include "QUIEvent.h"

class ARModelPreviewWindow;

class ProcesProcessToolTab : public QObject
{
	Q_OBJECT
public:
	ProcesProcessToolTab(QWidget* parent, ARModelPreviewWindow* Main);
	~ProcesProcessToolTab();

	void SetUIValue(QPostProcessEvent* E);

private slots:
	void OnCBEnableBloomChanged(int value);
	void OnSliderBloomAlphaChanged(int value);
	void OnSliderBloomRadiusChanged(int value);
	void OnSliderBloomLoopTimeChanged(int value);
	void OnSliderBloomStrengthChanged(int value);

private:
	QPointer<QWidget> m_ParentWidget;
	QPointer<ARModelPreviewWindow> m_MainWindow;
};