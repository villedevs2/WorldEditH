#pragma once

#include <QtGui>
#include <qdialog.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qdialogbuttonbox.h>

class LevelConf : public QDialog
{
	Q_OBJECT

public:
	LevelConf::LevelConf(QWidget *parent);
	LevelConf::~LevelConf();

	void getTimelimit(bool *enable, int *mins, int *secs);
	void getCollect1(bool *enable, QString *item, int *num);
	void getCollect2(bool *enable, QString *item, int *num);
	void getCollect3(bool *enable, QString *item, int *num);
	void getDontcollect(bool *enable, QString *item);
	void getAvoid(bool *enable);
	void getExit(bool *enable);
	void getGravity(bool *enable);

public slots:
	void timelimitEnable(bool enable);
	void setTimelimit(unsigned int limit);
	void collect1Enable(bool enable);
	void setCollect1Item(std::string item);
	void setCollect1Num(int num);
	void collect2Enable(bool enable);
	void setCollect2Item(std::string item);
	void setCollect2Num(int num);
	void collect3Enable(bool enable);
	void setCollect3Item(std::string item);
	void setCollect3Num(int num);
	void dontcollectEnable(bool enable);
	void setDontcollectItem(std::string item);
	void avoidEnable(bool enable);
	void exitEnable(bool enable);
	void gravityEnable(bool enable);

private slots:
	void timelimitToggle();
	void collect1Toggle();
	void collect2Toggle();
	void collect3Toggle();
	void dontcollectToggle();
	void avoidToggle();
	void exitToggle();
	void gravityToggle();

private:
	bool m_timelimit_enable;
	bool m_collect1_enable;
	bool m_collect2_enable;
	bool m_collect3_enable;
	bool m_dontcollect_enable;
	bool m_avoid_enable;
	bool m_exit_enable;
	bool m_gravity_enable;

	QFormLayout *m_layout;
	QDialogButtonBox* m_button_box;

	QHBoxLayout *m_timelimit_layout;
	QPushButton *m_timelimit_button;
	QSpinBox *m_timelimit_mins;
	QSpinBox *m_timelimit_secs;

	QHBoxLayout *m_collect1_layout;
	QPushButton *m_collect1_button;
	QLineEdit *m_collect1_item;
	QSpinBox *m_collect1_num;

	QHBoxLayout *m_collect2_layout;
	QPushButton *m_collect2_button;
	QLineEdit *m_collect2_item;
	QSpinBox *m_collect2_num;

	QHBoxLayout *m_collect3_layout;
	QPushButton *m_collect3_button;
	QLineEdit *m_collect3_item;
	QSpinBox *m_collect3_num;

	QHBoxLayout *m_dontcollect_layout;
	QPushButton *m_dontcollect_button;
	QLineEdit *m_dontcollect_item;

	QHBoxLayout *m_avoid_layout;
	QPushButton *m_avoid_button;

	QHBoxLayout *m_exit_layout;
	QPushButton *m_exit_button;

	QHBoxLayout *m_gravity_layout;
	QPushButton *m_gravity_button;
};