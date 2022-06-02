#pragma once

#include <QWidget>
#include <QVBoxLayout>

#include "d3d11renderwidget.h"

class RenderBgWidget : public QWidget {
	Q_OBJECT
public:
	RenderBgWidget(QWidget* parent = nullptr);
	virtual ~RenderBgWidget();

	D3d11RenderWidget* getRenderWidget() { return m_render; }

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	D3d11RenderWidget*	m_render;
	QVBoxLayout*		m_layout;
};