#ifndef MAINWIN_H
#define MAINWIN_H

#include <QtWidgets/QMainWindow>
#include <QtCore/QFile>
#include <QtCore/QThread>

#include "DefaultMessages.h"
#include "MessageQueue.h"

class Worker : public QObject
{
	Q_OBJECT

public slots:
	void send_one(Message _message)
	{
		_message.set_text(_message.text() + QString("0x%1").arg((int)QThread::currentThread()));
		MessageQueue::instance() << _message;
	}

	void send_ten(Message _message)
	{
		_message.set_text(_message.text() + QString("0x%1").arg((int)QThread::currentThread()));

		for (int i = 0; i < 10; ++i)
			MessageQueue::instance() << _message;
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
