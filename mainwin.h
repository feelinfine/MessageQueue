#ifndef MAINWIN_H
#define MAINWIN_H

#include <QtWidgets/QMainWindow>

class MainWin : public QMainWindow
{
	Q_OBJECT

public:
	MainWin(QWidget *parent = 0);
	~MainWin();
};

#endif // MAINWIN_H
