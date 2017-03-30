#ifndef MAINWIN_H
#define MAINWIN_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QFile>
#include <QtCore/QThread>

#include "Message.h"
#include "MessageQueue.h"

class Worker : public QObject
{
	Q_OBJECT

public slots:
	void send_one(MessageQueue* _msgqueue, MsgType _mtype, const QString& _msg)
	{
		*_msgqueue << Message(_mtype, _msg + QString("0x%1").arg((int)QThread::currentThread()));
	}

	void send_ten(MessageQueue* _msgqueue, MsgType _mtype, const QString& _msg)
	{
		for (int i = 0; i < 10; ++i)
			*_msgqueue << Message(_mtype, _msg + QString("0x%1").arg((int)QThread::currentThread()));
	}
};

class MainWin : public QMainWindow
{
	Q_OBJECT

public:
	MainWin(QWidget *parent = 0);
	~MainWin();

	QFile* out;
};

#endif // MAINWIN_H
