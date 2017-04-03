#pragma once

#include <atomic>

#include <QtWidgets/QDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>

#include <QtCore/QTimer>
#include <QtCore/QPropertyAnimation>

#include <QtGui/QCloseEvent>

#include <memory>

#include "Message.h"

class PopupMsgWindow : public QDialog
{
	Q_OBJECT

	const size_t DEF_FADE_DURATION = 200;
	const size_t DEF_MOVE_DURATION = 400;

public:
	PopupMsgWindow();

	void set_base_widget(QWidget* _widget);
	void set_close_time(size_t _msec);
	void set_move_down_duration(size_t _msec);
	void set_move_up_duration(size_t _msec);
	void set_fade_in_duration(size_t _msec);
	void set_fade_out_duration(size_t _msec);

	virtual ~PopupMsgWindow();

protected:
	void closeEvent(QCloseEvent* _e) override;
	void showEvent(QShowEvent* _e) override;

public slots:
	void move_up();
	void move_down();
	void fade_out();
	virtual void set_message(const Message& _message);

signals:
	void begin_moving_down();
	void begin_moving_up();

	void finish_moving_down();
	void finish_moving_up();
	void finish_fade_out();
	void finish_fade_in();

private:
	void start_close_timer();
	void fade_in();

private:
	size_t m_close_time;
	QWidget* m_binded_widget;
	QTimer* m_close_timer;
	QLabel* m_icon_lbl;
	QPlainTextEdit* m_viewer;

	size_t m_move_up_duration, m_move_down_duration, m_fade_in_duration, m_fade_out_duration;
};