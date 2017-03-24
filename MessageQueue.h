#pragma once

#include <list>
#include <functional>

#include "PopupMessageWin.h"
#include "Message.h"

class MessageQueue final : public QObject
{
	Q_OBJECT

	static const size_t DEF_QUEUE_SIZE = 3;
	static const size_t DEF_CLOSE_TIMER = 4000;	//msec

public:
	static MessageQueue& instance(QWidget* _base_widget = nullptr, size_t _close_time = DEF_CLOSE_TIMER, size_t _queue_size = DEF_QUEUE_SIZE)
	{
		static MessageQueue self(_base_widget, _close_time, _queue_size);
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

	void post_message(MsgType _mtype, const QString& _str)
	{
		PopupMsgWindow* win = new PopupMsgWindow();	//WA_DeleteOnClose
		win->set_base_widget(m_base_widget);
		win->set_close_time(m_close_timer_value);
		win->set_message(_str);

		QObject::connect(win, &PopupMsgWindow::finish_fade_out, this, std::bind(&MessageQueue::remove_from_queue, this, win));
		
		add_to_queue(win);
	}

	const MessageQueue& operator << (const QString& _info)
	{
		post_message(MsgType::INFO, _info);
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
	}

	void remove_from_queue(PopupMsgWindow* _win)
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

	void add_to_queue(PopupMsgWindow* _win)
	{
		if (!m_active_list.empty())
		{
			QObject::connect(_win, &PopupMsgWindow::begin_moving_up, m_active_list.front(), &PopupMsgWindow::move_up);
			m_active_list.front()->move_up();
		}

		m_active_list.push_front(_win);

		_win->fade_in();
		_win->show();

		if (m_active_list.size() > m_queue_size)
			remove_from_queue(m_active_list.back());
	}

	size_t m_queue_size;
	size_t m_close_timer_value;
	QWidget* m_base_widget;

	std::list<PopupMsgWindow*> m_active_list;
};
