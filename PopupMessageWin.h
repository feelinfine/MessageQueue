#pragma once

#include <atomic>

#include <QtWidgets/QDialog>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>

#include <QtCore/QTimer>
#include <QtCore/QPropertyAnimation>

#include <QtGui/QCloseEvent>

class PopupMsgWindow : public QDialog
{
	Q_OBJECT

	const size_t WIN_WIDTH = 300;
	const size_t WIN_HEIGTH = 150;

public:
	PopupMsgWindow(QWidget* _binded_widget, size_t _close_time = 0) : QDialog(nullptr), m_forced_close(false), m_first_show(true), m_closed(false)
	{
		setWindowFlags(windowFlags() | Qt::Tool);
		setWindowOpacity(0);

		bind_to_widget(_binded_widget);
		set_close_timer(_close_time);

		QPropertyAnimation* fade_in = new QPropertyAnimation(this, "windowOpacity");
		fade_in->setDuration(200);
		fade_in->setStartValue(0.0);
		fade_in->setEndValue(1.0);
		fade_in->start(QAbstractAnimation::DeleteWhenStopped);

		QPushButton* btn = new QPushButton("OK", this);
		QObject::connect(btn, &QPushButton::clicked, this, &PopupMsgWindow::fade_out_close);

		QHBoxLayout* bottom_layout = new QHBoxLayout();
		bottom_layout->addStretch(1);
		bottom_layout->addWidget(btn);

		QVBoxLayout* main_layout = new QVBoxLayout();
		main_layout->addStretch(1);
		main_layout->addLayout(bottom_layout);

		setLayout(main_layout);
	}

	~PopupMsgWindow()
	{
		int d = 0;
	}

	void move_up(size_t _px)
	{
		QPoint current_pos = pos();
		QPropertyAnimation* moving = new QPropertyAnimation(this, "pos");
		moving->setDuration(200);
		moving->setStartValue(current_pos);
		moving->setEndValue(QPoint(current_pos.x(), current_pos.y() - _px));
		moving->start(QAbstractAnimation::DeleteWhenStopped);
	}

	void bind_to_widget(QWidget* _widget)
	{
		size_t x = 0, y = 0, w = WIN_WIDTH, h = WIN_HEIGTH;
		if (_widget != nullptr)
		{
			x = _widget->x() + _widget->width() - WIN_WIDTH;
			y = _widget->y() + style()->pixelMetric(QStyle::PM_TitleBarHeight) + _widget->rect().height() - WIN_HEIGTH;
		}

		setGeometry(x, y, WIN_WIDTH, WIN_HEIGTH);
	}

	void set_close_timer(size_t _msec)
	{
		m_close_time = _msec;
	}

	bool closed() const
	{
		return m_closed;
	}

	void closeEvent(QCloseEvent* _e) override
	{
		if (!m_forced_close)
		{
			fade_out_close();
			_e->ignore();
			return;
		}
		QDialog::closeEvent(_e);
		m_closed = true;
	}

	void showEvent(QShowEvent* _e) override
	{
		if (m_first_show)
			start_close_timer();

		QDialog::showEvent(_e);
	}

private:
	void start_close_timer()
	{
		if (m_close_time)
			QTimer::singleShot(m_close_time, [this]() {fade_out_close();});
	}

	private slots:
	void fade_out_close()
	{
		QPropertyAnimation *fade_out = new QPropertyAnimation(this, "windowOpacity");
		fade_out->setDuration(200);
		fade_out->setStartValue(1.0);
		fade_out->setEndValue(0.0);
		QObject::connect(fade_out, &QPropertyAnimation::finished, this, &PopupMsgWindow::close);
		m_forced_close = true;
		fade_out->start(QAbstractAnimation::DeleteWhenStopped);
	}

private:
	bool m_forced_close;
	bool m_first_show;
	size_t m_close_time;
	QWidget* m_binded_widget;
	std::atomic<bool> m_closed;
};