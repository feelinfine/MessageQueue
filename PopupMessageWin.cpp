#include "PopupMessageWin.h"

PopupMsgWindow::PopupMsgWindow() : 
	QDialog(nullptr), 
	m_fade_in_duration(DEF_FADE_DURATION), 
	m_fade_out_duration(DEF_FADE_DURATION), 
	m_moving_duration(DEF_MOVE_DURATION)
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

	// machine

	m_machine = new QStateMachine(this);

	QState* fixed_state = new QState(m_machine);
	QState* moving_up_state = new QState(m_machine);
	QState* moving_down_state = new QState(m_machine);
	QState* paused_state = new QState(m_machine);

	fixed_state->addTransition(this, &PopupMsgWindow::start_moving_up, moving_up_state);
	fixed_state->addTransition(this, &PopupMsgWindow::start_moving_down, moving_down_state);
	moving_up_state->addTransition(this, &PopupMsgWindow::finish_moving_up, fixed_state);
	moving_down_state->addTransition(this, &PopupMsgWindow::finish_moving_down, fixed_state);
	moving_up_state->addTransition(this, &PopupMsgWindow::paused, paused_state);
	moving_down_state->addTransition(this, &PopupMsgWindow::paused, paused_state);
//	paused_state->addTransition(this, &PopupMsgWindow::resumed, paused_state);

	QPropertyAnimation* moving_animation = new QPropertyAnimation(this, "pos", this);

	QObject::connect(fixed_state, &QState::entered, this, [this]()
	{
		m_duration_diff = m_moving_duration;
		m_moving_diff.setX(0);
		m_moving_diff.setY(frameGeometry().height());
	});

	QObject::connect(moving_up_state, &QState::entered, this, [this, moving_animation]()
	{
		moving_animation->setStartValue(pos());
		moving_animation->setEndValue(pos() - m_moving_diff);
		moving_animation->setDuration(m_duration_diff);

		QObject::connect(moving_animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_moving_up);
		moving_animation->start();
	});

	QObject::connect(moving_down_state, &QState::entered, this, [this, moving_animation]()
	{
		moving_animation->setStartValue(pos());
		moving_animation->setEndValue(pos() + m_moving_diff);
		moving_animation->setDuration(m_duration_diff);

		QObject::connect(moving_animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_moving_down);
		moving_animation->start();
	});

	QObject::connect(paused_state, &QState::entered, this, [this, moving_animation]()
	{
		if (moving_animation)
		{
			moving_animation->stop();
			m_moving_diff = moving_animation->endValue().toPoint() - moving_animation->currentValue().toPoint();
			m_duration_diff = m_moving_duration - moving_animation->currentTime();
		}
	});

	m_machine->setInitialState(fixed_state);
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
	if (m_machine->isRunning())
		m_machine->stop();

	m_machine->start();

	start_close_timer();

	QDialog::showEvent(_e);

	fade_in();
}

void PopupMsgWindow::set_moving_duration(size_t _msec)
{
	m_moving_duration = _msec;
}

void PopupMsgWindow::set_fade_in_duration(size_t _msec)
{
	m_fade_in_duration = _msec;
}

void PopupMsgWindow::set_fade_out_duration(size_t _msec)
{
	m_fade_out_duration = _msec;
}

void PopupMsgWindow::move_up()
{
	emit start_moving_up();
}

void PopupMsgWindow::move_down()
{
	emit start_moving_down();
}

void PopupMsgWindow::pause()
{
	emit paused();
}

void PopupMsgWindow::resume()
{
	emit resumed();
}

void PopupMsgWindow::fade_in()
{
	QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
	animation->setStartValue(0.0);
	animation->setEndValue(1.0);
	animation->setDuration(m_fade_in_duration);

	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_fade_in);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupMsgWindow::fade_out()
{
	QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity", this);
	animation->setStartValue(1.0);
	animation->setEndValue(0.0);
	animation->setDuration(m_fade_out_duration);

	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_fade_out);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupMsgWindow::set_message(const Message& _message)
{
	setWindowTitle(_message.title());
	m_icon_lbl->setPixmap(QPixmap::fromImage(_message.image()));
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