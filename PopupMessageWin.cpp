#include "PopupMessageWin.h"

PopupMsgWindow::PopupMsgWindow() : QDialog(nullptr), m_forced_close(false), m_first_show(true), m_closed(false), m_close_time(DEF_CLOSE_TIME)
{
	m_close_timer = new QTimer(this);	//owns

	setWindowFlags(windowFlags() | Qt::Tool);
	setWindowOpacity(0);
	setAttribute(Qt::WA_DeleteOnClose);

	m_out = new QPlainTextEdit();
	m_out->setReadOnly(true);
	m_out->setStyleSheet("QPlainTextEdit {background-color: transparent;} ");
	m_out->setFrameStyle(QFrame::NoFrame);

	m_icon_lbl = new QLabel();

	QPushButton* btn = new QPushButton("OK", this);
	QObject::connect(btn, &QPushButton::clicked, this, &PopupMsgWindow::close);

	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->addStretch(1);
	bottom_layout->addWidget(btn);

	QHBoxLayout* central_layout = new QHBoxLayout();
	central_layout->addWidget(m_icon_lbl);
	central_layout->addWidget(m_out);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->addLayout(central_layout);
	main_layout->addStretch(1);
	main_layout->addLayout(bottom_layout);

	setFixedSize(DEF_WIN_WIDTH, DEF_WIN_HEIGTH);

	setLayout(main_layout);
}

void PopupMsgWindow::set_icon(const QIcon& _icon)
{
	m_icon_lbl->setPixmap(_icon.pixmap(_icon.actualSize(QSize(32, 32))));
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

	if (!m_forced_close)
	{
		_e->ignore();
		fade_out_close();
		return;
	}

	QDialog::closeEvent(_e);
}

void PopupMsgWindow::showEvent(QShowEvent* _e)
{
	if (m_first_show)
		start_close_timer();

	QDialog::showEvent(_e);
}

void PopupMsgWindow::move_up()
{
	QPoint begin_pos = pos();
	QPoint end_pos = QPoint(begin_pos.x(), begin_pos.y() - frameGeometry().height());

	ForcedStartAnimation* animation = new ForcedStartAnimation(this, "pos", begin_pos, end_pos, 500, false, this);
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
	ForcedStartAnimation* animation = new ForcedStartAnimation(this, "windowOpacity", 0.0, 1.0, 200, false, this);
	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_fade_in);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
	//QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity");
	//animation->setDuration(200);
	//animation->setStartValue(0.0);
	//animation->setEndValue(1.0);
	//animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupMsgWindow::fade_out()
{
	ForcedStartAnimation* animation = new ForcedStartAnimation(this, "windowOpacity", 1.0, 0.0, 200, false, this);
	QObject::connect(animation, &QPropertyAnimation::finished, this, &PopupMsgWindow::finish_fade_out);
	animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void PopupMsgWindow::fade_out_close()
{
	m_forced_close = true;
	QObject::connect(this, &PopupMsgWindow::finish_fade_out, &PopupMsgWindow::close);
	fade_out();
}

void PopupMsgWindow::set_message(const QString& _str)
{
	m_out->setPlainText(_str);
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
