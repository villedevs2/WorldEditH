#include "TilemapWidget.h"

TilemapWidget::TilemapWidget(QWidget* parent, Level* level) : QDialog(parent, 0)
{
	m_level = level;

	setMinimumWidth(300);
	setMinimumHeight(88);	

	m_xstart = new QSpinBox(this);
	m_xstart->setRange(-10000,10000);
	m_xstart->setSingleStep(1);
	//m_xstart->setMaximumWidth(80);

	m_xend = new QSpinBox(this);
	m_xend->setRange(-10000,10000);
	m_xend->setSingleStep(1);
	//m_xend->setMaximumWidth(80);

	m_ystart = new QSpinBox(this);
	m_ystart->setRange(-10000,10000);
	m_ystart->setSingleStep(1);
	//m_ystart->setMaximumWidth(80);

	m_yend = new QSpinBox(this);
	m_yend->setRange(-10000,10000);
	m_yend->setSingleStep(1);
	//m_yend->setMaximumWidth(80);

	m_tile_width = new QDoubleSpinBox(this);
	m_tile_width->setRange(0.1, 100.0);
	m_tile_width->setSingleStep(0.1);

	m_tile_height = new QDoubleSpinBox(this);
	m_tile_height->setRange(0.1, 100.0);
	m_tile_height->setSingleStep(0.1);

	m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

	m_layout = new QFormLayout(this);
	m_layout->addRow(QString("X Start:"), m_xstart);
	m_layout->addRow(QString("X End:"), m_xend);
	m_layout->addRow(QString("Y Start:"), m_ystart);
	m_layout->addRow(QString("Y End:"), m_yend);
	m_layout->addRow(QString("Tile width:"), m_tile_width);
	m_layout->addRow(QString("Tile heighy:"), m_tile_height);
	m_layout->addRow(m_button_box);

	setWindowTitle(QString("Tilemap Settings"));

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

TilemapWidget::~TilemapWidget()
{
}


void TilemapWidget::configAccept()
{
	emit onConfigChanged();
}


int TilemapWidget::getXStart()
{
	return m_xstart->value();
}

int TilemapWidget::getXEnd()
{
	return m_xend->value();
}

int TilemapWidget::getYStart()
{
	return m_ystart->value();
}

int TilemapWidget::getYEnd()
{
	return m_yend->value();
}

void TilemapWidget::setValues(int xstart, int xend, int ystart, int yend)
{
	m_xstart->setValue(xstart);
	m_xend->setValue(xend);
	m_ystart->setValue(ystart);
	m_yend->setValue(yend);
}

float TilemapWidget::getTileWidth()
{
	return m_tile_width->value();
}

void TilemapWidget::setTileWidth(float width)
{
	m_tile_width->setValue(width);
}

float TilemapWidget::getTileHeight()
{
	return m_tile_height->value();
}

void TilemapWidget::setTileHeight(float height)
{
	m_tile_height->setValue(height);
}