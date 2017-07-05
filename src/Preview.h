#pragma once

#include <QtGui>
#include <QGLWidget>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qglshaderprogram.h>

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>

#include "Level.h"
#include "PolygonDef.h"

class GLPreview : public QGLWidget
{
	Q_OBJECT

public:
	GLPreview(QWidget* parent, Level* level);
	~GLPreview();

private:
	Level* m_level;
};



class PreviewWindow : public QDockWidget
{
	Q_OBJECT

public:
	PreviewWindow(QWidget* parent, Level* level);
	~PreviewWindow();

	void setTexture(QImage* texture);

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void onClose();

private:
	QMainWindow* m_window;

	GLPreview* m_widget;
	Level* m_level;
};