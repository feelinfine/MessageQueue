#pragma once

#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtGui/QDragMoveEvent>
#include <QtWidgets/QWidget>

class EventFilter : public QObject
{
	Q_OBJECT

public:
	EventFilter(QWidget* _spy_widget, QObject* _parent = nullptr): QObject(_parent), m_spy_widget(_spy_widget)
	{
		_spy_widget->installEventFilter(this);
	}

	bool eventFilter(QObject *_obj, QEvent *_event)
	{
		if (qobject_cast<QWidget*>(_obj) == m_spy_widget) 
		{
			if (_event->type() == QEvent::Move)
			{
				QMoveEvent* move_event = dynamic_cast<QMoveEvent*>(_event);
				emit position_changed(move_event->pos() - move_event->oldPos());
			}

			if (_event->type() == QEvent::NonClientAreaMouseButtonRelease)
			{
				int t = 0;
			}
		}

		return m_spy_widget->eventFilter(_obj, _event);
	}

private:
	QWidget* m_spy_widget;

signals:
	void position_changed(QPoint _diff_pos);
};