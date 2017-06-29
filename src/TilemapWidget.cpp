#include "TilemapWidget.h"

TilemapEnlarge::TilemapEnlarge(QWidget* parent, Level* level) : QDialog(parent, 0)
{
	m_level = level;

	setMinimumWidth(300);
	setMinimumHeight(88);	

	m_xleft = new QSpinBox(this);
	m_xleft->setRange(0,10000);
	m_xleft->setSingleStep(1);

	m_xright = new QSpinBox(this);
	m_xright->setRange(0,10000);
	m_xright->setSingleStep(1);

	m_ytop = new QSpinBox(this);
	m_ytop->setRange(0,10000);
	m_ytop->setSingleStep(1);

	m_ybottom = new QSpinBox(this);
	m_ybottom->setRange(0,10000);
	m_ybottom->setSingleStep(1);

	m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	m_layout = new QFormLayout(this);
	m_layout->addRow(QString("X Left:"), m_xleft);
	m_layout->addRow(QString("X Right:"), m_xright);
	m_layout->addRow(QString("Y Left:"), m_ytop);
	m_layout->addRow(QString("Y Right:"), m_ybottom);
	m_layout->addRow(m_button_box);

	setWindowTitle(QString("Enlarge Tilemap"));

	connect(m_button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_button_box, SIGNAL(accepted()), this, SLOT(configAccept()));
	connect(m_button_box, SIGNAL(rejected()), this, SLOT(reject()));

	/*
	connect(m_xstart, SIGNAL(valueChanged(int)), this, SLOT(xstartChanged(int)));
	connect(m_xend, SIGNAL(valueChanged(int)), this, SLOT(xendChanged(int)));
	connect(m_ystart, SIGNAL(valueChanged(int)), this, SLOT(ystartChanged(int)));
	connect(m_yend, SIGNAL(valueChanged(int)), this, SLOT(yendChanged(int)));
	*/
}

TilemapEnlarge::~TilemapEnlarge()
{
}


void TilemapEnlarge::configAccept()
{
	emit onEnlarge();
}


int TilemapEnlarge::getXLeft()
{
	return m_xleft->value();
}

int TilemapEnlarge::getXRight()
{
	return m_xright->value();
}

int TilemapEnlarge::getYTop()
{
	return m_ytop->value();
}

int TilemapEnlarge::getYBottom()
{
	return m_ybottom->value();
}

void TilemapEnlarge::setValues(int xleft, int xright, int ytop, int ybottom)
{
	m_xleft->setValue(xleft);
	m_xright->setValue(xright);
	m_ytop->setValue(ytop);
	m_ybottom->setValue(ybottom);
}