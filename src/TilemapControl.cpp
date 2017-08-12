#include "TilemapControl.h"

TilemapControl::TilemapControl(QWidget* parent, Level* level) : QDockWidget("Tilemap Control", parent, 0)
{
	m_level = level;

	m_window = new QMainWindow(0);

	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	setMinimumWidth(500);
	setMaximumWidth(500);
	setMinimumHeight(88);
	setMaximumHeight(88);

	// don't allow docking
	setAllowedAreas(0);
	setHidden(false);


	m_tilemap_label = new QLabel("Tilemap edit:");
	m_tilemap_combo = new QComboBox();
	m_tilemap_combo->addItem("Basic", QVariant(Level::TILEMAP_NORMAL));
	m_tilemap_combo->addItem("Floor", QVariant(Level::TILEMAP_FLOOR));
	connect(m_tilemap_combo, SIGNAL(activated(int)), this, SLOT(selectTilemap(int)));

	QBoxLayout* tile_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	tile_layout->setSpacing(3);
	tile_layout->setMargin(3);
	tile_layout->addWidget(m_tilemap_label);
	tile_layout->addWidget(m_tilemap_combo);

	m_tilemap_widget = new QWidget();
	m_tilemap_widget->setLayout(tile_layout);



	m_toolbar = new QToolBar(m_window);
	m_toolbar->addWidget(m_tilemap_widget);
	m_toolbar->setFloatable(false);
	m_toolbar->setMovable(false);
	m_window->addToolBar(m_toolbar);

	m_window->setParent(this);
	setWidget(m_window);
}

TilemapControl::~TilemapControl()
{

}

void TilemapControl::reset()
{
	m_tilemap_combo->setCurrentIndex(0);
	emit onSelectTilemap((Level::TilemapType)(0));
}

void TilemapControl::closeEvent(QCloseEvent* event)
{
	emit onClose();
}


void TilemapControl::selectTilemap(int map)
{
	emit onSelectTilemap((Level::TilemapType)m_tilemap_combo->itemData(map).toInt());
}