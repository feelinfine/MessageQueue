#ifndef MAINWIN_H
#define MAINWIN_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwin.h"

class MainWin : public QMainWindow
{
	Q_OBJECT

public:
	MainWin(QWidget *parent = 0);
	~MainWin();

private:
	Ui::MainWinClass ui;
};

#endif // MAINWIN_H
