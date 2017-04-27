#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QPixmap>

enum class DropBehavior
{
	NO_DROP,
	DROP_IF_LIMITED,
	DROP_IF_THREAD_CONFLICT,
	DROP_ALWAYS
};

enum class LogBehavior
{
	WRITE_TO_LOG,
	DONT_WRITE_TO_LOG
};

class Message : public QObject
{
	Q_OBJECT

public:
	Message(QObject* _parent = nullptr);
	Message(const QString& _title, const QString& _text, QObject* _parent = nullptr);
	Message(const QString& _prefix, const QString& _title, const QString& _text, QObject* _parent = nullptr);
	Message(const QString& _prefix, const QString& _title, const QString& _text, const QImage& _image, QObject* _parent = nullptr);
	Message(const Message& _other);

	QString text() const;
	QString title() const;
	QString prefix() const;
	DropBehavior drop_behavior() const;
	LogBehavior log_behavior() const;
	QImage image() const;

	virtual ~Message();

public slots:
	void set_text(const QString& _text);
	void set_title(const QString& _title);
	void set_prefix(const QString& _prefix);
	void set_drop_behavior(DropBehavior _db);
	void set_log_behavior(LogBehavior _lb);
	void set_image(const QImage& _pixmap);

	virtual QString to_qstring() const;

private:
	QString m_text;
	QString m_title;
	QString m_prefix;
	DropBehavior m_db;
	LogBehavior m_lb;
	QImage m_image;
};

class WarnMessage : public Message
{
public:
	explicit WarnMessage(const QString& _text, QObject* _parent = nullptr) : Message("Warning", "Warning!", _text, QImage("Resources/icon_warning.png"), _parent)
	{
	}
};

class ErrorMessage : public Message
{
public:
	explicit ErrorMessage(const QString& _text, QObject* _parent = nullptr) : Message("Error", "Error!", _text, QImage("Resources/icon_critical.png"), _parent)
	{
	}
};

class InfoMessage : public Message
{
public:
	explicit InfoMessage(const QString& _text, QObject* _parent = nullptr) : Message("Info", "Information", _text, QImage("Resources/icon_info.png"), _parent)
	{
	}
};