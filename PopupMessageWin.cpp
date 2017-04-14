#include "PopupMessageWin.h"

#include <QtCore/QStateMachine>
#include <QtCore/QHistoryState>

#include <QtCore/QPropertyAnimation>
#include "PrivatePopupMessageWin.h"

PopupMsgWindow::PopupMsgWindow() : QDialog(nullptr), p_impl(std::make_unique<PrivatePopupMessageWin>())
{
	QObject::connect(p_impl.get(), &PrivatePopupMessageWin::close_window, this, &PopupMsgWindow::close);
	QObject::connect(p_impl.get(), &PrivatePopupMessageWin::finish_fade_in, this, &PopupMsgWindow::finish_fade_in);
	QObject::connect(p_impl.get(), &PrivatePopupMessageWin::finish_fade_out, this, &PopupMsgWindow::finish_fade_out);

	p_impl->m_fade_in_duration = DEF_FADE_DURATION;
	p_impl->m_fade_out_duration = DEF_FADE_DURATION;
	p_impl->m_moving_duration = DEF_MOVE_DURATION;
	p_impl->m_moving = false;

	p_impl->m_close_timer = new QTimer(this);	//owns

	setWindowFlags(windowFlags() | Qt::Tool);
	setWindowOpacity(0);

	p_impl->m_viewer = new QPlainTextEdit();
	p_impl->m_viewer->setReadOnly(true);
	p_impl->m_viewer->setStyleSheet("QPlainTextEdit {background-color: transparent;} ");
	p_impl->m_viewer->setFrameStyle(QFrame::NoFrame);

	p_impl->m_icon_lbl = new QLabel();

	QPushButton* btn = new QPushButton("OK", this);
	QObject::connect(btn, &QPushButton::clicked, this, &PopupMsgWindow::close);

	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->addStretch(1);
	bottom_layout->addWidget(btn);

	QHBoxLayout* central_layout = new QHBoxLayout();
	central_layout->addWidget(p_impl->m_icon_lbl);
	central_layout->addWidget(p_impl->m_viewer);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->addLayout(central_layout);
	main_layout->addStretch(1);
	main_layout->addLayout(bottom_layout);

	setLayout(main_layout);

	p_impl->m_moving_animation = new QPropertyAnimation(this, "pos", this);
	QObject::connect(p_impl->m_moving_animation, &QPropertyAnimation::finished, this, [this]
	{
		p_impl->m_moving = false;
		emit finish_moving();
	});
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
	p_impl->m_close_time = _msec;
}

void PopupMsgWindow::closeEvent(QCloseEvent* _e)
{
	if (p_impl->m_close_timer->isActive())
		p_impl->m_close_timer->stop();

	QObject::connect(this, &PopupMsgWindow::finish_fade_out, [this, _e] 
	{
		QDialog::closeEvent(_e);
	});

	p_impl->fade_out(new QPropertyAnimation(this, "windowOpacity")); //delete when stopped
	_e->ignore();
}

void PopupMsgWindow::showEvent(QShowEvent* _e)
{
	p_impl->m_moving_diff = QPoint(0, frameGeometry().height());

	p_impl->start_close_timer();

	QDialog::showEvent(_e);

	p_impl->fade_in(new QPropertyAnimation(this, "windowOpacity"));	//delete when stopped
}

void PopupMsgWindow::set_moving_duration(size_t _msec)
{
	p_impl->m_moving_duration = _msec;
	p_impl->m_duration_diff = p_impl->m_moving_duration;
}

void PopupMsgWindow::set_fade_in_duration(size_t _msec)
{
	p_impl->m_fade_in_duration = _msec;
}

void PopupMsgWindow::set_fade_out_duration(size_t _msec)
{
	p_impl->m_fade_out_duration = _msec;
}

bool PopupMsgWindow::moving() const
{
	return p_impl->m_moving;
}

void PopupMsgWindow::move_up()
{
	p_impl->m_duration_diff = p_impl->m_moving_duration;
	p_impl->m_moving_diff = QPoint(0, -frameGeometry().height());
	p_impl->start_moving_animation(pos());
	emit start_moving_up();
}

void PopupMsgWindow::move_down()
{
	p_impl->m_duration_diff = p_impl->m_moving_duration;
	p_impl->m_moving_diff = QPoint(0, frameGeometry().height());
	p_impl->start_moving_animation(pos());
	emit start_moving_down();
}

void PopupMsgWindow::pause()
{
	p_impl->m_moving_animation->stop();
	p_impl->m_moving_diff = p_impl->m_moving_animation->endValue().toPoint() - p_impl->m_moving_animation->currentValue().toPoint();
	p_impl->m_duration_diff = p_impl->m_moving_duration - p_impl->m_moving_animation->currentTime();
	emit paused();
}

void PopupMsgWindow::resume()
{
	p_impl->start_moving_animation(pos());
	emit resumed();
}

void PopupMsgWindow::set_message(const Message& _message)
{
	setWindowTitle(_message.title());
	p_impl->m_icon_lbl->setPixmap(QPixmap::fromImage(_message.image()));
	p_impl->m_viewer->appendHtml(_message.text());
}