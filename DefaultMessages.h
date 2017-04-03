#pragma once

#include "Message.h"

#include <QtWidgets/QStyle>
#include <QtWidgets/QApplication>

class WarnMessage : public Message
{
public:
	explicit WarnMessage(const QString& _text, QObject* _parent = nullptr) : Message("Warning", "Warning!", _text, _parent)
	{
		set_image(QImage("Resources/icon_warning.png"));
	}
};

class ErrorMessage : public Message
{
public:
	explicit ErrorMessage(const QString& _text, QObject* _parent = nullptr) : Message("Error", "Error!", _text, _parent)
	{
		set_image(QImage("Resources/icon_critical.png"));
	}
};

class InfoMessage : public Message
{
public:
	explicit InfoMessage(const QString& _text, QObject* _parent = nullptr) : Message("Info", "Information", _text, _parent)
	{
		set_image(QImage("Resources/icon_info.png"));
	}
};