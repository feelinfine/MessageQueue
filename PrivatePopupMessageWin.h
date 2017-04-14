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

struct PrivatePopupMessageWin
{
	void start_close_timer();
	void fade_in();
	void fade_out();
	void start_animation();

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
};