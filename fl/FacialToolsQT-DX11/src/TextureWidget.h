#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

class DX11Texture;

class TextureWidget : public QWidget {
	Q_OBJECT
public:
	TextureWidget(DX11Texture* tex, QWidget* parent = nullptr);
	virtual ~TextureWidget();

public slots:
	void openImage();

Q_SIGNALS:
	void replaceTexture(DX11Texture* tex);

protected:
	void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;

private:
	QPixmap getPixmapFromTex();
	void loadTexture(QString path);

private:
	DX11Texture*	 m_tex;

	QVBoxLayout*	 m_vlayout;
	QLabel*			 m_img;
	QPushButton*	 m_replace_btn;
};