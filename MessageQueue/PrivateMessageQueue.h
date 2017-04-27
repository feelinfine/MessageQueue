#pragma once

#include <list>
#include <queue>
#include <mutex>

#include <QtCore/QIODevice>

#include "PopupMessageWin.h"
#include "EventFilter.h"

#include "MessageQueue.h"

class MessageQueue::PrivateMessageQueue
{
public:
	size_t m_active_list_size_limit;
	size_t m_waiting_list_size_limit;
	size_t m_close_timer_value;
	size_t m_processing_interval;
	std::recursive_mutex m_rm;
	QIODevice* m_out;
	QSize m_msg_window_size;
	QWidget* m_base_widget;

	std::list<PopupMsgWindow*> m_active_list;
	std::queue<Message> m_waiting_messages;
	std::queue<PopupMsgWindow*> m_remove_list;

	QTimer* m_processing_timer;
	EventFilter* m_filter;
};