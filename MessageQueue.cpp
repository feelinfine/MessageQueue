
#include "MessageQueue.h"

#include <QtCore/QStateMachine>

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

void MessageQueue::push_message(const Message& _message)
{
	PopupMsgWindow* win = new PopupMsgWindow();	//WA_DeleteOnClose
	win->set_base_widget(m_base_widget);
	win->set_close_time(m_close_timer_value);
	win->set_message(_message.text());

	switch (_message.type())
	{
	case MsgType::INFO:		win->set_icon(win->style()->standardIcon(QStyle::SP_MessageBoxInformation));	break;
	case MsgType::WARNING:	win->set_icon(win->style()->standardIcon(QStyle::SP_MessageBoxWarning));		break;
	case MsgType::ERROR:	win->set_icon(win->style()->standardIcon(QStyle::SP_MessageBoxCritical));		break;
	}

	std::lock_guard<std::recursive_mutex> guard(m_rm);
	m_waiting_messages.push(win);
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

///////////////////////////////////////////////////////////

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

	QStateMachine* machine = new QStateMachine(this);
	QState* add_win_state = new QState(machine);
	QState* remove_win_state = new QState(machine);
	QState* busy_state = new QState(machine);

	busy_state->addTransition(this, &MessageQueue::removing_msg, remove_win_state);
	busy_state->addTransition(this, &MessageQueue::adding_msg, add_win_state);
	add_win_state->addTransition(this, &MessageQueue::busy, busy_state);
	remove_win_state->addTransition(this, &MessageQueue::busy, busy_state);

	QObject::connect(add_win_state, &QState::entered, this, [this]()
	{
		m_out->write("Add window state");
		std::lock_guard<std::recursive_mutex> guard(m_rm);
		add_to_active_list(m_waiting_messages.front());
		m_waiting_messages.pop();
	});

	QObject::connect(remove_win_state, &QState::entered, this, [this]()
	{
		m_out->write("Remove window state");
		remove_from_active_list(m_remove_list.front());
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

	_win->fade_in();
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
		connection = QObject::connect(upper, &PopupMsgWindow::finish_moving_down, this, [this]()
		{		
			QObject::disconnect(connection);
			emit busy();
		});

		for (auto it = std::next(cwin); it != m_active_list.end(); ++it)
			(*it)->move_down();
	}
	else
		emit busy();


	m_active_list.remove(*cwin);
}

void MessageQueue::process_messages()
{
	if (m_waiting_messages.empty() && m_remove_list.empty())
		return;

	if (!m_remove_list.empty())
		emit removing_msg();

	if (!m_waiting_messages.empty())
		emit adding_msg();
}

void MessageQueue::set_output_stream(QIODevice* _out)
{
	m_out = _out;
}
