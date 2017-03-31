#pragma once

#include "Message.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QMessageBox>

class WarnMessage : public Message
{
public:
	explicit WarnMessage(const QString& _text, QObject* _parent = nullptr) : Message("Warning", "Warning!", _text, _parent)
	{
		QWidget wdg;
		QIcon icon = wdg.style()->standardIcon(QStyle::SP_MessageBoxWarning);
		set_pixmap(icon.pixmap(icon.actualSize(QSize(32, 32))));
	}
};

class ErrorMessage : public Message
{
public:
	explicit ErrorMessage(const QString& _text, QObject* _parent = nullptr) : Message("Error", "Error!", _text, _parent)
	{
		QWidget wdg;
		QIcon icon = wdg.style()->standardIcon(QStyle::SP_MessageBoxCritical);
		set_pixmap(icon.pixmap(icon.actualSize(QSize(32, 32))));
	}
};

class InfoMessage : public Message
{
public:
	explicit InfoMessage(const QString& _text, QObject* _parent = nullptr) : Message("Info", "Information", _text, _parent)
	{
		QWidget wdg;
		QIcon icon = wdg.style()->standardIcon(QStyle::SP_MessageBoxInformation);
		set_pixmap(icon.pixmap(icon.actualSize(QSize(32, 32))));
	}
};