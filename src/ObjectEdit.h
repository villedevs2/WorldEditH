#pragma once

#include <QtGui>
#include <qdockwidget.h>
#include <qcombobox.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcolordialog.h>

#include "GLWidget.h"
#include "Level.h"

class ObjectEdit : public QDockWidget
{
	Q_OBJECT

public:
	ObjectEdit(QWidget* parent, Level* level);
	~ObjectEdit();

	void reset();

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void onClose();
	void onSetCreateType(Level::ObjectType type);
	void onSetCreateTriggerType(int type);

public slots:
	void setMode(GLWidget::OperationMode mode);
	void select(int object_id);
	void deselect();
	void remove(int object_id);
	void objectDataChanged(int object_id);
	void triggerAngleChanged(double value);
	void triggerParam1Changed(const QString& text);
	void triggerParam2Changed(const QString& text);
	void sliderAngleChanged(double value);
	void moverAngleChanged(double value);
	void moverLengthChanged(double value);
	void moverUptimeChanged(double value);
	void moverDowntimeChanged(double value);
	void moverTimeChanged(double value);
	void setDefaultType(Level::ObjectType type);
	void chooseColor();
	
private slots:
	void setType(int index);
	void setTrigger(int index);
	void setDepth(int depth);

private:
	enum TypeBoxItem
	{
		ITEM_TRIGGER		= 0,
		ITEM_DESTRUCTIBLE	= 1,
		ITEM_MOVER			= 2,
		ITEM_ENEMY			= 3,
	};

	Level* m_level;
	QMainWindow* m_window;
	QToolBar* m_toolbar;

	QWidget* m_type_select;
	QComboBox* m_type_box;
	QLabel* m_type_label;

	QWidget* m_depth_select;
	QSpinBox* m_depth_edit;
	QLabel* m_depth_label;

	QStackedWidget* m_stacked_widget;

	QWidget* m_empty_widget;
	QWidget* m_trigger_widget;
	QWidget* m_mover_widget;
	QWidget* m_enemy_widget;

	QWidget* m_color_widget;
	QPushButton* m_color_button;
	QLabel* m_color_label;

	QDoubleSpinBox* m_slider_angle;

	QComboBox* m_trigger_type;
	QDoubleSpinBox* m_trigger_angle;
	QLineEdit* m_trigger_param1;
	QLineEdit* m_trigger_param2;

	QDoubleSpinBox* m_mover_angle;
	QDoubleSpinBox* m_mover_length;
	QDoubleSpinBox* m_mover_uptime;
	QDoubleSpinBox* m_mover_downtime;
	QDoubleSpinBox* m_mover_time;

	QString m_title_text;

	int m_selected_object;

	GLWidget::OperationMode m_opmode;

	QColor m_object_color;

	TypeBoxItem m_default_type;
};