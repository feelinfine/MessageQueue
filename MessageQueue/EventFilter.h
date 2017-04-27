#pragma once

#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtGui/QDragMoveEvent>
#include <QtWidgets/QWidget>
#include <QtCore/QTimer>

class EventFilter : public QObject
{
	Q_OBJECT

public:
	EventFilter(QObject* _parent = nullptr): QObject(_parent), m_filtered_widget(nullptr)
	{
	}

	void set_widget(QWidget* _filter_this_widget)
	{
		if (_filter_this_widget)
		{
			m_filtered_widget = _filter_this_widget;
			_filter_this_widget->installEventFilter(this);
		}
	}

	bool eventFilter(QObject *_obj, QEvent *_event)
	{
		if (qobject_cast<QWidget*>(_obj) == m_filtered_widget) 
		{
			if (_event->type() == QEvent::Move)
			{
				QMoveEvent* move_event = dynamic_cast<QMoveEvent*>(_event);
				emit position_changed(move_event->pos() - move_event->oldPos());
			}
			else if (_event->type() == QEvent::NonClientAreaMouseButtonRelease)
			{
				emit end_freeze();
			}
			else if (_event->type() == QEvent::NonClientAreaMouseButtonPress)
			{
				emit freeze();
			}
			else if (_event->type() == QEvent::WindowStateChange)
			{
				emit end_freeze();
			}		
			else if (_event->type() == QEvent::Resize)
			{
				QResizeEvent* resize_event = dynamic_cast<QResizeEvent*>(_event);
				QSize s = resize_event->size() - resize_event->oldSize();
				emit position_changed(QPoint(s.width(), s.height()));
			}

		}

		return m_filtered_widget->eventFilter(_obj, _event);
	}

private:
	QWidget* m_filtered_widget;

signals:
	void position_changed(QPoint _diff_pos);
	void end_freeze();
	void freeze();
};