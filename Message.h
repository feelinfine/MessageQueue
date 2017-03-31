#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QPixmap>

enum class DropBehavior
{
	NO_DROP,
	DROP_IF_LIMITED,
	DROP_IF_THREAD_CONFLICT
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
	Message(const QString& _prefix, const QString& _title, const QString& _text, const QPixmap& _pixmap, QObject* _parent = nullptr);
	Message(const Message& _other);

	QString text() const;
	QString title() const;
	QString prefix() const;
	DropBehavior drop_behavior() const;
	LogBehavior log_behavior() const;
	QPixmap pixmap() const;

	virtual ~Message();

public slots:
	void set_text(const QString& _text);
	void set_title(const QString& _title);
	void set_prefix(const QString& _prefix);
	void set_drop_behavior(DropBehavior _db);
	void set_log_behavior(LogBehavior _lb);
	void set_pixmap(const QPixmap& _pixmap);

	virtual QString to_qstring() const;

private:
	QString m_text;
	QString m_title;
	QString m_prefix;
	DropBehavior m_db;
	LogBehavior m_lb;
	QPixmap m_pixmap;
};