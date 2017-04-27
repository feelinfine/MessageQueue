#pragma once

#include <QtWidgets/QDialog>
#include <QtGui/QCloseEvent>

#include <memory>

#include "Message.h"

class PopupMsgWindow : public QDialog
{
	Q_OBJECT

	const size_t DEF_FADE_DURATION = 200;
	const size_t DEF_MOVE_DURATION = 900;

public:
	PopupMsgWindow();

	void set_base_widget(QWidget* _widget);
	void set_close_time(size_t _msec);
	void set_moving_duration(size_t _msec);
	void set_fade_in_duration(size_t _msec);
	void set_fade_out_duration(size_t _msec);
	bool moving() const;

	virtual ~PopupMsgWindow();

protected:
	void closeEvent(QCloseEvent* _e) override;
	void showEvent(QShowEvent* _e) override;

public slots:
	void move_up();
	void move_down();
	void pause();
	void resume();
	virtual void set_message(const Message& _message);

signals:
	void start_moving_down();
	void start_moving_up();
	void finish_moving();
	void finish_fade_out();
	void finish_fade_in();
	void paused();
	void resumed();

private:
	class PrivatePopupMessageWin;
	std::unique_ptr<PrivatePopupMessageWin> p_impl;
};