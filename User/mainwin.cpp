#include "mainwin.h"

#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDoubleSpinBox>

#include <QtCore/QTimer>
#include <QtCore/QThread>

#include "QtWidgets/QLCDNumber"

MainWin::MainWin(QWidget *parent) : QMainWindow(parent)
{
	//default
	const size_t WIN_W = 640;
	const size_t WIN_H = 480;

	const size_t TIMER_INTERVAL = 1000; //msec

	//timer
	MessageQueue& message_queue = MessageQueue::instance();
	message_queue.set_base_widget(this);

	out = new QFile("log.txt");
	bool res = out->open(QIODevice::WriteOnly | QIODevice::Text);

	message_queue.set_output_device(out);

	//Thread 1
	QDoubleSpinBox* thr1_time_interval = new QDoubleSpinBox();
	thr1_time_interval->setRange(100, 10000);

	QPushButton* thr1_start_messaging = new QPushButton("START", this);
	thr1_start_messaging->setCheckable(true);

	QTimer* thr1_message_timer = new QTimer();

	Worker* info_wrk = new Worker();

	QObject::connect(thr1_message_timer, &QTimer::timeout, info_wrk, std::bind(&Worker::send_one, info_wrk, InfoMessage("<font color=blue>From thread 1:</font> ")), Qt::QueuedConnection);

	QObject::connect(thr1_start_messaging, &QPushButton::clicked, [thr1_start_messaging, thr1_time_interval, thr1_message_timer](bool _checked)
	{
		if (_checked)
		{
			thr1_message_timer->setInterval(thr1_time_interval->value());
			thr1_start_messaging->setText("STOP");
			thr1_message_timer->start();
		}
		else
		{
			thr1_start_messaging->setText("START");
			thr1_message_timer->stop();
		}
	});


	QHBoxLayout* thr1_layout = new QHBoxLayout;
	thr1_layout->addWidget(new QLabel("Thread 1"));
	thr1_layout->addWidget(thr1_start_messaging);
	thr1_layout->addWidget(thr1_time_interval);
	thr1_layout->addStretch(1);

	//Thread 2
	QDoubleSpinBox* thr2_time_interval = new QDoubleSpinBox();
	thr2_time_interval->setRange(100, 10000);

	QPushButton* thr2_start_messaging = new QPushButton("START", this);
	thr2_start_messaging->setCheckable(true);

	QTimer* thr2_message_timer = new QTimer();
	QThread* thr2 = new QThread(this);

	Worker* warning_wrk = new Worker();
	warning_wrk->moveToThread(thr2);
	thr2->start();

	QObject::connect(thr2_message_timer, &QTimer::timeout, warning_wrk, std::bind(&Worker::send_one, warning_wrk, WarnMessage("<font color=green>From thread 2:</font> ")), Qt::QueuedConnection);

	QObject::connect(thr2_start_messaging, &QPushButton::clicked, [thr2_start_messaging, thr2_time_interval, thr2_message_timer](bool _checked)
	{
		if (_checked)
		{
			thr2_message_timer->setInterval(thr2_time_interval->value());
			thr2_start_messaging->setText("STOP");
			thr2_message_timer->start();
		}
		else
		{
			thr2_start_messaging->setText("START");
			thr2_message_timer->stop();
		}
	});

	QHBoxLayout* thr2_layout = new QHBoxLayout;
	thr2_layout->addWidget(new QLabel("Thread 2"));
	thr2_layout->addWidget(thr2_start_messaging);
	thr2_layout->addWidget(thr2_time_interval);
	thr2_layout->addStretch(1);

	//Thread 3
	QPushButton* thr3_start_messaging = new QPushButton("Push 10 messages");
	QThread* thr3 = new QThread(this);

	Worker* error_wrk = new Worker();
	error_wrk->moveToThread(thr3);
	thr3->start();

	QObject::connect(thr3_start_messaging, &QPushButton::clicked, error_wrk, std::bind(&Worker::send_ten, error_wrk, ErrorMessage("<font color=red>From thread 3:</font> ")), Qt::QueuedConnection);

	QHBoxLayout* thr3_layout = new QHBoxLayout;
	thr3_layout->addWidget(new QLabel("Thread 3"));
	thr3_layout->addWidget(thr3_start_messaging);
	thr3_layout->addStretch(1);

	//layout
	QLabel* lbl_counter = new QLabel("0");

	QObject::connect(&message_queue, &MessageQueue::waiting_list_size_changed, this, [lbl_counter](size_t _size)
	{
		lbl_counter->setText(QString::number(_size));
	});

	QHBoxLayout* message_counter_layout = new QHBoxLayout;
	message_counter_layout->addWidget(new QLabel("Queued messages: "));
	message_counter_layout->addWidget(lbl_counter);
	message_counter_layout->addStretch(1);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->addLayout(message_counter_layout);
	main_layout->addStretch(1);
	main_layout->addLayout(thr1_layout);
	main_layout->addLayout(thr2_layout);
	main_layout->addLayout(thr3_layout);

	QWidget* main_widget = new QWidget();
	main_widget->setLayout(main_layout);

	setCentralWidget(main_widget);

	resize(WIN_W, WIN_H);
}

MainWin::~MainWin()
{
	out->close();
}