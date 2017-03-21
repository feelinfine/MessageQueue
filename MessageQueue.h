#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QLayout>

#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>

#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>

#include <QtGui/QCloseEvent>
#include <QtGui/QShowEvent>

#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

#include "PopupMessageWin.h"

enum class MsgType
{
	INFO,
	WARNING,
	ERROR
};

class MessageQueue
{
	static const size_t DEF_QUEUE_SIZE = 3;
	static const size_t DEF_CLOSE_TIMER = 10000;	//msec

public:
	MessageQueue(QWidget* _main_widget, size_t _close_time = DEF_CLOSE_TIMER, size_t _queue_size = DEF_QUEUE_SIZE) : m_queue_size(_queue_size), m_close_timer_value(_close_time), m_main_widget(_main_widget), counter(0), m_finished(false)
	{
		m_thr = std::make_unique<std::thread>([this]()
		{
			do
			{
				m_rm.lock();
				std::remove_if(m_queue.begin(), m_queue.end(), [](const std::shared_ptr<PopupMsgWindow>& _win) -> bool
				{
					return _win->closed();
				});
				m_rm.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(300));

			} while (!m_finished);
		
		});
	}

	MessageQueue() : MessageQueue(nullptr)
	{
	}

	void set_main_widget(QWidget* _main_widget)	//no owns
	{
		m_main_widget = _main_widget;
	}

	void set_close_timer(size_t _msec)
	{
		m_close_timer_value = _msec;
	}

	void set_max_queue_size(size_t _size)
	{
		m_queue_size = _size;
	}

	void post_message(MsgType _mtype, const QString& _str)
	{
		std::shared_ptr<PopupMsgWindow> win = std::make_shared<PopupMsgWindow>(m_main_widget, m_close_timer_value);

		m_rm.lock();
		for (auto& it : m_queue)
			it->move_up(win->height() + win->style()->pixelMetric(QStyle::PM_TitleBarHeight));
			
		m_queue.push_back(win);
		m_rm.unlock();

		win->show();
	}

private:
	size_t m_queue_size;
	size_t m_close_timer_value;
	QWidget* m_main_widget;

	std::vector<std::shared_ptr<PopupMsgWindow> > m_queue;
	PopupMsgWindow* m_last_win;

	size_t counter;

	std::unique_ptr<std::thread> m_thr;
	std::atomic<bool> m_finished;

	std::recursive_mutex m_rm;
};
