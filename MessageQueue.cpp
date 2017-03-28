
#include "MessageQueue.h"

MessageQueue& MessageQueue::instance(QWidget* _base_widget /*= nullptr*/, size_t _close_time /*= DEF_CLOSE_TIMER*/, size_t _queue_size /*= DEF_QUEUE_SIZE*/)
{
	static MessageQueue self(_base_widget, _close_time, _queue_size);	//C++11 thread safe, if C++03 use double-check-locking
	return self;
}

void MessageQueue::set_base_widget(QWidget* _base_widget) //no owns
{
	m_base_widget = _base_widget;
}

void MessageQueue::set_close_timer(size_t _msec)
{
	m_close_timer_value = _msec;
}

void MessageQueue::set_limit_size(size_t _size)
{
	m_queue_size = _size;
}

bool MessageQueue::try_push_message(const Message& _message)
{
	std::unique_lock<std::recursive_mutex> guard(m_rm, std::try_to_lock);

	if (guard.owns_lock())
	{
		m_waiting_messages.push(_message);
		return true;
	}

	return false;
}

void MessageQueue::push_message(const Message& _message)
{
	std::lock_guard<std::recursive_mutex> guard(m_rm);
	m_waiting_messages.push(_message);
}

const MessageQueue& MessageQueue::operator<<(const QString& _info)
{
	push_message(Message(MsgType::INFO, _info));
	return *this;
}

const MessageQueue& MessageQueue::operator<<(const Message& _msg)
{
	push_message(_msg);
	return *this;
}

MessageQueue::~MessageQueue()
{
}

MessageQueue::MessageQueue(QWidget* _main_widget, size_t _close_time /*= DEF_CLOSE_TIMER*/, size_t _queue_size /*= DEF_QUEUE_SIZE*/) :
	m_queue_size(_queue_size), 
	m_close_timer_value(_close_time), 
	m_base_widget(_main_widget),
	m_processing(true)
{
	m_processing_interval = 50;

	QTimer* timer = new QTimer(this);
	timer->setInterval(m_processing_interval);
	QObject::connect(timer, &QTimer::timeout, this, &MessageQueue::process_messages);
	timer->start();
}

void MessageQueue::show_message(MsgType _mtype, const QString& _str)
{
	PopupMsgWindow* win = new PopupMsgWindow();	//WA_DeleteOnClose
	win->set_base_widget(m_base_widget);
	win->set_close_time(m_close_timer_value);
	win->set_message(_str);

	switch (_mtype)
	{
	case MsgType::INFO:		win->set_icon(win->style()->standardIcon(QStyle::SP_MessageBoxInformation));	break;
	case MsgType::WARNING:	win->set_icon(win->style()->standardIcon(QStyle::SP_MessageBoxWarning));		break;
	case MsgType::ERROR:	win->set_icon(win->style()->standardIcon(QStyle::SP_MessageBoxCritical));		break;
	}

	add_to_active_list(win);

	win->fade_in();
	win->show();
}

void MessageQueue::add_to_active_list(PopupMsgWindow* _win)
{
	lock_processing();

	QObject::connect(_win, &PopupMsgWindow::finish_fade_out, this, std::bind(&MessageQueue::remove_from_active_list, this, _win));		//remove after close

	if (m_active_list.empty())		//first message
	{
		m_active_list.push_front(_win);
		unlock_processing();
	}
	else //there are already messages
	{
		//wait until first finish moving up
		static QMetaObject::Connection connection;
		connection = QObject::connect(m_active_list.front(), &PopupMsgWindow::finish_moving_up, this, [this, _win]()
		{
			m_active_list.push_front(_win);
			QObject::disconnect(connection);
			unlock_processing();
		});

		for (auto& it : m_active_list)
			it->move_up();
	}
}

void MessageQueue::remove_from_active_list(PopupMsgWindow* _win)
{
	lock_processing();

	std::list<PopupMsgWindow*>::iterator cwin = std::find(m_active_list.begin(), m_active_list.end(), _win);

	if (cwin == m_active_list.end())
	{
		unlock_processing();
		return;
	}

	if (*cwin != m_active_list.back())
	{
		//wait until back finish moving down
		PopupMsgWindow* upper = *std::next(cwin);
		static QMetaObject::Connection connection;
		connection = QObject::connect(upper, &PopupMsgWindow::finish_moving_down, this, [this]()
		{
			QObject::disconnect(connection);
			unlock_processing();
		});

		for (auto it = std::next(cwin); it!= m_active_list.end(); ++it)
			(*it)->move_down();
	}
	else
		unlock_processing();

	m_active_list.remove(*cwin);
}

void MessageQueue::process_messages()
{
	if (m_waiting_messages.empty() || !m_processing)
		return;

	std::lock_guard<std::recursive_mutex> guard(m_rm);

	Message msg = m_waiting_messages.front();
	show_message(msg.type(), msg.text());
	m_waiting_messages.pop();
}

void MessageQueue::lock_processing()
{
	m_processing = false;
}

void MessageQueue::unlock_processing()
{
	m_processing = true;
}
