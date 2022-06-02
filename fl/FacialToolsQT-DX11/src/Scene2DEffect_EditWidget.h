#pragma once

#include <QWidget>
#include <vector>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "EffectKernel2\FacialAnimation.h"

class Effect2DRect;

class Scene2DEffectEditWidget : public QWidget
{
	Q_OBJECT

public:
	Scene2DEffectEditWidget(FacialAnimation *ani, QWidget *parent = nullptr);
	~Scene2DEffectEditWidget();

public slots:
	void editFinish();
	void alignChange(const QString & str);
	void openFolder();
	void getImageFileList();

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	FacialAnimation*		 m_ani;
	
	QHBoxLayout*	m_hlayout;
	QVBoxLayout*	m_vlayout;
	QLabel*			m_pos;
	QLineEdit*		m_pos_x;
	QLineEdit*		m_pos_y;

	QComboBox*		m_align_box;

	QLineEdit*		m_open_folder;
};