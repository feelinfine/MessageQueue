#pragma once

#include "Message.h"

#include <QtCore/QDateTime>

Message::Message(QObject* _parent /*= nullptr*/) : Message("", "", _parent)
{
}

Message::Message(const QString& _title, const QString& _text, QObject* _parent /*= nullptr*/) : Message("", _title, _text, _parent)
{
}

Message::Message(const QString& _prefix, const QString& _title, const QString& _text, QObject* _parent /*= nullptr*/) : Message(_prefix,_title, _text, QImage(),_parent)
{
}

Message::Message(const QString& _prefix, const QString& _title, const QString& _text, const QImage& _pixmap, QObject* _parent /*= nullptr*/) : 
	m_prefix(_prefix),
	m_title(_title),
	m_text(_text),
	m_image(_pixmap),
	m_db(DropBehavior::NO_DROP),
	m_lb(LogBehavior::WRITE_TO_LOG),
	QObject(_parent)
{

}

Message::Message(const Message& _other)
{
	m_text = _other.text();
	m_title = _other.title();
	m_prefix = _other.prefix();
	m_image = _other.image();
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

QImage Message::image() const
{
	return m_image;
}

void Message::set_image(const QImage& _pixmap)
{
	m_image = _pixmap;
}

LogBehavior Message::log_behavior() const
{
	return m_lb;
}

QString Message::to_qstring() const
{
	QDateTime date_time = QDateTime::currentDateTime();
	return date_time.toString() + " | " + m_prefix + ": " + m_title + " " + m_text;
}

Message::~Message()
{
}
