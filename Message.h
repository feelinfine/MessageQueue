#pragma once

#include <QtCore/QString>

enum class MsgType
{
	INFO,
	WARNING,
	ERROR
};

class Message
{
public:
	Message(MsgType _type, const QString& _msg) : m_msg(_msg), m_type(_type) {};

	QString text() const
	{
		return m_msg;
	}

	MsgType type() const
	{
		return m_type;
	}

private:
	QString m_msg;
	MsgType m_type;
};