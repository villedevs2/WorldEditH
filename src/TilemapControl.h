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
	void onSelectTilemap(Level::TilemapType map);

private slots:
	void selectTilemap(int map);

private:
	Level* m_level;
	QMainWindow* m_window;
	QToolBar* m_toolbar;

	QLabel* m_tilemap_label;
	QComboBox* m_tilemap_combo;
	QWidget* m_tilemap_widget;
};