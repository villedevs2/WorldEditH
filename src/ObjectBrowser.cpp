#include "ObjectBrowser.h"

ObjectBrowser::ObjectBrowser(QWidget* parent, Level* level) : QListWidget(parent)
{
	connect(this, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(itemSelectionChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemDataChanged(QListWidgetItem*)));

	m_level = level;
	m_filter = 0;

	setItemDelegate(new ObjectBrowserDelegate(this));
}

ObjectBrowser::~ObjectBrowser()
{
}


void ObjectBrowser::itemSelectionChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
	if (current)
	{
		int id = current->data(Qt::UserRole + 1).toInt();
		emit onSelectObject(id);
	}
}

void ObjectBrowser::itemDataChanged(QListWidgetItem* item)
{
	if (item)
	{
		QString name = item->data(Qt::DisplayRole).toString();
		int id = item->data(Qt::UserRole + 1).toInt();
		
		Level::Object* obj = m_level->getObjectById(id);
		if (obj != NULL)
		{
			obj->setName(name.toStdString());

			emit onObjectDataChanged(id);
		}
	}
}


void ObjectBrowser::reset()
{
	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{		
		QListWidgetItem* obj = i.next();
		delete obj;
	};
}

void ObjectBrowser::add(int object_id)
{
	Level::Object* obj = m_level->getObjectById(object_id);
	assert(obj != NULL);

	QString name = QString(obj->getName().c_str());
	Level::ObjectType type = obj->getType();

	QListWidgetItem* item = new QListWidgetItem();
	item->setData(Qt::DisplayRole, name);
	item->setData(Qt::UserRole + 1, object_id);
	item->setData(Qt::UserRole + 2, (int)type);
	item->setFlags(item->flags() | Qt::ItemIsEditable);

	// filter
	Qt::ItemFlags flags = item->flags();
	if ((1 << (type - 1)) & m_filter)
		item->setFlags(flags | Qt::ItemIsEnabled);
	else
		item->setFlags(flags & ~Qt::ItemIsEnabled);

	addItem(item);
}

void ObjectBrowser::remove(int object_id)
{
	/*
	int numrows = count();
	for (int i=0; i < numrows; i++)
	{
		QListWidgetItem* row = item(i);
		int id = row->data(Qt::UserRole + 1).toInt();

		// if object id matches, delete
		if (object_id == id)
		{
			takeItem(i);
			delete row;
		}
	}
	*/

	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{		
		QListWidgetItem* obj = i.next();
		int id = obj->data(Qt::UserRole + 1).toInt();

		if (object_id == id)
		{
			if (obj->isSelected())
				setCurrentRow(-1);
			delete obj;
		}
	};
}

void ObjectBrowser::select(int object_id)
{
	int numrows = count();
	for (int i=0; i < numrows; i++)
	{
		QListWidgetItem* obj = item(i);
		int id = obj->data(Qt::UserRole + 1).toInt();

		if (object_id == id)
		{
			setCurrentRow(i);
			return;
		}
	}
}

void ObjectBrowser::deselect()
{
	setCurrentRow(-1);
}

void ObjectBrowser::filterItems()
{
	QList<QListWidgetItem*> list = findItems("*", Qt::MatchWildcard);
	QListIterator<QListWidgetItem*> i(list);

	while (i.hasNext())
	{		
		QListWidgetItem* obj = i.next();
		int type = obj->data(Qt::UserRole + 2).toInt();

		Qt::ItemFlags flags = obj->flags();

		if ((1 << (type - 1)) & m_filter)
			obj->setFlags(flags | Qt::ItemIsEnabled);
		else
			obj->setFlags(flags & ~Qt::ItemIsEnabled);
	};
}

void ObjectBrowser::enableFilter(int filter)
{
	m_filter |= filter;

	filterItems();
}

void ObjectBrowser::disableFilter(int filter)
{
	m_filter &= ~filter;

	filterItems();
}