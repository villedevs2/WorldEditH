#include "ObjectEdit.h"

ObjectEdit::ObjectEdit(QWidget* parent, Level* level) : QDockWidget("Object Edit", parent, 0)
{
	m_level = level;

	m_window = new QMainWindow(0);

	m_title_text = "Object Edit";

	

	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	setMinimumWidth(500);
	setMaximumWidth(500);
	setMinimumHeight(88);	
	setMaximumHeight(88);

	// don't allow docking
	setAllowedAreas(0);
	setHidden(false);

	// type selector
	// ------------------------------------------------------------------------
	m_type_box = new QComboBox(this);
	m_type_box->addItem("Trigger", QVariant(Level::OBJECT_TYPE_TRIGGER));
	m_type_box->addItem("Destructible", QVariant(Level::OBJECT_TYPE_DESTRUCTIBLE));
	m_type_box->addItem("Mover", QVariant(Level::OBJECT_TYPE_MOVER));
	m_type_box->addItem("Enemy", QVariant(Level::OBJECT_TYPE_ENEMY));
	m_type_box->setFocusPolicy(Qt::NoFocus);

	m_type_label = new QLabel("Type: ");

	m_type_select = new QWidget(this);
	QBoxLayout* type_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	type_layout->addWidget(m_type_label);
	type_layout->addWidget(m_type_box);
	type_layout->setMargin(2);
	type_layout->setSpacing(2);
	m_type_select->setLayout(type_layout);
	m_type_select->setMaximumHeight(35);
	
	connect(m_type_box, SIGNAL(activated(int)), this, SLOT(setType(int)));
	// ------------------------------------------------------------------------

	
	// depth select
	// ------------------------------------------------------------------------
	m_depth_edit = new QSpinBox(this);
	m_depth_edit->setRange(-100, 100);
	m_depth_edit->setSingleStep(1);
	m_depth_edit->setMaximumWidth(60);

	m_depth_label = new QLabel("Depth: ");

	m_depth_select = new QWidget(this);
	QBoxLayout* depth_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	depth_layout->addWidget(m_depth_label);
	depth_layout->addWidget(m_depth_edit);
	depth_layout->setMargin(2);
	depth_layout->setSpacing(2);
	m_depth_select->setLayout(depth_layout);
	m_depth_select->setMaximumHeight(35);

	connect(m_depth_edit, SIGNAL(valueChanged(int)), this, SLOT(setDepth(int)));
	// ------------------------------------------------------------------------


	// color
	// ------------------------------------------------------------------------
	m_color_button = new QPushButton("", this);
	m_color_button->setFocusPolicy(Qt::NoFocus);
	m_color_button->setStyleSheet(tr("background-color: #%1%2%3").arg(m_object_color.red(), 2, 16, QChar('0')).arg(m_object_color.green(), 2, 16, QChar('0')).arg(m_object_color.blue(), 2, 16, QChar('0')));
	m_color_button->setMaximumHeight(25);
	m_color_button->setMaximumWidth(25);
	connect(m_color_button, SIGNAL(clicked()), this, SLOT(chooseColor()));
	m_color_label = new QLabel("Color:");
	QBoxLayout* color_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	color_layout->setSpacing(2);
	color_layout->setMargin(1);
	color_layout->addWidget(m_color_label);
	color_layout->addWidget(m_color_button);

	m_color_widget = new QWidget;
	m_color_widget->setMaximumHeight(30);
	m_color_widget->setLayout(color_layout);


	
	m_empty_widget = new QWidget;

	// trigger widget
	// ------------------------------------------------------------------------
	m_trigger_widget = new QWidget;

	m_trigger_type = new QComboBox;
	for (int i=0; i < m_level->getNumTriggers(); i++)
	{
		m_trigger_type->addItem(m_level->getTriggerName(i).c_str());
	}

	QLabel* trigger_label = new QLabel("Type: ");
	trigger_label->setMaximumWidth(40);

	QLabel* trigger_angle_label = new QLabel("Angle: ");
	trigger_angle_label->setMaximumWidth(40);
	
	m_trigger_angle = new QDoubleSpinBox();
	m_trigger_angle->setRange(0.0, 360.0);
	m_trigger_angle->setSingleStep(5.0);
	m_trigger_angle->setMaximumWidth(75);

	QLabel* trigger_param1_label = new QLabel("Param 1: ");
	trigger_param1_label->setMaximumWidth(40);

	m_trigger_param1 = new QLineEdit();
	m_trigger_param1->setValidator(new QIntValidator());

	QLabel* trigger_param2_label = new QLabel("Param 2: ");
	trigger_param2_label->setMaximumWidth(40);

	m_trigger_param2 = new QLineEdit();
	m_trigger_param2->setValidator(new QIntValidator());

	QGridLayout* trigger_layout = new QGridLayout();
	trigger_layout->addWidget(trigger_label,		0, 0, 1, 1);
	trigger_layout->addWidget(m_trigger_type,		0, 1, 1, 1);
	trigger_layout->addWidget(trigger_angle_label,	0, 2, 1, 1);
	trigger_layout->addWidget(m_trigger_angle,		0, 3, 1, 1);
	trigger_layout->addWidget(trigger_param1_label, 1, 0, 1, 1);
	trigger_layout->addWidget(m_trigger_param1,		1, 1, 1, 1);
	trigger_layout->addWidget(trigger_param2_label, 1, 2, 1, 1);
	trigger_layout->addWidget(m_trigger_param2,		1, 3, 1, 1);

	m_trigger_widget->setLayout(trigger_layout);

	connect(m_trigger_type, SIGNAL(activated(int)), this, SLOT(setTrigger(int)));
	connect(m_trigger_angle, SIGNAL(valueChanged(double)), this, SLOT(triggerAngleChanged(double)));
	connect(m_trigger_param1, SIGNAL(textChanged(const QString&)), this, SLOT(triggerParam1Changed(const QString&)));
	connect(m_trigger_param2, SIGNAL(textChanged(const QString&)), this, SLOT(triggerParam2Changed(const QString&)));
	
	// ------------------------------------------------------------------------

	// mover widget
	// ------------------------------------------------------------------------
	m_mover_widget = new QWidget;

	QLabel* mover_angle_label = new QLabel("Angle: ");
	mover_angle_label->setMaximumWidth(55);

	m_mover_angle = new QDoubleSpinBox();
	m_mover_angle->setRange(0.0, 360.0);
	m_mover_angle->setSingleStep(5.0);

	QLabel* mover_length_label = new QLabel("Length: ");
	mover_length_label->setMaximumWidth(80);
	
	m_mover_length = new QDoubleSpinBox();
	m_mover_length->setRange(0.0, 100.0);
	m_mover_length->setSingleStep(1.0);

	QLabel* mover_uptime_label = new QLabel("Up time: ");
	mover_uptime_label->setMaximumWidth(55);

	m_mover_uptime = new QDoubleSpinBox();
	m_mover_uptime->setRange(0.0, 100.0);
	m_mover_uptime->setSingleStep(1.0);

	QLabel* mover_downtime_label = new QLabel("Down time: ");
	mover_downtime_label->setMaximumWidth(55);

	m_mover_downtime = new QDoubleSpinBox();
	m_mover_downtime->setRange(0.0, 100.0);
	m_mover_downtime->setSingleStep(1.0);

	QLabel* mover_time_label = new QLabel("Time: ");
	mover_time_label->setMaximumWidth(55);

	m_mover_time = new QDoubleSpinBox();
	m_mover_time->setRange(0.0, 100.0);
	m_mover_time->setSingleStep(1.0);

	QGridLayout* mover_layout = new QGridLayout();
	mover_layout->addWidget(mover_angle_label,		0, 0, 1, 1);
	mover_layout->addWidget(m_mover_angle,			0, 1, 1, 1);
	mover_layout->addWidget(mover_length_label,		0, 2, 1, 1);
	mover_layout->addWidget(m_mover_length,			0, 3, 1, 1);
	mover_layout->addWidget(mover_uptime_label,		1, 0, 1, 1);
	mover_layout->addWidget(m_mover_uptime,			1, 1, 1, 1);
	mover_layout->addWidget(mover_downtime_label	,	1, 2, 1, 1);
	mover_layout->addWidget(m_mover_downtime,		1, 3, 1, 1);
	mover_layout->addWidget(mover_time_label,		1, 4, 1, 1);
	mover_layout->addWidget(m_mover_time,			1, 5, 1, 1);

	m_mover_widget->setLayout(mover_layout);

	connect(m_mover_angle, SIGNAL(valueChanged(double)), this, SLOT(moverAngleChanged(double)));
	connect(m_mover_length, SIGNAL(valueChanged(double)), this, SLOT(moverLengthChanged(double)));
	connect(m_mover_uptime, SIGNAL(valueChanged(double)), this, SLOT(moverUptimeChanged(double)));
	connect(m_mover_downtime, SIGNAL(valueChanged(double)), this, SLOT(moverDowntimeChanged(double)));
	connect(m_mover_time, SIGNAL(valueChanged(double)), this, SLOT(moverTimeChanged(double)));

	// ------------------------------------------------------------------------

	// mover widget
	// ------------------------------------------------------------------------

	m_enemy_widget = new QWidget;

	// TODO: path and stuff

	// ------------------------------------------------------------------------

	m_stacked_widget = new QStackedWidget(this);
	m_stacked_widget->addWidget(m_empty_widget);
	m_stacked_widget->addWidget(m_trigger_widget);
	m_stacked_widget->addWidget(m_mover_widget);
	m_stacked_widget->addWidget(m_enemy_widget);


	m_toolbar = new QToolBar(m_window);
	m_toolbar->addWidget(m_type_select);
	m_toolbar->addWidget(m_depth_select);
	m_toolbar->addWidget(m_color_widget);
	m_toolbar->setFloatable(false);
	m_toolbar->setMovable(false);
	m_window->addToolBar(m_toolbar);


	m_window->setParent(this);
	setWidget(m_window);

	m_window->setCentralWidget(m_stacked_widget);

	m_default_type = ITEM_TRIGGER;

	reset();
}

ObjectEdit::~ObjectEdit()
{
}


void ObjectEdit::reset()
{
	m_selected_object = -1;
}


// Slots
// ----------------------------------------------------------------------------

void ObjectEdit::setDefaultType(Level::ObjectType type)
{
	switch (type)
	{
		case Level::OBJECT_TYPE_TRIGGER:		m_default_type = ITEM_TRIGGER; break;
		case Level::OBJECT_TYPE_DESTRUCTIBLE:	m_default_type = ITEM_DESTRUCTIBLE; break;
		case Level::OBJECT_TYPE_MOVER:			m_default_type = ITEM_MOVER; break;
		case Level::OBJECT_TYPE_ENEMY:			m_default_type = ITEM_ENEMY; break;
		default: m_default_type = ITEM_TRIGGER; break;
	}
}

void ObjectEdit::setMode(GLWidget::OperationMode mode)
{
	m_opmode = mode;

	m_depth_edit->setEnabled(false);

	switch (mode)
	{
		case GLWidget::MODE_DRAW_POLY:
			this->setWindowTitle(m_title_text + " - Poly draw");
			m_type_box->setEnabled(true);
			m_type_box->setCurrentIndex(m_default_type);
			setType(m_default_type);
			m_stacked_widget->setCurrentWidget(m_empty_widget);
			break;

		case GLWidget::MODE_DRAW_RECT:
			this->setWindowTitle(m_title_text + " - Rect draw");
			m_type_box->setEnabled(true);
			m_type_box->setCurrentIndex(m_default_type);
			setType(m_default_type);
			m_stacked_widget->setCurrentWidget(m_empty_widget);
			break;

		case GLWidget::MODE_MOVE:
			m_type_box->setEnabled(false);
			break;

		case GLWidget::MODE_ROTATE:
			m_type_box->setEnabled(false);
			break;

		case GLWidget::MODE_SCALE:
			m_type_box->setEnabled(false);
			break;

		case GLWidget::MODE_SELECT:
			m_type_box->setEnabled(false);
			break;
	}
}

void ObjectEdit::select(int object_id)
{
	m_selected_object = m_level->getIndexById(object_id);
	if (m_selected_object >= 0)
	{
		Level::Object* obj = m_level->getObject(m_selected_object);
		this->setWindowTitle(m_title_text + " - " + QString(obj->getName().c_str()));
		
		int z = obj->getZ();
		m_depth_edit->setEnabled(true);
		m_depth_edit->setValue(z);

		m_type_box->setEnabled(true);

		m_color_button->setEnabled(true);
		unsigned int cc = obj->getColor();
		m_object_color = QColor(cc & 0xff, (cc >> 8) & 0xff, (cc >> 16) & 0xff);
		m_color_button->setStyleSheet(tr("background-color: #%1%2%3").arg(m_object_color.red(), 2, 16, QChar('0')).arg(m_object_color.green(), 2, 16, QChar('0')).arg(m_object_color.blue(), 2, 16, QChar('0')));

		switch (obj->getType())
		{
			case Level::OBJECT_TYPE_DESTRUCTIBLE:
			{
				m_type_box->setCurrentIndex(ITEM_DESTRUCTIBLE);
				m_stacked_widget->setCurrentWidget(m_empty_widget);
				break;
			}

			case Level::OBJECT_TYPE_TRIGGER:
			{
				m_type_box->setCurrentIndex(ITEM_TRIGGER);
				m_stacked_widget->setCurrentWidget(m_trigger_widget);

				Level::Object::Param trigger = obj->getParam(0);
				Level::Object::Param angle = obj->getParam(1);
				Level::Object::Param param1 = obj->getParam(2);
				Level::Object::Param param2 = obj->getParam(3);

				assert(trigger.i >= 0 && trigger.i < m_trigger_type->count());

				m_trigger_type->setCurrentIndex(trigger.i);
				m_trigger_angle->setValue((double)(angle.f));
				m_trigger_param1->setText(QString("%1").arg(param1.i));
				m_trigger_param2->setText(QString("%1").arg(param2.i));
				break;
			}

			case Level::OBJECT_TYPE_MOVER:
			{
				m_type_box->setCurrentIndex(ITEM_MOVER);
				m_stacked_widget->setCurrentWidget(m_mover_widget);

				Level::Object::Param angle = obj->getParam(0);
				Level::Object::Param length = obj->getParam(1);
				Level::Object::Param uptime = obj->getParam(2);
				Level::Object::Param downtime = obj->getParam(3);
				Level::Object::Param time = obj->getParam(4);

				m_mover_angle->setValue((double)(angle.f));
				m_mover_length->setValue((double)(length.f));
				m_mover_uptime->setValue((double)(uptime.f));
				m_mover_downtime->setValue((double)(downtime.f));
				m_mover_time->setValue((double)(time.f));
				break;
			}

			case Level::OBJECT_TYPE_ENEMY:
			{
				m_type_box->setCurrentIndex(ITEM_ENEMY);
				m_stacked_widget->setCurrentWidget(m_enemy_widget);

				// TODO: Stuff

				break;
			}
		}
	}
}

void ObjectEdit::deselect()
{
	m_selected_object = -1;

	this->setWindowTitle(m_title_text + " - No object selected");

	m_type_box->setEnabled(false);
	m_type_box->setCurrentIndex(-1);
	
	m_depth_edit->setEnabled(false);
	m_depth_edit->clear();
	
	m_color_button->setEnabled(false);
	m_color_button->setStyleSheet(tr("background-color: #AFAFAF"));

	m_stacked_widget->setCurrentWidget(m_empty_widget);
}

void ObjectEdit::remove(int object_id)
{
	deselect();
}

void ObjectEdit::objectDataChanged(int object_id)
{
	if (m_selected_object >= 0)
	{
		int object = m_level->getIndexById(object_id);
		if (object == m_selected_object)
		{
			select(object_id);
		}
	}
}



void ObjectEdit::sliderAngleChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(0, param);
	}
}

void ObjectEdit::triggerAngleChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(1, param);
	}
}

void ObjectEdit::triggerParam1Changed(const QString& text)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.i = text.toInt();
		m_level->getObject(m_selected_object)->setParam(2, param);
	}
}

void ObjectEdit::triggerParam2Changed(const QString& text)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.i = text.toInt();
		m_level->getObject(m_selected_object)->setParam(3, param);
	}
}

void ObjectEdit::moverAngleChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(0, param);
	}
}

void ObjectEdit::moverLengthChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(1, param);
	}
}

void ObjectEdit::moverUptimeChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(2, param);
	}
}

void ObjectEdit::moverDowntimeChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(3, param);
	}
}

void ObjectEdit::moverTimeChanged(double value)
{
	if (m_selected_object >= 0)
	{
		Level::Object::Param param;
		param.f = (float)(value);
		m_level->getObject(m_selected_object)->setParam(4, param);
	}
}


void ObjectEdit::setType(int index)
{
	if (m_selected_object >= 0)
	{
		Level::Object* obj = m_level->getObject(m_selected_object);
		Level::ObjectType current_type = obj->getType();

		Level::ObjectType new_type = Level::OBJECT_TYPE_INVALID;

		switch (index)
		{
			case ITEM_TRIGGER:			new_type = Level::OBJECT_TYPE_TRIGGER; break;
			case ITEM_DESTRUCTIBLE:		new_type = Level::OBJECT_TYPE_DESTRUCTIBLE; break;
			case ITEM_MOVER:			new_type = Level::OBJECT_TYPE_MOVER; break;
			case ITEM_ENEMY:			new_type = Level::OBJECT_TYPE_ENEMY; break;
		}

		assert(new_type != Level::OBJECT_TYPE_INVALID);

		if (current_type != new_type)
		{
			obj->setType(new_type);

			// update editor interface for the new type
			select(obj->getId());
		}
	}
	else if (m_opmode == GLWidget::MODE_DRAW_POLY ||
			 m_opmode == GLWidget::MODE_DRAW_RECT)
	{
		Level::ObjectType new_type = Level::OBJECT_TYPE_INVALID;

		switch (index)
		{
			case ITEM_TRIGGER:		new_type = Level::OBJECT_TYPE_TRIGGER; break;
			case ITEM_DESTRUCTIBLE:	new_type = Level::OBJECT_TYPE_DESTRUCTIBLE; break;
			case ITEM_MOVER:		new_type = Level::OBJECT_TYPE_MOVER; break;
			case ITEM_ENEMY:		new_type = Level::OBJECT_TYPE_ENEMY; break;
		}

		assert(new_type != Level::OBJECT_TYPE_INVALID);

		emit onSetCreateType(new_type);

		switch (new_type)
		{
			case Level::OBJECT_TYPE_DESTRUCTIBLE:
				m_stacked_widget->setCurrentWidget(m_empty_widget);
				break;

			case Level::OBJECT_TYPE_TRIGGER:
				m_stacked_widget->setCurrentWidget(m_trigger_widget);
				break;

			case Level::OBJECT_TYPE_MOVER:
				m_stacked_widget->setCurrentWidget(m_mover_widget);
				break;

			case Level::OBJECT_TYPE_ENEMY:
				m_stacked_widget->setCurrentWidget(m_enemy_widget);
		}
	}
}

void ObjectEdit::setDepth(int depth)
{
	if (m_selected_object >= 0)
	{
		Level::Object* obj = m_level->getObject(m_selected_object);

		obj->setZ(depth);
	}
}

void ObjectEdit::setTrigger(int index)
{
	if (m_selected_object >= 0)
	{
		Level::Object* obj = m_level->getObject(m_selected_object);
		Level::Object::Param param = obj->getParam(0);

		if (index != param.i)
		{
			param.i = index;
			obj->setParam(0, param);
		}
	}
	else if (m_opmode == GLWidget::MODE_DRAW_POLY ||
			 m_opmode == GLWidget::MODE_DRAW_RECT)
	{
		emit onSetCreateTriggerType(index);
	}
}

void ObjectEdit::chooseColor()
{
	QColor result = QColorDialog::getColor(m_object_color, this, tr("Select object color"));
	if (result.isValid())
	{
		m_color_button->setStyleSheet(tr("background-color: #%1%2%3").arg(result.red(), 2, 16, QChar('0')).arg(result.green(), 2, 16, QChar('0')).arg(result.blue(), 2, 16, QChar('0')));

		m_object_color = result;
		if (m_selected_object >= 0)
		{
			Level::Object* obj = m_level->getObject(m_selected_object);

			unsigned int cc = 0xff000000 | m_object_color.red() | (m_object_color.green() << 8) | (m_object_color.blue() << 16);
			obj->setColor(cc);
		}
	}
}


// ----------------------------------------------------------------------------

void ObjectEdit::closeEvent(QCloseEvent* event)
{
	emit onClose();
}