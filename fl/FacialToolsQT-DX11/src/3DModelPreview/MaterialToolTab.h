#pragma once
#include "inc.h"
#include "QUIEvent.h"

class ARModelPreviewWindow;

class MaterialToolTab : public QObject
{
	Q_OBJECT
public:
	MaterialToolTab(QWidget* parent, ARModelPreviewWindow* Main);
	~MaterialToolTab();

	void SetUIValue(QFurDataEvent* E);

private slots:
	void OnSliderFurLightExposure(int value);
	void OnSliderFurAmbientStrength(int value);
	void OnSliderFurLevelChanged(int value);
	void OnSliderFurLengthChanged(int value);
	void OnSliderFurUVScaleChanged(int value);
	void OnSliderFurLightFilterChanged(int value);

private:
	QPointer<QWidget> m_ParentWidget;
	QPointer<ARModelPreviewWindow> m_MainWindow;
};