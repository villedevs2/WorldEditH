#pragma once

#include <QtGui>
#include <qdialog.h>
#include <qformlayout.h>
#include <qspinbox.h>
#include <qdialogbuttonbox.h>

class VisboxConf : public QDialog
{
	Q_OBJECT

public:
	VisboxConf::VisboxConf(QWidget* parent);
	VisboxConf::~VisboxConf();

	float getWidth();
	float getHeight();

public slots:

signals:

private:
	QFormLayout* m_layout;
	QDialogButtonBox* m_button_box;
	QDoubleSpinBox* m_width_box;
	QDoubleSpinBox* m_height_box;
};
