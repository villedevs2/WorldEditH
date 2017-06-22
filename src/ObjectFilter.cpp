#include "ObjectFilter.h"

ObjectFilter::ObjectFilter(QWidget* parent) : QDockWidget("Object Filter", parent, 0)
{
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	setMinimumWidth(400);
	setMaximumWidth(400);
	setMinimumHeight(80);	
	setMaximumHeight(80);

	m_widget = new QWidget(this);

	// don't allow docking
	setAllowedAreas(0);
	setHidden(false);

	// labels
	m_edit_label = new QLabel("Edit:");
	m_show_label = new QLabel("Show:");
	
	m_collision_label = new QLabel("Collision");
	m_geovis_label = new QLabel("Geovis");
	m_tilemap_label = new QLabel("Tilemap");
	m_trigger_label = new QLabel("Trigger");
	m_slider_label = new QLabel("Slider");
	m_dest_label = new QLabel("Destr.");
	m_mover_label = new QLabel("Mover");

	// checkboxes
	m_edit_collision = new QCheckBox();
	m_edit_geovis = new QCheckBox();
	m_edit_trigger = new QCheckBox();
	m_edit_slider = new QCheckBox();
	m_edit_dest = new QCheckBox();
	m_edit_mover = new QCheckBox();
	m_show_collision = new QCheckBox();
	m_show_geovis = new QCheckBox();
	m_show_tilemap = new QCheckBox();
	m_show_trigger = new QCheckBox();
	m_show_slider = new QCheckBox();
	m_show_dest = new QCheckBox();
	m_show_mover = new QCheckBox();

	connect(m_edit_collision, SIGNAL(stateChanged(int)), this, SLOT(editCollision(int)));
	connect(m_edit_geovis, SIGNAL(stateChanged(int)), this, SLOT(editGeovis(int)));
	connect(m_edit_trigger, SIGNAL(stateChanged(int)), this, SLOT(editTrigger(int)));
	connect(m_edit_slider, SIGNAL(stateChanged(int)), this, SLOT(editSlider(int)));
	connect(m_edit_dest, SIGNAL(stateChanged(int)), this, SLOT(editDest(int)));
	connect(m_edit_mover, SIGNAL(stateChanged(int)), this, SLOT(editMover(int)));

	connect(m_show_collision, SIGNAL(stateChanged(int)), this, SLOT(showCollision(int)));
	connect(m_show_geovis, SIGNAL(stateChanged(int)), this, SLOT(showGeovis(int)));
	connect(m_show_trigger, SIGNAL(stateChanged(int)), this, SLOT(showTrigger(int)));
	connect(m_show_slider, SIGNAL(stateChanged(int)), this, SLOT(showSlider(int)));
	connect(m_show_dest, SIGNAL(stateChanged(int)), this, SLOT(showDest(int)));
	connect(m_show_mover, SIGNAL(stateChanged(int)), this, SLOT(showMover(int)));


	m_layout = new QGridLayout();
	
	m_layout->addWidget(m_edit_label, 1, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);
	m_layout->addWidget(m_show_label, 2, 0, 1, 1, Qt::AlignLeft|Qt::AlignVCenter);

	m_layout->addWidget(m_collision_label, 0, 1, Qt::AlignHCenter);
	m_layout->addWidget(m_geovis_label,    0, 2, Qt::AlignHCenter);
	m_layout->addWidget(m_tilemap_label,	   0, 3, Qt::AlignHCenter);
	m_layout->addWidget(m_trigger_label,   0, 4, Qt::AlignHCenter);
	m_layout->addWidget(m_slider_label,    0, 5, Qt::AlignHCenter);
	m_layout->addWidget(m_dest_label,      0, 6, Qt::AlignHCenter);
	m_layout->addWidget(m_mover_label,     0, 7, Qt::AlignHCenter);

	m_layout->addWidget(m_edit_collision,  1, 1, Qt::AlignHCenter);
	m_layout->addWidget(m_edit_geovis,     1, 2, Qt::AlignHCenter);
	m_layout->addWidget(m_edit_trigger,    1, 4, Qt::AlignHCenter);
	m_layout->addWidget(m_edit_slider,     1, 5, Qt::AlignHCenter);
	m_layout->addWidget(m_edit_dest,       1, 6, Qt::AlignHCenter);
	m_layout->addWidget(m_edit_mover,      1, 7, Qt::AlignHCenter);

	m_layout->addWidget(m_show_collision,  2, 1, Qt::AlignHCenter);
	m_layout->addWidget(m_show_geovis,     2, 2, Qt::AlignHCenter);
	m_layout->addWidget(m_show_tilemap,    2, 3, Qt::AlignHCenter);
	m_layout->addWidget(m_show_trigger,    2, 4, Qt::AlignHCenter);
	m_layout->addWidget(m_show_slider,     2, 5, Qt::AlignHCenter);
	m_layout->addWidget(m_show_dest,       2, 6, Qt::AlignHCenter);
	m_layout->addWidget(m_show_mover,      2, 7, Qt::AlignHCenter);

	m_widget->setLayout(m_layout);
	setWidget(m_widget);
}

ObjectFilter::~ObjectFilter()
{
}

// ----------------------------------------------------------------------------

void ObjectFilter::closeEvent(QCloseEvent* event)
{
	emit onClose();
}



void ObjectFilter::setFilters(int filter)
{
	m_edit_collision->setChecked((filter & 0x01) ? true : false);
	m_edit_geovis->setChecked((filter & 0x02) ? true : false);
	m_edit_trigger->setChecked((filter & 0x04) ? true : false);
	m_edit_slider->setChecked((filter & 0x08) ? true : false);
	m_edit_dest->setChecked((filter & 0x10) ? true : false);
	m_edit_mover->setChecked((filter & 0x20) ? true : false);

	onFilterEnable(filter);
}

void ObjectFilter::setDisplays(int filter)
{
	m_show_collision->setChecked((filter & 0x01) ? true : false);
	m_show_geovis->setChecked((filter & 0x02) ? true : false);
	m_show_trigger->setChecked((filter & 0x04) ? true : false);
	m_show_slider->setChecked((filter & 0x08) ? true : false);
	m_show_dest->setChecked((filter & 0x10) ? true : false);
	m_show_mover->setChecked((filter & 0x20) ? true : false);
	m_show_tilemap->setChecked((filter & 0x40) ? true : false);

	onDisplayEnable(filter);
}



void ObjectFilter::editCollision(int state)
{
	if (state == Qt::Checked)
		emit onFilterEnable(0x01);
	else
		emit onFilterDisable(0x01);
}

void ObjectFilter::editGeovis(int state)
{
	if (state == Qt::Checked)
		emit onFilterEnable(0x02);
	else
		emit onFilterDisable(0x02);
}

void ObjectFilter::editTrigger(int state)
{
	if (state == Qt::Checked)
		emit onFilterEnable(0x04);
	else
		emit onFilterDisable(0x04);
}

void ObjectFilter::editSlider(int state)
{
	if (state == Qt::Checked)
		emit onFilterEnable(0x08);
	else
		emit onFilterDisable(0x08);
}

void ObjectFilter::editDest(int state)
{
	if (state == Qt::Checked)
		emit onFilterEnable(0x10);
	else
		emit onFilterDisable(0x10);
}

void ObjectFilter::editMover(int state)
{
	if (state == Qt::Checked)
		emit onFilterEnable(0x20);
	else
		emit onFilterDisable(0x20);
}

void ObjectFilter::showCollision(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x01);

		m_edit_collision->setEnabled(true);
		editCollision(m_edit_collision->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	else
	{
		emit onDisplayDisable(0x01);
		
		m_edit_collision->setEnabled(false);
		editCollision(Qt::Unchecked);
	}
}

void ObjectFilter::showGeovis(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x02);

		m_edit_geovis->setEnabled(true);
		editGeovis(m_edit_geovis->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	else
	{
		emit onDisplayDisable(0x02);

		m_edit_geovis->setEnabled(false);
		editGeovis(Qt::Unchecked);
	}
}

void ObjectFilter::showTrigger(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x04);

		m_edit_trigger->setEnabled(true);
		editTrigger(m_edit_trigger->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	else
	{
		emit onDisplayDisable(0x04);

		m_edit_trigger->setEnabled(false);
		editTrigger(Qt::Unchecked);
	}
}

void ObjectFilter::showSlider(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x08);

		m_edit_slider->setEnabled(true);
		editSlider(m_edit_slider->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	else
	{
		emit onDisplayDisable(0x08);

		m_edit_slider->setEnabled(false);
		editSlider(Qt::Unchecked);
	}
}

void ObjectFilter::showDest(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x10);

		m_edit_dest->setEnabled(true);
		editDest(m_edit_dest->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	else
	{
		emit onDisplayDisable(0x10);

		m_edit_dest->setEnabled(false);
		editDest(Qt::Unchecked);
	}
}

void ObjectFilter::showMover(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x20);

		m_edit_mover->setEnabled(true);
		editMover(m_edit_mover->isChecked() ? Qt::Checked : Qt::Unchecked);
	}
	else
	{
		emit onDisplayDisable(0x20);

		m_edit_mover->setEnabled(false);
		editMover(Qt::Unchecked);
	}
}

void ObjectFilter::showTilemap(int state)
{
	if (state == Qt::Checked)
	{
		emit onDisplayEnable(0x40);
	}
	else
	{
		emit onDisplayDisable(0x40);
	}
}