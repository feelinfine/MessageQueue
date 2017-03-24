#pragma once

#include <list>
#include <functional>
#include <queue>
#include <mutex>

#include "PopupMessageWin.h"
#include "Message.h"

class MessageQueue final : public QObject
{
	Q_OBJECT

	static const size_t DEF_QUEUE_SIZE = 4;
	static const size_t DEF_CLOSE_TIMER = 4000;	//msec

public:
	static MessageQueue& instance(QWidget* _base_widget = nullptr, size_t _close_time = DEF_CLOSE_TIMER, size_t _queue_size = DEF_QUEUE_SIZE)
	{
		static MessageQueue self(_base_widget, _close_time, _queue_size);	//C++11 thread safe, if C++03 use double-check-locking
		return self;
	}

	void set_base_widget(QWidget* _base_widget)	//no owns
	{
		m_base_widget = _base_widget;
	}

	void set_close_timer(size_t _msec)
	{
		m_close_timer_value = _msec;
	}

	void set_limit_size(size_t _size)
	{
		m_queue_size = _size;
	}

	bool try_push_message(const Message& _message)
	{
		std::unique_lock<std::recursive_mutex> guard(m_rm, std::try_to_lock);

		if (guard.owns_lock())
		{
			m_waiting_messages.push(_message);
			return true;
		}

		return false;
	}

	void push_message(const Message& _message)
	{
		std::lock_guard<std::recursive_mutex> guard(m_rm);
		m_waiting_messages.push(_message);
	}

	const MessageQueue& operator << (const QString& _info)
	{
		push_message(Message(MsgType::INFO, _info));
		return *this;
	}

	const MessageQueue& operator << (const Message& _msg)
	{
		push_message(_msg);
		return *this;
	}

	~MessageQueue()
	{
	}

private:
	MessageQueue(const MessageQueue&) = delete;
	MessageQueue& operator= (const MessageQueue&) = delete;

	MessageQueue(QWidget* _main_widget, size_t _close_time = DEF_CLOSE_TIMER, size_t _queue_size = DEF_QUEUE_SIZE) : m_queue_size(_queue_size), m_close_timer_value(_close_time), m_base_widget(_main_widget)
	{
		m_processing_interval = 200;
		m_maximum_processed_at_once = 3;

		QTimer* timer = new QTimer(this);
		timer->setInterval(m_processing_interval);
		QObject::connect(timer, &QTimer::timeout, this, &MessageQueue::process_messages);
		timer->start();
	}

	void show_message(MsgType _mtype, const QString& _str)
	{
		PopupMsgWindow* win = new PopupMsgWindow();	//WA_DeleteOnClose
		win->set_base_widget(m_base_widget);
		win->set_close_time(m_close_timer_value);
		win->set_message(_str);

		QObject::connect(win, &PopupMsgWindow::finish_fade_out, this, std::bind(&MessageQueue::remove_from_active_list, this, win));

		add_to_active_list(win);

		win->fade_in();
		win->show();
	}

	void add_to_active_list(PopupMsgWindow* _win)
	{
		if (!m_active_list.empty())
		{
			QObject::connect(_win, &PopupMsgWindow::begin_moving_up, m_active_list.back(), &PopupMsgWindow::move_up);
			m_active_list.back()->move_up();
		}

		m_active_list.push_back(_win);
	}

	void remove_from_active_list(PopupMsgWindow* _win)
	{
		std::list<PopupMsgWindow*>::iterator it = std::find(m_active_list.begin(), m_active_list.end(), _win);

		if (it == m_active_list.end())
			return;

		PopupMsgWindow* cur_win = *it;
		if (cur_win != m_active_list.back() && cur_win != m_active_list.front())
		{
			PopupMsgWindow* upper = *std::next(it);
			PopupMsgWindow* lower = *std::prev(it);
			upper->move_down();

			QObject::connect(upper, &PopupMsgWindow::finish_moving_down, [lower, upper]()
			{
				QObject::connect(lower, &PopupMsgWindow::begin_moving_up, upper, &PopupMsgWindow::move_up);
			});
		}

		m_active_list.remove(*it);
	}

private slots:
	void process_messages()
	{
		size_t counter = m_maximum_processed_at_once;

		while (counter)
		{
			if (m_waiting_messages.empty())
				break;

			Message msg = m_waiting_messages.front();
			show_message(msg.type(), msg.text());
			m_waiting_messages.pop();

			counter--;
		}
	}

private:
	size_t m_queue_size;
	size_t m_close_timer_value;

	size_t m_processing_interval;
	size_t m_maximum_processed_at_once;

	QWidget* m_base_widget;

	std::list<PopupMsgWindow*> m_active_list;
	std::queue<Message> m_waiting_messages;

	std::recursive_mutex m_rm;
};