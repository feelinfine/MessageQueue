#include "PopupMessageWin.h"

PopupMsgWindow::PopupMsgWindow() : QDialog(nullptr), m_first_show(true), m_closed(false)
{
	m_close_timer = new QTimer(this);	//owns

	setWindowFlags(windowFlags() | Qt::Tool);
	setWindowOpacity(0);

	m_viewer = new QPlainTextEdit();
	m_viewer->setReadOnly(true);
	m_viewer->setStyleSheet("QPlainTextEdit {background-color: transparent;} ");
	m_viewer->setFrameStyle(QFrame::NoFrame);

	m_icon_lbl = new QLabel();

	QPushButton* btn = new QPushButton("OK", this);
	QObject::connect(btn, &QPushButton::clicked, this, &PopupMsgWindow::close);

	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->addStretch(1);
	bottom_layout->addWidget(btn);

	QHBoxLayout* central_layout = new QHBoxLayout();
	central_layout->addWidget(m_icon_lbl);
	central_layout->addWidget(m_viewer);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->addLayout(central_layout);
	main_layout->addStretch(1);
	main_layout->addLayout(bottom_layout);

	setLayout(main_layout);
}

PopupMsgWindow::~PopupMsgWindow()
{
}

void PopupMsgWindow::set_base_widget(QWidget* _widget)
{
	setParent(_widget);
	setWindowFlags(windowFlags() | Qt::Tool);
	size_t x = 0, y = 0;
	if (_widget != nullptr)
	{
		x = _widget->x() + _widget->width() - width();
		y = _widget->y() + style()->pixelMetric(QStyle::PM_TitleBarHeight) + _widget->rect().height() - height();
	}

	setGeometry(x, y, width(), height());
}

void PopupMsgWindow::set_close_time(size_t _msec)
{
	m_close_time = _msec;
}

void PopupMsgWindow::closeEvent(QCloseEvent* _e)
{
	if (m_close_timer->isActive())
		m_close_timer->stop();

	QObject::connect(this, &PopupMsgWindow::finish_fade_out, [this, _e] 
	{
		QDialog::closeEvent(_e);
	});

	fade_out();
	_e->ignore();
}

void PopupMsgWindow::showEvent(QShowEvent* _e)
{
	if (m_first_show)
		start_close_timer();

	QDialog::showEvent(_e);

	fade_in();
}

void PopupMsgWindow::move_up()
{
	QPoint begin_pos = pos();
	QPoint end_pos = QPoint(begin_pos.x(), begin_pos.y() - frameGeometry().height());

	QPropertyAnimation* animation = new QPropertyAnimation(this, "pos", this);
	animation->setStartValue(begin_pos);
	animation->setEndValue(end_pos);
	animation->setDuration(500);
	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_moving_up);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
	emit begin_moving_up();
}

void PopupMsgWindow::move_down()
{
	QPoint begin_pos = pos();
	QPoint end_pos = QPoint(begin_pos.x(), begin_pos.y() + frameGeometry().height());

	QPropertyAnimation* animation = new QPropertyAnimation(this, "pos", this);
	animation->setDuration(500);
	animation->setStartValue(begin_pos);
	animation->setEndValue(end_pos);

	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_moving_down);

	animation->start(QAbstractAnimation::DeleteWhenStopped);
	emit begin_moving_down();
}

void PopupMsgWindow::fade_in()
{
	QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setDuration(200);

	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_fade_in);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupMsgWindow::fade_out()
{
	QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
	animation->setStartValue(1.0);
	animation->setEndValue(0.0);
	animation->setDuration(200);

	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_fade_out);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupMsgWindow::set_message(const Message& _message)
{
	setWindowTitle(_message.title());
	m_icon_lbl->setPixmap(_message.pixmap());
	m_viewer->appendHtml(_message.text());
}

void PopupMsgWindow::start_close_timer()
{
	if (m_close_time)
	{
		m_close_timer->setInterval(m_close_time);
		m_close_timer->setSingleShot(true);
		QObject::connect(m_close_timer, &QTimer::timeout, this, &PopupMsgWindow::close);
		m_close_timer->start();
	}
}
