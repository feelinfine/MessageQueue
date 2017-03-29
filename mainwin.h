#ifndef MAINWIN_H
#define MAINWIN_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QFile>

class MainWin : public QMainWindow
{
	Q_OBJECT

public:
	MainWin(QWidget *parent = 0);
	~MainWin();

	QFile* out;
};

#endif // MAINWIN_H
