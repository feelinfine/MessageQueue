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

	out = new QFile("log.txt");
	bool res = out->open(QIODevice::WriteOnly | QIODevice::Text);

	MessageQueue::instance().set_output_stream(out);

	static size_t counter = 0;
	QObject::connect(message_timer, &QTimer::timeout, this, []()
	{
		MessageQueue::instance() << Message(MsgType(counter % 3), QString::number(++counter));
	});

	//controls
	QPushButton* start_messaging = new QPushButton("START", this);
	start_messaging->setCheckable(true);

	QObject::connect(start_messaging, &QPushButton::clicked, [start_messaging, message_timer, this](bool _checked)
	{
			//for (int i = 0; i < 10; ++i)
			//	MessageQueue::instance() << "Ololo";

		if (_checked)
		{
			start_messaging->setText("STOP");
			message_timer->start();
		}
		else
		{
			out->close();
			start_messaging->setText("START");
			message_timer->stop();
		}
	});


	QLabel* lbl = new QLabel("STATUS");

	QObject::connect(&MessageQueue::instance(), &MessageQueue::locked, [lbl]() 
	{
		lbl->setStyleSheet("QLabel {background-color: red;}");
	});

	QObject::connect(&MessageQueue::instance(), &MessageQueue::unlocked, [lbl]()
	{
		lbl->setStyleSheet("QLabel {background-color: green;}");
	});

	//layout
	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->addWidget(lbl);
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
