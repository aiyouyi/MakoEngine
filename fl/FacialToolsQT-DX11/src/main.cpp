#include <QtWidgets/QApplication>
#include <QDebug>
#include "mainWidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWidget w;
	w.resize(1080, 720);
	w.show();


	qDebug() << "heguanyou";
	return a.exec();
}
