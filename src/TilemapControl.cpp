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


	m_toolbar = new QToolBar(m_window);
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
}

void TilemapControl::closeEvent(QCloseEvent* event)
{
	emit onClose();
}