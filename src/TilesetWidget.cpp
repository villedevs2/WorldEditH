#include "TilesetWidget.h"


TilesetWidget::TilesetWidget(QWidget* parent, Level* level, QImage* texture) : QListWidget(parent)
{
	m_level = level;
	m_texture = texture;

	setFlow(QListView::LeftToRight);
	setWrapping(true);

	setItemDelegate(new TilesetDelegate(this));

	defaults();

	setCurrentRow(0);
}

TilesetWidget::~TilesetWidget()
{
}

void TilesetWidget::defaults()
{
	QImage* image = new QImage(TILESET_THUMB_W, TILESET_THUMB_H, QImage::Format_ARGB32);
	image->fill(QColor(0, 0, 0, 255));

	QListWidgetItem* item = new QListWidgetItem();
	item->setData(Qt::DisplayRole, QString("[Empty]"));
	item->setData(Qt::UserRole + 1, QVariant::fromValue<QImage*>(image));
	item->setData(Qt::UserRole + 2, -1);

	addItem(item);
}

void TilesetWidget::reset()
{
	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{		
		QListWidgetItem* obj = i.next();
		delete obj;
	};

	defaults();

	setCurrentRow(0);
}

void TilesetWidget::add(int tile_id)
{
	Tileset::Tile* tile = m_level->getTileset()->getTileById(tile_id);
	assert(tile != NULL);

	if (tile != nullptr)
	{
		/*
		QImage* image = new QImage(TILESET_THUMB_W, TILESET_THUMB_H, QImage::Format_ARGB32);

		int numtop = tile->numTopPoints();

		Thumbnail::fromTileType(image, m_texture, tile->top_points, numtop, tile->side_points, 4, tile->type);
		*/

		QImage* image = new QImage(tile->thumb_width, tile->thumb_height, QImage::Format_ARGB32);
		for (int j = 0; j < tile->thumb_height; j++)
		{
			QRgb* line = (QRgb*)image->scanLine(j);
			for (int i = 0; i < tile->thumb_width; i++)
			{
				line[i] = tile->thumbnail[(j * tile->thumb_width) + i];
			}
		}

		QListWidgetItem* item = new QListWidgetItem();
		item->setData(Qt::DisplayRole, QString(tile->name.c_str()));
		item->setData(Qt::UserRole + 1, QVariant::fromValue<QImage*>(image));
		item->setData(Qt::UserRole + 2, tile_id);

		addItem(item);
	}
}

void TilesetWidget::replace(int tile_id)
{
	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{
		QListWidgetItem* obj = i.next();
		int id = obj->data(Qt::UserRole + 2).toInt();

		if (tile_id == id)
		{
			QImage* image = obj->data(Qt::UserRole + 1).value<QImage*>();
			if (image != nullptr)
				delete image;



			Tileset::Tile* tile = m_level->getTileset()->getTileById(tile_id);
			assert(tile != nullptr);

			image = new QImage(tile->thumb_width, tile->thumb_height, QImage::Format_ARGB32);
			for (int j = 0; j < tile->thumb_height; j++)
			{
				QRgb* line = (QRgb*)image->scanLine(j);
				for (int i = 0; i < tile->thumb_width; i++)
				{
					line[i] = tile->thumbnail[(j * tile->thumb_width) + i];
				}
			}

			//int numtop = tile->numTopPoints();

			//Thumbnail::fromTileType(image, m_texture, tile->top_points, numtop, tile->side_points, 4, tile->type);

			obj->setData(Qt::DisplayRole, QString(tile->name.c_str()));
			obj->setData(Qt::UserRole + 1, QVariant::fromValue<QImage*>(image));
			// tile_id stays as is

			return;
		}
	};
}

void TilesetWidget::remove(int tile_id)
{
	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{		
		QListWidgetItem* obj = i.next();
		int id = obj->data(Qt::UserRole + 2).toInt();

		if (tile_id == id)
		{
			if (obj->isSelected())
				setCurrentRow(0);

			QImage* image = obj->data(Qt::UserRole + 1).value<QImage*>();
			if (image != nullptr)
				delete image;

			delete obj;
		}
	};
}

void TilesetWidget::setTexture(QImage* texture)
{
	m_texture = texture;

	// update thumbnails
	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{
		QListWidgetItem* obj = i.next();

		QImage* image = obj->data(Qt::UserRole + 1).value<QImage*>();
		int id = obj->data(Qt::UserRole + 2).toInt();
		
		Tileset::Tile* tile = m_level->getTileset()->getTileById(id);
		
		// TODO: update thumbnails...

		/*
		if (tile != nullptr)
		{
			int numtop = tile->numTopPoints();

			Thumbnail::fromTileType(image, m_texture, tile->top_points, numtop, tile->side_points, 4, tile->type);
		}
		*/
	};

	update();
}



TilesetWindow::TilesetWindow(QWidget* parent, Level* level, QImage* texture) : QDockWidget("Tileset", parent, 0)
{
	m_window = new QMainWindow(0);
	m_level = level;

	setFocusPolicy(Qt::ClickFocus);

	m_widget = new TilesetWidget(this, level, texture);
	m_widget->setMinimumWidth(1000);


	m_window->setParent(this);
	setWidget(m_window);
	m_window->setCentralWidget(m_widget);
	m_window->setContextMenuPolicy(Qt::NoContextMenu);
	
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	this->setMinimumWidth(1000);
	this->setMinimumHeight(700);
	this->setMaximumWidth(1800);
	this->setMaximumHeight(1500);

	// don't allow docking
	this->setAllowedAreas(0);

	this->setHidden(false);

	connect(m_widget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(itemSelectionChanged(QListWidgetItem*, QListWidgetItem*)));
}

TilesetWindow::~TilesetWindow()
{
}

void TilesetWindow::closeEvent(QCloseEvent* event)
{
	emit onClose();
}

void TilesetWindow::reset()
{
	m_widget->reset();
}

void TilesetWindow::add(int tile_id)
{
	m_widget->add(tile_id);
}

void TilesetWindow::remove(int tile_id)
{
	m_widget->remove(tile_id);
}

void TilesetWindow::replace(int tile_id)
{
	m_widget->replace(tile_id);
}

void TilesetWindow::setTexture(QImage* texture)
{
	m_widget->setTexture(texture);
}

void TilesetWindow::itemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (current)
	{
		int id = current->data(Qt::UserRole + 2).toInt();
		int index = m_level->getTileset()->getTileIndexById(id);
		emit onSelectTile(index);
	}
}