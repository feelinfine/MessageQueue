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

class ForcedStartAnimation : public QPropertyAnimation
{
public:
	ForcedStartAnimation(QObject* _target, const QByteArray& _property, const QVariant& _start_val, const QVariant& _end_val, size_t _duration, bool _start, QObject* _parent = nullptr) : QPropertyAnimation(_target, _property, _parent)
	{
		setDuration(_duration);
		setStartValue(_start_val);
		setEndValue(_end_val);
		if (_start)
			start(QAbstractAnimation::DeleteWhenStopped);
	}
};

class PopupMsgWindow : public QDialog
{
	Q_OBJECT

	const size_t DEF_WIN_WIDTH = 300;
	const size_t DEF_WIN_HEIGTH = 150;
	const size_t DEF_CLOSE_TIME = 5000;

public:
	PopupMsgWindow();

	void set_icon(const QIcon& _icon);
	virtual ~PopupMsgWindow();
	void set_base_widget(QWidget* _widget);
	void set_close_time(size_t _msec);
	void closeEvent(QCloseEvent* _e) override;
	void showEvent(QShowEvent* _e) override;

public slots:
	void move_up();
	void move_down();
	void fade_in();
	void fade_out();
	void fade_out_close();
	virtual void set_message(const QString& _str);;

signals:
	void begin_moving_down();
	void finish_moving_down();
	void begin_moving_up();
	void finish_moving_up();
	void finish_fade_out();
	void finish_fade_in();

private:
	void start_close_timer();

private:
	bool m_forced_close;
	bool m_first_show;
	size_t m_close_time;
	QWidget* m_binded_widget;
	std::atomic<bool> m_closed;
	QTimer* m_close_timer;

	QLabel* m_icon_lbl;
	QPlainTextEdit* m_out;
};