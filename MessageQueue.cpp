
#include "MessageQueue.h"
#include "DefaultMessages.h"

#include <QtCore/QStateMachine>

MessageQueue& MessageQueue::instance()
{
	static MessageQueue self;	//C++11 thread safe, if C++03 use double-check-locking
	return self;
}

void MessageQueue::push_message(const Message& _message)
{
	std::lock_guard<std::recursive_mutex> guard(m_rm);
	m_waiting_messages.push(_message);
	emit waiting_list_size_changed(m_waiting_messages.size());
}

const MessageQueue& MessageQueue::operator<<(const QString& _info)
{
	push_message(WarnMessage(_info));
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

MessageQueue::MessageQueue() : m_active_list_size_limit(DEF_ACTIVE_LIST_LIMIT), m_close_timer_value(DEF_CLOSE_TIMER), m_processing_interval(DEF_PROCESSING_INTERVAL), m_base_widget(nullptr)
{
	m_msg_window_size = QSize(DEF_WIN_WIDTH, DEF_WIN_HEIGTH);

	QTimer* timer = new QTimer(this);
	timer->setInterval(m_processing_interval);
	QObject::connect(timer, &QTimer::timeout, this, &MessageQueue::process_messages);
	timer->start();

	QStateMachine* machine = new QStateMachine(this);
	QState* add_win_state = new QState(machine);
	QState* remove_win_state = new QState(machine);
	QState* busy_state = new QState(machine);

	busy_state->addTransition(this, &MessageQueue::remove_msg, remove_win_state);
	busy_state->addTransition(this, &MessageQueue::add_msg, add_win_state);
	add_win_state->addTransition(this, &MessageQueue::busy, busy_state);
	remove_win_state->addTransition(this, &MessageQueue::busy, busy_state);

	QObject::connect(add_win_state, &QState::entered, this, [this]()
	{
		std::lock_guard<std::recursive_mutex> guard(m_rm);

		if (m_active_list.size() >= m_active_list_size_limit)
		{
			emit busy();
			return;
		}

		PopupMsgWindow* win = new PopupMsgWindow();
		win->setFixedSize(m_msg_window_size);
		win->set_base_widget(m_base_widget);
		win->set_close_time(m_close_timer_value);
		win->set_message(m_waiting_messages.front());

		add_to_active_list(win);
		m_waiting_messages.pop();
		emit waiting_list_size_changed(m_waiting_messages.size());
	});

	QObject::connect(remove_win_state, &QState::entered, this, [this]
	{
		remove_from_active_list(m_remove_list.front());
		delete m_remove_list.front();						//I really don't like raw pointers, but this window has a parent and shared_ptr (QSharedPointer) is not a good idea here
		m_remove_list.pop();
	});

	machine->setInitialState(busy_state);
	machine->start();
}

void MessageQueue::add_to_remove_list(PopupMsgWindow* _win)
{
	m_remove_list.push(_win);
}

void MessageQueue::add_to_active_list(PopupMsgWindow* _win)
{
	QObject::connect(_win, &PopupMsgWindow::finish_fade_out, this, std::bind(&MessageQueue::add_to_remove_list, this, _win));		//remove after close

	if (m_active_list.empty())		//first message
	{
		QObject::connect(_win, &PopupMsgWindow::finish_fade_in, this, &MessageQueue::busy);
	}
	else //there are already messages
	{
		//wait until first finish moving up
		static QMetaObject::Connection connection;
		connection = QObject::connect(m_active_list.front(), &PopupMsgWindow::finish_moving_up, this, [this, _win]()
		{
			QObject::disconnect(connection);
			emit busy();
		});

		for (auto& it : m_active_list)
			it->move_up();
	}

	m_active_list.push_front(_win);

//	m_out->write(_win)
//	_win->fade_in();
	_win->show();
}

void MessageQueue::remove_from_active_list(PopupMsgWindow* _win)
{
	std::list<PopupMsgWindow*>::iterator cwin = std::find(m_active_list.begin(), m_active_list.end(), _win);

	if (cwin == m_active_list.end())
		return;

	if (*cwin != m_active_list.back())
	{
		//wait until back finish moving down
		PopupMsgWindow* upper = *std::next(cwin);
		static QMetaObject::Connection connection;
		connection = QObject::connect(upper, &PopupMsgWindow::finish_moving_down, this, [cwin, this]()
		{		
			QObject::disconnect(connection);
			m_active_list.remove(*cwin);
			emit busy();
		});

		for (auto it = std::next(cwin); it != m_active_list.end(); ++it)
			(*it)->move_down();
	}
	else
	{
		m_active_list.remove(*cwin);
		emit busy();
	}	
}

void MessageQueue::process_messages()
{
	if (m_waiting_messages.empty() && m_remove_list.empty())
		return;

	if (!m_remove_list.empty())
		emit remove_msg();

	if (!m_waiting_messages.empty())
		emit add_msg();
}

/*-------------------------------------*/

void MessageQueue::set_processing_interval(size_t _msec)
{
	m_processing_interval = _msec;
}

size_t MessageQueue::processing_interval() const
{
	return m_processing_interval;
}

/*-------------------------------------*/

void MessageQueue::set_base_widget(QWidget* _base_widget) //no owns
{
	m_base_widget = _base_widget;
}

QWidget* MessageQueue::base_widget() const
{
	return m_base_widget;
}

/*-------------------------------------*/

void MessageQueue::set_msg_close_time(size_t _msec)
{
	m_close_timer_value = _msec;
}

size_t MessageQueue::msg_close_time() const
{
	return m_close_timer_value;
}

/*-------------------------------------*/

void MessageQueue::set_active_size_limit(size_t _size)
{
	m_active_list_size_limit = _size;
}

size_t MessageQueue::active_size_limit() const
{
	return m_active_list_size_limit;
}

/*-------------------------------------*/

void MessageQueue::set_output_device(QIODevice* _out)
{
	m_out = _out;
}

QIODevice* MessageQueue::output_device() const
{
	return m_out;
}

/*-------------------------------------*/

void MessageQueue::set_waiting_cutoff_size(size_t _size)
{
	m_waiting_list_size_limit = _size;
}

size_t MessageQueue::waiting_cutoff_size() const
{
	return m_waiting_list_size_limit;
}

/*-------------------------------------*/

void MessageQueue::set_window_size(const QSize& _size)
{
	m_msg_window_size = _size;
}

QSize MessageQueue::window_size() const
{
	return m_msg_window_size;
}