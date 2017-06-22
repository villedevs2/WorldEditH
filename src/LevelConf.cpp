#include "LevelConf.h"

LevelConf::LevelConf(QWidget* parent) : QDialog(parent)
{
	this->setMinimumWidth(300);
	this->setMinimumHeight(200);

	/*
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
	*/

	m_layout = new QFormLayout(this);

	// time limit
	m_timelimit_layout = new QHBoxLayout();
	
	m_timelimit_button = new QPushButton(tr("X"), this);
	m_timelimit_button->setMaximumWidth(25);

	m_timelimit_mins = new QSpinBox(this);
	m_timelimit_mins->setMaximum(200);
	m_timelimit_mins->setMinimum(0);

	m_timelimit_secs = new QSpinBox(this);
	m_timelimit_secs->setMaximum(59);
	m_timelimit_secs->setMinimum(0);

	m_timelimit_layout->addWidget(m_timelimit_button);
	m_timelimit_layout->addWidget(m_timelimit_mins);
	m_timelimit_layout->addWidget(m_timelimit_secs);

	// collect 1
	m_collect1_layout = new QHBoxLayout();

	m_collect1_button = new QPushButton(tr("X"), this);
	m_collect1_button->setMaximumWidth(25);

	m_collect1_item = new QLineEdit(this);

	m_collect1_num = new QSpinBox(this);
	m_collect1_num->setMinimum(0);
	m_collect1_num->setMaximum(50000);

	m_collect1_layout->addWidget(m_collect1_button);
	m_collect1_layout->addWidget(m_collect1_item);
	m_collect1_layout->addWidget(m_collect1_num);

	// collect 2
	m_collect2_layout = new QHBoxLayout();

	m_collect2_button = new QPushButton(tr("X"), this);
	m_collect2_button->setMaximumWidth(25);

	m_collect2_item = new QLineEdit(this);

	m_collect2_num = new QSpinBox(this);
	m_collect2_num->setMinimum(0);
	m_collect2_num->setMaximum(50000);

	m_collect2_layout->addWidget(m_collect2_button);
	m_collect2_layout->addWidget(m_collect2_item);
	m_collect2_layout->addWidget(m_collect2_num);

	// collect 3
	m_collect3_layout = new QHBoxLayout();

	m_collect3_button = new QPushButton(tr("X"), this);
	m_collect3_button->setMaximumWidth(25);

	m_collect3_item = new QLineEdit(this);

	m_collect3_num = new QSpinBox(this);
	m_collect3_num->setMinimum(0);
	m_collect3_num->setMaximum(50000);

	m_collect3_layout->addWidget(m_collect3_button);
	m_collect3_layout->addWidget(m_collect3_item);
	m_collect3_layout->addWidget(m_collect3_num);

	// don't collect
	m_dontcollect_layout = new QHBoxLayout();

	m_dontcollect_button = new QPushButton(tr("X"), this);
	m_dontcollect_button->setMaximumWidth(25);

	m_dontcollect_item = new QLineEdit(this);

	m_dontcollect_layout->addWidget(m_dontcollect_button);
	m_dontcollect_layout->addWidget(m_dontcollect_item);

	// avoid hazards
	m_avoid_layout = new QHBoxLayout();

	m_avoid_button = new QPushButton(tr("X"), this);
	m_avoid_button->setMaximumWidth(25);

	m_avoid_layout->addWidget(m_avoid_button);

	// find exit
	m_exit_layout = new QHBoxLayout();

	m_exit_button = new QPushButton(tr("X"), this);
	m_exit_button->setMaximumWidth(25);

	m_exit_layout->addWidget(m_exit_button);

	// inverse gravity
	m_gravity_layout = new QHBoxLayout();

	m_gravity_button = new QPushButton(tr("X"), this);
	m_gravity_button->setMaximumWidth(25);

	m_gravity_layout->addWidget(m_gravity_button);


	// layout

	m_layout->addRow(QString("Time Limit:"), m_timelimit_layout);
	m_layout->addRow(QString("Collect:"), m_collect1_layout);
	m_layout->addRow(QString("Collect:"), m_collect2_layout);
	m_layout->addRow(QString("Collect:"), m_collect3_layout);
	m_layout->addRow(QString("Don't collect:"), m_dontcollect_layout);
	m_layout->addRow(QString("Avoid Hazards:"), m_avoid_layout);
	m_layout->addRow(QString("Find Exit:"), m_exit_layout);
	m_layout->addRow(QString("Inverse Gravity:"), m_gravity_layout);

	m_button_box = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	m_layout->addRow(m_button_box);

	connect(m_button_box, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_button_box, SIGNAL(rejected()), this, SLOT(reject()));

	connect(m_timelimit_button, SIGNAL(clicked()), this, SLOT(timelimitToggle()));
	connect(m_collect1_button, SIGNAL(clicked()), this, SLOT(collect1Toggle()));
	connect(m_collect2_button, SIGNAL(clicked()), this, SLOT(collect2Toggle()));
	connect(m_collect3_button, SIGNAL(clicked()), this, SLOT(collect3Toggle()));
	connect(m_dontcollect_button, SIGNAL(clicked()), this, SLOT(dontcollectToggle()));
	connect(m_avoid_button, SIGNAL(clicked()), this, SLOT(avoidToggle()));
	connect(m_exit_button, SIGNAL(clicked()), this, SLOT(exitToggle()));
	connect(m_gravity_button, SIGNAL(clicked()), this, SLOT(gravityToggle()));

	this->setWindowTitle(QString("Level settings"));

	timelimitEnable(false);
	collect1Enable(false);
	collect2Enable(false);
	collect3Enable(false);
	dontcollectEnable(false);
	avoidEnable(false);
	exitEnable(false);
	gravityEnable(false);
}

LevelConf::~LevelConf()
{
}

void LevelConf::timelimitEnable(bool enable)
{
	m_timelimit_mins->setEnabled(enable);
	m_timelimit_mins->setValue(0);
	m_timelimit_secs->setEnabled(enable);
	m_timelimit_secs->setValue(0);
	m_timelimit_button->setText(enable ? "X" : " ");
	m_timelimit_enable = enable;
}

void LevelConf::setTimelimit(unsigned int limit)
{
	m_timelimit_mins->setValue((limit >> 8) & 0xff);
	m_timelimit_secs->setValue(limit & 0xff);
}

void LevelConf::collect1Enable(bool enable)
{
	m_collect1_item->setEnabled(enable);
	m_collect1_item->setText("");
	m_collect1_num->setEnabled(enable);
	m_collect1_num->setValue(0);
	m_collect1_button->setText(enable ? "X" : " ");
	m_collect1_enable = enable;
}

void LevelConf::setCollect1Item(std::string item)
{
	m_collect1_item->setText(QString(item.c_str()));
}

void LevelConf::setCollect1Num(int num)
{
	m_collect1_num->setValue(num);
}

void LevelConf::collect2Enable(bool enable)
{
	m_collect2_item->setEnabled(enable);
	m_collect2_item->setText("");
	m_collect2_num->setEnabled(enable);
	m_collect2_num->setValue(0);
	m_collect2_button->setText(enable ? "X" : " ");
	m_collect2_enable = enable;
}

void LevelConf::setCollect2Item(std::string item)
{
	m_collect2_item->setText(QString(item.c_str()));
}

void LevelConf::setCollect2Num(int num)
{
	m_collect2_num->setValue(num);
}

void LevelConf::collect3Enable(bool enable)
{
	m_collect3_item->setEnabled(enable);
	m_collect3_item->setText("");
	m_collect3_num->setEnabled(enable);
	m_collect3_num->setValue(0);
	m_collect3_button->setText(enable ? "X" : " ");
	m_collect3_enable = enable;
}

void LevelConf::setCollect3Item(std::string item)
{
	m_collect3_item->setText(QString(item.c_str()));
}

void LevelConf::setCollect3Num(int num)
{
	m_collect3_num->setValue(num);
}

void LevelConf::dontcollectEnable(bool enable)
{
	m_dontcollect_item->setEnabled(enable);
	m_dontcollect_item->setText("");
	m_dontcollect_button->setText(enable ? "X" : " ");
	m_dontcollect_enable = enable;
}

void LevelConf::setDontcollectItem(std::string item)
{
	m_dontcollect_item->setText(QString(item.c_str()));
}

void LevelConf::avoidEnable(bool enable)
{
	m_avoid_button->setText(enable ? "X" : " ");
	m_avoid_enable = enable;
}

void LevelConf::exitEnable(bool enable)
{
	m_exit_button->setText(enable ? "X" : " ");
	m_exit_enable = enable;
}

void LevelConf::gravityEnable(bool enable)
{
	m_gravity_button->setText(enable ? "X" : " ");
	m_gravity_enable = enable;
}



void LevelConf::timelimitToggle()
{
	if (m_timelimit_enable)
		timelimitEnable(false);
	else
		timelimitEnable(true);
}

void LevelConf::collect1Toggle()
{
	if (m_collect1_enable)
		collect1Enable(false);
	else
		collect1Enable(true);
}

void LevelConf::collect2Toggle()
{
	if (m_collect2_enable)
		collect2Enable(false);
	else
		collect2Enable(true);
}

void LevelConf::collect3Toggle()
{
	if (m_collect3_enable)
		collect3Enable(false);
	else
		collect3Enable(true);
}

void LevelConf::dontcollectToggle()
{
	if (m_dontcollect_enable)
		dontcollectEnable(false);
	else
		dontcollectEnable(true);
}

void LevelConf::avoidToggle()
{
	if (m_avoid_enable)
		avoidEnable(false);
	else
		avoidEnable(true);
}

void LevelConf::exitToggle()
{
	if (m_exit_enable)
		exitEnable(false);
	else
		exitEnable(true);
}

void LevelConf::gravityToggle()
{
	if (m_gravity_enable)
		gravityEnable(false);
	else
		gravityEnable(true);
}




void LevelConf::getTimelimit(bool *enable, int *mins, int *secs)
{
	*enable = m_timelimit_enable;
	*mins = m_timelimit_mins->value();
	*secs = m_timelimit_secs->value();
}

void LevelConf::getCollect1(bool *enable, QString *item, int *num)
{
	*enable = m_collect1_enable;
	*item = m_collect1_item->text();
	*num = m_collect1_num->value();
}

void LevelConf::getCollect2(bool *enable, QString *item, int *num)
{
	*enable = m_collect2_enable;
	*item = m_collect2_item->text();
	*num = m_collect2_num->value();
}

void LevelConf::getCollect3(bool *enable, QString *item, int *num)
{
	*enable = m_collect3_enable;
	*item = m_collect3_item->text();
	*num = m_collect3_num->value();
}

void LevelConf::getDontcollect(bool *enable, QString *item)
{
	*enable = m_dontcollect_enable;
	*item = m_dontcollect_item->text();
}

void LevelConf::getAvoid(bool *enable)
{
	*enable = m_avoid_enable;
}

void LevelConf::getExit(bool *enable)
{
	*enable = m_exit_enable;
}

void LevelConf::getGravity(bool *enable)
{
	*enable = m_gravity_enable;
}