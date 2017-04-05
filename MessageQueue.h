#pragma once

#include <list>
#include <queue>
#include <mutex>

#include <QtCore/QIODevice>

#include "PopupMessageWin.h"
#include "Message.h"
#include "EventFilter.h"

class MessageQueue final : public QObject
{
	Q_OBJECT

	const size_t DEF_ACTIVE_LIST_LIMIT = 3;
	const size_t DEF_CLOSE_TIMER = 5000;	//msec
	const size_t DEF_WIN_WIDTH = 300;
	const size_t DEF_WIN_HEIGTH = 150;
	const size_t DEF_PROCESSING_INTERVAL = 50; //msec

public:
	static MessageQueue& instance();

	void set_processing_interval(size_t _msec);
	void set_base_widget(QWidget* _base_widget);	//no owns
	void set_msg_close_time(size_t _msec);
	void set_active_size_limit(size_t _size);
	void set_waiting_cutoff_size(size_t _size);
	void set_output_device(QIODevice* _out);
	void set_window_size(const QSize& _size);

	size_t processing_interval() const;
	QWidget* base_widget() const;
	size_t msg_close_time() const;
	size_t active_size_limit() const;
	size_t waiting_cutoff_size() const;
	QIODevice* output_device() const;
	QSize window_size() const;

	const MessageQueue& operator << (const QString& _info);
	const MessageQueue& operator << (const Message& _msg);

	~MessageQueue();
	MessageQueue(const MessageQueue&) = delete;
	MessageQueue& operator= (const MessageQueue&) = delete;

public slots:
	void push_message(const Message& _message);

private slots:
	void process_messages();
	void create_one();
	void remove_one();
	void freeze_messages();
	void unfreeze_messages();

private:
	MessageQueue();

signals:
	void ready();
	void add_msg();
	void remove_msg();
	void waiting_list_size_changed(size_t _size);

private:
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