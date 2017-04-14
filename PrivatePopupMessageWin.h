#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtCore/QTimer>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QCloseEvent>

#include "PopupMessageWin.h"

class PopupMsgWindow::PrivatePopupMessageWin : public QObject
{
	Q_OBJECT

public:
	//behavior
	void start_close_timer()
	{
		if (m_close_time)
		{
			m_close_timer->setInterval(m_close_time);
			m_close_timer->setSingleShot(true);
			QObject::connect(m_close_timer, &QTimer::timeout, this, &PrivatePopupMessageWin::close_window);
			m_close_timer->start();
		}
	};

	void fade_in(QPropertyAnimation* _animation)
	{
		_animation->setStartValue(0.0);
		_animation->setEndValue(1.0);
		_animation->setDuration(m_fade_in_duration);

		QObject::connect(_animation, &QPropertyAnimation::finished, this, &PrivatePopupMessageWin::finish_fade_in);
		_animation->start(QAbstractAnimation::DeleteWhenStopped);
	}

	void fade_out(QPropertyAnimation* _animation)
	{
		_animation->setStartValue(1.0);
		_animation->setEndValue(0.0);
		_animation->setDuration(m_fade_out_duration);

		QObject::connect(_animation, &QPropertyAnimation::finished, this, &PrivatePopupMessageWin::finish_fade_out);
		_animation->start(QAbstractAnimation::DeleteWhenStopped);
	};

	void start_moving_animation(const QPoint& _current_pos)
	{
		m_moving = true;
		m_moving_animation->setStartValue(_current_pos);
		m_moving_animation->setEndValue(_current_pos + m_moving_diff);
		m_moving_animation->setDuration(m_duration_diff);
		m_moving_animation->start();
	};

	//data
	size_t m_close_time;
	QWidget* m_binded_widget;
	QTimer* m_close_timer;
	QLabel* m_icon_lbl;
	QPlainTextEdit* m_viewer;
	size_t m_moving_duration, m_fade_in_duration, m_fade_out_duration;
	QPoint m_moving_diff;
	size_t m_duration_diff;
	QPropertyAnimation* m_moving_animation;
	bool m_moving;

signals:
	void close_window();
	void finish_fade_in();
	void finish_fade_out();
};