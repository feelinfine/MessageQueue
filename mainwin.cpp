#include "mainwin.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>

#include "MessageQueue.h"

MainWin::MainWin(QWidget *parent) : QMainWindow(parent)
{
	//default
	const size_t WIN_W = 640;
	const size_t WIN_H = 480;

	const size_t TIMER_INTERVAL = 1000; //msec

	//timer
	QTimer* message_timer = new QTimer();
	message_timer->setInterval(TIMER_INTERVAL);

	MessageQueue::instance().set_base_widget(this);

	QObject::connect(message_timer, &QTimer::timeout, this, []()
	{
		MessageQueue::instance() << "Ololo";
	});

	//controls
	QPushButton* start_messaging = new QPushButton("START", this);
	start_messaging->setCheckable(true);

	QObject::connect(start_messaging, &QPushButton::clicked, [start_messaging, message_timer](bool _checked)
	{
		if (_checked)
		{
			start_messaging->setText("STOP");
			message_timer->start();
		}
		else
		{
			start_messaging->setText("START");
			message_timer->stop();
		}
	});


	//layout
	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->addWidget(start_messaging);
	bottom_layout->addStretch(1);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->addStretch(1);
	main_layout->addLayout(bottom_layout);

	QWidget* main_widget = new QWidget();
	main_widget->setLayout(main_layout);

	setCentralWidget(main_widget);

	resize(WIN_W, WIN_H);
}

MainWin::~MainWin()
{

}
