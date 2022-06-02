#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <vector>
#include <map>

#include "TextureWidget.h"

class CFaceMakeUp;

class CFaceMakeUpEidtWidget : public QWidget {
	Q_OBJECT
public:
	CFaceMakeUpEidtWidget(CFaceMakeUp* face, QWidget* parent = nullptr);
	virtual ~CFaceMakeUpEidtWidget();

public slots:
	void replaceTexture(DX11Texture* tex);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	CFaceMakeUp*	m_face;

	QVBoxLayout*	m_vlayout;

	std::vector<TextureWidget*>		m_tex_ws;
	std::map<TextureWidget*, int>	m_tex_w_int;
};