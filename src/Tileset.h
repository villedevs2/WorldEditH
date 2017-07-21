#pragma once

#include <QtGui>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qlistwidget.h>
#include <qstyleditemdelegate.h>
#include <qimage.h>

#include "Level.h"
#include "Thumbnail.h"

#define TILESET_THUMB_W		70
#define TILESET_THUMB_H		70

class TilesetDelegate : public QStyledItemDelegate
{
public:
	TilesetDelegate(QObject* parent=0) : QStyledItemDelegate(parent) {}
	~TilesetDelegate() {}
	
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
			painter->drawRect(QRect(r.x(), r.y(), r.width() - 1, r.height() - 1));
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

		QImage* image = index.data(Qt::UserRole + 1).value<QImage*>();
		if (image != nullptr)
		{
			int mx = r.x() + (r.width() / 2);
			int my = r.y() + ((r.height() - 20) / 2);
			painter->drawImage(QRect(mx-(TILESET_THUMB_W/2), my-(TILESET_THUMB_H/2), TILESET_THUMB_W, TILESET_THUMB_H), *image);
		}

		painter->drawText(QRect(r.x()+5, r.y()-3, r.width(), r.height()), Qt::AlignBottom|Qt::AlignLeft, title);


//		QString id = "ID: " + index.data(Qt::UserRole + 1).toString();
//		painter->drawText(QRect(r.x()+230, r.y()-3, r.width(), r.height()), Qt::AlignBottom|Qt::AlignLeft, id);
	}

	QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		return QSize(90, 90);
	}
};


class TilesetWidget : public QListWidget
{
	Q_OBJECT

public:
	TilesetWidget(QWidget* parent, Level* level, QImage* texture);
	~TilesetWidget();

signals:
	void onSelectTile(int tile_id);

public slots:
	void reset();
	void add(int tile_id);
	void remove(int tile_id);
	void replace(int tile_id);
	void setTexture(QImage* texture);

protected:

private:
	void defaults();

	Level* m_level;
	QImage* m_texture;
};


class TilesetWindow : public QDockWidget
{
	Q_OBJECT

public:
	TilesetWindow(QWidget* parent, Level* level, QImage* texture);
	~TilesetWindow();

signals:
	void onSelectTile(int tile_id);
	void onClose();

public slots:
	void reset();
	void add(int tile_id);
	void remove(int tile_id);
	void replace(int tile_id);
	void setTexture(QImage* texture);
	void itemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous);

protected:
	void closeEvent(QCloseEvent* event);

private:
	QMainWindow* m_window;
	TilesetWidget* m_widget;

	Level* m_level;
};