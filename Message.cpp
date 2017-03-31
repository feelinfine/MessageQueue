#pragma once

#include "Message.h"

Message::Message(QObject* _parent /*= nullptr*/) : Message("", "", _parent)
{
}

Message::Message(const QString& _title, const QString& _text, QObject* _parent /*= nullptr*/) : Message("", _title, _text, _parent)
{
}

Message::Message(const QString& _prefix, const QString& _title, const QString& _text, QObject* _parent /*= nullptr*/) : Message(_prefix,_title, _text, QPixmap(),_parent)
{
}

Message::Message(const QString& _prefix, const QString& _title, const QString& _text, const QPixmap& _pixmap, QObject* _parent /*= nullptr*/) : 
	m_prefix(_prefix),
	m_title(_title),
	m_text(_text),
	m_pixmap(_pixmap),
	QObject(_parent)
{
}

Message::Message(const Message& _other)
{
	m_text = _other.text();
	m_title = _other.title();
	m_prefix = _other.prefix();
	m_pixmap = _other.pixmap();
	m_db = _other.drop_behavior();
	m_lb = _other.log_behavior();
}

void Message::set_text(const QString& _text)
{
	m_text = _text;
}

QString Message::text() const
{
	return m_text;
}

void Message::set_title(const QString& _title)
{
	m_title = _title;
}

QString Message::title() const
{
	return m_title;
}

void Message::set_prefix(const QString& _prefix)
{
	m_prefix = _prefix;
}

QString Message::prefix() const
{
	return m_prefix;
}

void Message::set_drop_behavior(DropBehavior _db)
{
	m_db = _db;
}

DropBehavior Message::drop_behavior() const
{
	return m_db;
}

void Message::set_log_behavior(LogBehavior _lb)
{
	m_lb = m_lb;
}

QPixmap Message::pixmap() const
{
	return m_pixmap;
}

void Message::set_pixmap(const QPixmap& _pixmap)
{
	m_pixmap = _pixmap;
}

LogBehavior Message::log_behavior() const
{
	return m_lb;
}

QString Message::to_qstring() const
{
	return m_prefix + ": " + m_title + ". " + m_text;
}

Message::~Message()
{
}
