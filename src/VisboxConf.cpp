#include "VisboxConf.h"

VisboxConf::VisboxConf(QWidget* parent) : QDialog(parent)
{
	this->setMinimumWidth(200);
	this->setMinimumHeight(90);

	m_layout = new QFormLayout(this);

	m_width_box = new QDoubleSpinBox();
	m_width_box->setRange(1.0, 100.0);
	m_width_box->setValue(32.0);

	m_height_box = new QDoubleSpinBox();
	m_height_box->setRange(1.0, 100.0);
	m_height_box->setValue(20.0);

	m_layout->addRow(QString("Width:"), m_width_box);
	m_layout->addRow(QString("Height:"), m_height_box);

	m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	m_layout->addRow(m_button_box);

	connect(m_button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_button_box, SIGNAL(rejected()), this, SLOT(reject()));
}

VisboxConf::~VisboxConf()
{
	delete m_layout;
	delete m_width_box;
	delete m_height_box;
	delete m_button_box;
}

float VisboxConf::getWidth()
{
	return m_width_box->value();
}

float VisboxConf::getHeight()
{
	return m_height_box->value();
}