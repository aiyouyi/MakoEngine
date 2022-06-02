#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <vector>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QColorDialog>
#include <QPushButton>
#include "EffectKernel2\FacialAnimation.h"

class NPREditWidget : public QWidget {
	Q_OBJECT
public:
	NPREditWidget(FacialAnimation* ani, QWidget *parent = nullptr);
	virtual ~NPREditWidget();

protected:
	void paintEvent(QPaintEvent* event) override;

public slots:
	void selectColor();
	void changeColor(QColor color);

private:
	FacialAnimation* m_ani;
	QCheckBox*		 m_use_npr;

	QLabel*			 m_line_label;
	QLineEdit*		 m_line_width;

	QPushButton*	 m_color_btn;

	QVBoxLayout*	 m_vlayout;
};