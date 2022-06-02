#pragma once

#include <QWidget>
#include <vector>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "Toolbox\EffectModel.hpp"

class Effect2DRect;
class C2DPicTextureEffect;

const static std::string szArrAlignType[] = { "EAPT_LT", "EAPT_LB", "EAPT_RT", "EAPT_RB", "EAPT_CT", \
"EAPT_CB", "EAPT_LC", "EAPT_RC", "EAPT_CC" };
static std::string getStringFromAlignType(en_AlignParentType type) {
	if (type < EAPT_MAX) {
		return szArrAlignType[type];
	}
	return "EAPT_MAX";
}

static en_AlignParentType getAlignTypeFromString(const std::string str) {
	for (int i = 0; i < sizeof(szArrAlignType) / sizeof(std::string); i++) {
		if (str == szArrAlignType[i]) {
			return en_AlignParentType(i);
		}
	}
	return EAPT_MAX;
}

class C2DPicTextEidtWidget : public QWidget
{
	Q_OBJECT

public:
	C2DPicTextEidtWidget(C2DPicTextureEffect *pic, QWidget *parent = nullptr);
	~C2DPicTextEidtWidget();

public slots:
	void editFinish();
	void alignChange(const QString & str);
	void openFolder();
	void getImageFileList();

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	C2DPicTextureEffect*		 m_pic;
	
	QHBoxLayout*	m_hlayout;
	QVBoxLayout*	m_vlayout;
	QLabel*			m_pos;
	QLineEdit*		m_pos_x;
	QLineEdit*		m_pos_y;
	QLabel*			m_width_label;
	QLineEdit*		m_width;
	QLabel*			m_height_label;
	QLineEdit*		m_height;

	QComboBox*		m_align_box;

	QLineEdit*		m_open_folder;
};