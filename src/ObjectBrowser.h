#pragma once

#include <QtGui>
#include <QListWidget>
#include <QStyledItemDelegate>
#include <qmenu.h>
#include <qmessagebox.h>

#include "Level.h"

#include <string>


class ObjectBrowserDelegate : public QStyledItemDelegate
{
public:
	ObjectBrowserDelegate(QObject* parent=0) : QStyledItemDelegate(parent) {}
	~ObjectBrowserDelegate() {}
	
protected:
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QRect r = option.rect;

		if (option.state & QStyle::State_Selected)
		{
			//painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
			QLinearGradient gradientSelected(r.left(),r.top(),r.left(),r.height()+r.top());
			gradientSelected.setColorAt(0.0, QColor::fromRgb(119,213,247));
			gradientSelected.setColorAt(0.9, QColor::fromRgb(27,134,183));
			gradientSelected.setColorAt(1.0, QColor::fromRgb(0,120,174));
			painter->setBrush(gradientSelected);
			painter->drawRect(r);
		}

		QString title = index.data(Qt::DisplayRole).toString();
//		QString start = "Start: " + index.data(Qt::UserRole + 1).toString();
//		QString end = "End: " + index.data(Qt::UserRole + 2).toString();

		QPen white(QColor::fromRgb(255, 255, 255), 1, Qt::SolidLine);
		QPen black(QColor::fromRgb(0,0,0), 1, Qt::SolidLine);
		QPen grey(QColor::fromRgb(192, 192, 192), 1, Qt::SolidLine);

		painter->setFont(QFont("Lucida Grande", 8, QFont::Normal));
			
		if (option.state & QStyle::State_Selected)
		{
			painter->setPen(white);
		}
		else
		{
			if (index.flags() & Qt::ItemIsEnabled)
				painter->setPen(black);
			else
				painter->setPen(grey);
		}

		painter->drawText(QRect(r.x()+5, r.y()-3, r.width(), r.height()), Qt::AlignBottom|Qt::AlignLeft, title);


		QString id = "ID: " + index.data(Qt::UserRole + 1).toString();
		painter->drawText(QRect(r.x()+230, r.y()-3, r.width(), r.height()), Qt::AlignBottom|Qt::AlignLeft, id);
	}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		return QSize(290, 24);
	}
};


class ObjectBrowser : public QListWidget
{
	Q_OBJECT

public:
	ObjectBrowser(QWidget* parent, Level* level);
	~ObjectBrowser();

signals:
	void onSelectObject(int object_id);
	void onObjectDataChanged(int object_id);

public slots:
	void reset();
	void add(int object_id);
	void remove(int object_id);
	void select(int object_id);
	void deselect();
	void itemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void itemDataChanged(QListWidgetItem* item);
	void enableFilter(int filter);
	void disableFilter(int filter);

private:
	void filterItems();

	Level* m_level;
	int m_filter;
};