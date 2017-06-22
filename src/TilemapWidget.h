#pragma once

#include <qspinbox.h>
#include <qdialog.h>
#include <qformlayout.h>
#include <qdialogbuttonbox.h>

#include "Level.h"

class TilemapWidget : public QDialog
{
	Q_OBJECT

public:
	TilemapWidget(QWidget* parent, Level*  level);
	~TilemapWidget();

	int getXStart();
	int getXEnd();
	int getYStart();
	int getYEnd();
	void setValues(int xstart, int xend, int ystart, int yend);
	float getTileWidth();
	void setTileWidth(float width);
	float getTileHeight();
	void setTileHeight(float height);

signals:
	void onConfigChanged();

private slots:
	void configAccept();

private:
	Level* m_level;

	QFormLayout* m_layout;

	QSpinBox* m_xstart;
	QSpinBox* m_xend;
	QSpinBox* m_ystart;
	QSpinBox* m_yend;
	QDoubleSpinBox* m_tile_width;
	QDoubleSpinBox* m_tile_height;

	QDialogButtonBox* m_button_box;
};