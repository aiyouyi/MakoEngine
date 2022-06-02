#include <QStyleOption>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include "TextureWidget.h"
#include "Toolbox\DXUtils\DX11Texture.h"
#include "common.h"

TextureWidget::TextureWidget(DX11Texture* tex, QWidget* parent)
	: m_tex(tex), QWidget(parent) {
	setFixedHeight(100 + 20);
	
	m_img = new QLabel(this);
	QPixmap pix = getPixmapFromTex();
	m_img->setAlignment(Qt::AlignCenter);
	pix = pix.scaled(pix.width(), 100, Qt::KeepAspectRatio);
	m_img->setPixmap(pix);

	m_replace_btn = new QPushButton("Replace", this);
	m_replace_btn->setFixedHeight(20);
	connect(m_replace_btn, &QPushButton::clicked, this, &TextureWidget::openImage);

	m_vlayout = new QVBoxLayout;
	m_vlayout->setContentsMargins(-1, 0, -1, 0);
	m_vlayout->addWidget(m_img);
	m_vlayout->addWidget(m_replace_btn);

	setLayout(m_vlayout);
}

TextureWidget::~TextureWidget() {

}

void TextureWidget::paintEvent(QPaintEvent * event) {
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	QWidget::paintEvent(event);
}

void TextureWidget::mousePressEvent(QMouseEvent * event)
{
	qDebug() << "save pixmap" << m_img->width() << " " << m_img->height();
}

void TextureWidget::openImage() {
	QString strCombineFilter = tr("Images(*.png *.jpg *.jpeg *.bmp)");

	QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", strCombineFilter, nullptr);
	if (!filePath.isEmpty()) {
		QPixmap pix(filePath);
		pix = pix.scaled(pix.width(), 100, Qt::KeepAspectRatio);
		m_img->setPixmap(pix);

		loadTexture(filePath);
	}
}

void TextureWidget::loadTexture(QString path) {
	DX11Texture *pTex = new DX11Texture();
	pTex->initTextureFromFileCPUAcess(path.toStdString());

	replaceTexture(pTex);
	delete m_tex;
	m_tex = pTex;
}

QPixmap TextureWidget::getPixmapFromTex()
{
	int w = m_tex->width();
	int h = m_tex->height();
	uchar *buf = new uchar[w * h * 4];
	m_tex->ReadTextureToCpu(buf);
	QImage img(buf, w, h, w * 4, QImage::Format_RGBA8888);
	QPixmap pix;
	pix.convertFromImage(img);
	delete buf;

	return pix;
}
