#include "mainwin.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWin w;
	w.show();
	return a.exec();
}
