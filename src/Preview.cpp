#include "Preview.h"




GLPreview::GLPreview(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{

}

GLPreview::~GLPreview()
{

}





PreviewWindow::PreviewWindow(QWidget* parent, Level* level) : QDockWidget("Preview", parent, 0)
{
	m_level = level;

	m_widget = new GLPreview(parent, m_level);
	m_window = new QMainWindow(0);

	setFocusPolicy(Qt::ClickFocus);

	m_window->setParent(this);
	setWidget(m_window);
	m_window->setCentralWidget(m_widget);
	m_window->setContextMenuPolicy(Qt::NoContextMenu);

	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	this->setMinimumWidth(960);
	this->setMinimumHeight(540);
	this->setMaximumWidth(960);
	this->setMaximumHeight(540);

	// don't allow docking
	this->setAllowedAreas(0);

	this->setHidden(false);
}

PreviewWindow::~PreviewWindow()
{

}

void PreviewWindow::closeEvent(QCloseEvent* event)
{
	emit onClose();
}