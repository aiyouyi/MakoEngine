#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include "CFaceMakeUp_EditWidget.h"
#include "EffectKernel\FaceMakeUp\CFaceMakeUp.h"

CFaceMakeUpEidtWidget::CFaceMakeUpEidtWidget(CFaceMakeUp* face, QWidget* parent)
	: m_face(face), QWidget(parent) {
// 	setStyleSheet("background-color:#0000ff;");
// 	
// 	m_vlayout = new QVBoxLayout;
// 
// 	vector<MakeUpInfo>& make_infoes = m_face->getMakeUpInfo();
// 	for (int i = 0; i < make_infoes.size(); i++) {
// 		TextureWidget *tex_w = new TextureWidget(make_infoes[i].m_material, this);
// 		m_tex_ws.push_back(tex_w);
// 		m_tex_w_int[tex_w] = i;
// 		connect(tex_w, &TextureWidget::replaceTexture, this, &CFaceMakeUpEidtWidget::replaceTexture);
// 	}
// 	for(auto ww : m_tex_ws)
// 		m_vlayout->addWidget(ww);
// 	
// 	setLayout(m_vlayout);
}

CFaceMakeUpEidtWidget::~CFaceMakeUpEidtWidget() {
}

void CFaceMakeUpEidtWidget::replaceTexture(DX11Texture* tex) {
// 	TextureWidget* sender = static_cast<TextureWidget*>(QObject::sender());
// 	if (m_tex_w_int.end() != m_tex_w_int.find(sender)) {
// 		int i = m_tex_w_int[sender];
// 		m_face->getMakeUpInfo()[i].m_material = tex;
// 	}
}

void CFaceMakeUpEidtWidget::paintEvent(QPaintEvent * event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}
