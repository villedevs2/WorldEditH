#pragma once

#include <QtGui>
#include <qdockwidget.h>
#include <qcombobox.h>
#include <qmainwindow.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qtoolbar.h>

#include "Level.h"

class TilemapControl : public QDockWidget
{
	Q_OBJECT

public:
	TilemapControl(QWidget* parent, Level* level);
	~TilemapControl();

	void reset();

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void onClose();

private:
	Level* m_level;
	QMainWindow* m_window;
	QToolBar* m_toolbar;
};