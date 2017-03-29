#pragma once

#include <list>
#include <functional>
#include <queue>
#include <mutex>

#include <QtWidgets/QStyle>
#include <QtCore/QIODevice>

#include "PopupMessageWin.h"
#include "Message.h"

class MessageQueue final : public QObject
{
	Q_OBJECT

	static const size_t DEF_QUEUE_SIZE = 4;
	static const size_t DEF_CLOSE_TIMER = 4000;	//msec

public:
	static MessageQueue& instance(QWidget* _base_widget = nullptr, size_t _close_time = DEF_CLOSE_TIMER, size_t _queue_size = DEF_QUEUE_SIZE);

	void set_base_widget(QWidget* _base_widget);	//no owns
	void set_close_timer(size_t _msec);
	void set_limit_size(size_t _size);
	void set_output_stream(QIODevice* _out);
	void push_message(const Message& _message);
	const MessageQueue& operator << (const QString& _info);
	const MessageQueue& operator << (const Message& _msg);
	~MessageQueue();

private:
	MessageQueue(QWidget* _main_widget, size_t _close_time = DEF_CLOSE_TIMER, size_t _queue_size = DEF_QUEUE_SIZE);

	MessageQueue(const MessageQueue&) = delete;
	MessageQueue& operator= (const MessageQueue&) = delete;
	void show_message(MsgType _mtype, const QString& _str);

	void add_to_active_list(PopupMsgWindow* _win);
	void add_to_remove_list(PopupMsgWindow* _win);

	void remove_from_active_list(PopupMsgWindow* _win);

private slots:
	void process_messages();

signals:
	void locked();
	void unlocked();
	void busy();
	void adding_msg();
	void removing_msg();

private:
	bool m_processing;
	size_t m_queue_size;
	size_t m_close_timer_value;
	size_t m_processing_interval;
	QWidget* m_base_widget;
	std::list<PopupMsgWindow*> m_active_list;
	std::queue<PopupMsgWindow*> m_waiting_messages;
	std::queue<PopupMsgWindow*> m_remove_list;
	std::recursive_mutex m_rm;
	QIODevice* m_out;

	std::function<void(PopupMsgWindow*)> m_processing_function;
};