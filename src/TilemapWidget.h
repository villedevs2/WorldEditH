#pragma once

#include <qspinbox.h>
#include <qdialog.h>
#include <qformlayout.h>
#include <qdialogbuttonbox.h>

#include "Level.h"

class TilemapEnlarge : public QDialog
{
	Q_OBJECT

public:
	TilemapEnlarge(QWidget* parent, Level*  level);
	~TilemapEnlarge();

	int getXLeft();
	int getXRight();
	int getYTop();
	int getYBottom();
	void setValues(int xstart, int xend, int ystart, int yend);

signals:
	void onEnlarge();

private slots:
	void configAccept();

private:
	Level* m_level;

	QFormLayout* m_layout;

	QSpinBox* m_xleft;
	QSpinBox* m_xright;
	QSpinBox* m_ytop;
	QSpinBox* m_ybottom;

	QDialogButtonBox* m_button_box;
};