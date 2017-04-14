
#include "MessageQueue.h"
#include "DefaultMessages.h"

#include <QtCore/QStateMachine>
#include <QtCore/QHistoryState>

MessageQueue& MessageQueue::instance()
{
	static MessageQueue self;	//C++11 thread safe, if C++03 use double-check-locking
	return self;
}

void MessageQueue::push_message(const Message& _message)
{
	std::lock_guard<std::recursive_mutex> guard(m_rm);
	m_waiting_messages.push(_message);

	if (m_out && _message.log_behavior() == LogBehavior::WRITE_TO_LOG)
	{
		QTextDocument td;
		td.setHtml(_message.to_qstring());
		m_out->write((td.toPlainText() + "\n").toLocal8Bit());
	}

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

MessageQueue::MessageQueue() : 
	m_active_list_size_limit(DEF_ACTIVE_LIST_LIMIT), 
	m_close_timer_value(DEF_CLOSE_TIMER), 
	m_processing_interval(DEF_PROCESSING_INTERVAL), 
	m_base_widget(nullptr),
	m_out(nullptr),
	m_filter(nullptr)
{
	m_msg_window_size = QSize(DEF_WIN_WIDTH, DEF_WIN_HEIGTH);

	m_filter = new EventFilter(this);

	QObject::connect(m_filter, &EventFilter::position_changed, this, [this](QPoint _pos)
	{
		for (auto& it : m_active_list)
			it->move(it->pos() + _pos);
	});

	//timer
	m_processing_timer = new QTimer(this);			//owns
	m_processing_timer->setInterval(m_processing_interval);
	QObject::connect(m_processing_timer, &QTimer::timeout, this, &MessageQueue::process_messages);

	//machine
	QStateMachine* machine = new QStateMachine(this);
	QState* working_state = new QState(machine);
	QState* ready_state = new QState(working_state);
	QState* create_win_state = new QState(working_state);
	QState* remove_win_state = new QState(working_state);

	QState* paused_state = new QState(machine);

	ready_state->addTransition(this, &MessageQueue::remove_msg, remove_win_state);
	ready_state->addTransition(this, &MessageQueue::add_msg, create_win_state);
	create_win_state->addTransition(this, &MessageQueue::ready, ready_state);
	remove_win_state->addTransition(this, &MessageQueue::ready, ready_state);
	ready_state->addTransition(m_filter, &EventFilter::freeze, paused_state);
	create_win_state->addTransition(m_filter, &EventFilter::freeze, paused_state);
	remove_win_state->addTransition(m_filter, &EventFilter::freeze, paused_state);
	paused_state->addTransition(this, &MessageQueue::ready, ready_state);

	QObject::connect(ready_state, &QState::entered, m_processing_timer, static_cast<void(QTimer::*)()>(&QTimer::start));
	QObject::connect(ready_state, &QState::exited, m_processing_timer, &QTimer::stop);
	QObject::connect(create_win_state, &QState::entered, this, &MessageQueue::create_one);
	QObject::connect(remove_win_state, &QState::entered, this, &MessageQueue::remove_one);
	QObject::connect(paused_state, &QState::entered, this, &MessageQueue::freeze_messages);

	QObject::connect(m_filter, &EventFilter::end_freeze, this, [this]
	{
		int some = 0;
		for (auto& it : m_active_list)
		{
			if (it->moving())
			{
				some++;
				it->resume();
			}
		}

		if (some == 0)
			emit ready();
	});

	working_state->setInitialState(ready_state);
	machine->setInitialState(working_state);
	machine->start();
}

void MessageQueue::create_one()
{
	if (m_active_list.size() >= m_active_list_size_limit)
	{
		emit ready();
		return;
	}

	//maybe it's not needed because front item cannot be changed from another thread
	m_rm.lock();

	Message msg = m_waiting_messages.front();
	m_waiting_messages.pop();
	emit waiting_list_size_changed(m_waiting_messages.size());

	m_rm.unlock();
	//

	PopupMsgWindow* win = new PopupMsgWindow();
	win->setFixedSize(m_msg_window_size);
	win->set_base_widget(m_base_widget);
	win->set_close_time(m_close_timer_value);
	win->set_message(msg);

	if (m_active_list.empty())		//first message
	{
		QObject::connect(win, &PopupMsgWindow::finish_fade_in, this, &MessageQueue::ready);
	}
	else //there are already messages
	{
		//wait until first finish moving up
		static QMetaObject::Connection connection;
		connection = QObject::connect(m_active_list.front(), &PopupMsgWindow::finish_moving, this, [this]()
		{
			QObject::disconnect(connection);
			emit ready();
		});

		for (auto& it : m_active_list)
			it->move_up();
	}

	m_active_list.push_front(win);

	QObject::connect(win, &PopupMsgWindow::finish_fade_out, this, [win, this]()
	{
		m_remove_list.push(win);	//remove after close
	});

	win->show();
}

void MessageQueue::remove_one()
{
	auto cwin = std::find(m_active_list.begin(), m_active_list.end(), m_remove_list.front());
	m_remove_list.pop();

	if (cwin == m_active_list.end())
		return;

	PopupMsgWindow* to_delete = *cwin;

	if (*cwin != m_active_list.back())
	{
		//wait until back finish moving down
		PopupMsgWindow* upper = *std::next(cwin);
		static QMetaObject::Connection connection;
		connection = QObject ::connect(upper, &PopupMsgWindow::finish_moving, this, [this]()
		{		
			QObject::disconnect(connection);
			emit ready();
		});

		for (auto it = std::next(cwin); it != m_active_list.end(); ++it)
			(*it)->move_down();
	}
	else
		emit ready();

	m_active_list.remove(to_delete);
	delete to_delete;	//I really don't like raw pointers, but this window has a parent and shared_ptr (QSharedPointer) is not a good idea here
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

void MessageQueue::freeze_messages()
{
	for (auto& it : m_active_list)
	{
		if (it->moving())
			it->pause();
	}
}

void MessageQueue::unfreeze_messages()
{
	for (auto& it : m_active_list)
	{
		if (it->moving())
			it->resume();
	}
}

void MessageQueue::set_processing_interval(size_t _msec)
{
	m_processing_interval = _msec;
	m_processing_timer->stop();
	m_processing_timer->setInterval(m_processing_interval);
	m_processing_timer->start();
}

size_t MessageQueue::processing_interval() const
{
	return m_processing_interval;
}

void MessageQueue::set_base_widget(QWidget* _base_widget) //no owns
{
	m_base_widget = _base_widget;
	m_filter->set_widget(m_base_widget);
}

QWidget* MessageQueue::base_widget() const
{
	return m_base_widget;
}

void MessageQueue::set_msg_close_time(size_t _msec)
{
	m_close_timer_value = _msec;
}

size_t MessageQueue::msg_close_time() const
{
	return m_close_timer_value;
}

void MessageQueue::set_active_size_limit(size_t _size)
{
	m_active_list_size_limit = _size;
}

size_t MessageQueue::active_size_limit() const
{
	return m_active_list_size_limit;
}

void MessageQueue::set_output_device(QIODevice* _out)
{
	m_out = _out;
}

QIODevice* MessageQueue::output_device() const
{
	return m_out;
}

void MessageQueue::set_waiting_cutoff_size(size_t _size)
{
	m_waiting_list_size_limit = _size;
}

size_t MessageQueue::waiting_cutoff_size() const
{
	return m_waiting_list_size_limit;
}

void MessageQueue::set_window_size(const QSize& _size)
{
	m_msg_window_size = _size;
}

QSize MessageQueue::window_size() const
{
	return m_msg_window_size;
}