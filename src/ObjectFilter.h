#pragma once

#include <QtGui>
#include <qdockwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgridlayout.h>

class ObjectFilter : public QDockWidget
{
	Q_OBJECT

public:
	ObjectFilter(QWidget* parent);
	~ObjectFilter();

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void onClose();
	void onFilterEnable(int filter);
	void onFilterDisable(int filter);
	void onDisplayEnable(int filter);
	void onDisplayDisable(int filter);

public slots:
	void setFilters(int filter);
	void setDisplays(int filter);
	void editCollision(int state);
	void editGeovis(int state);
	void editTrigger(int state);
	void editSlider(int state);
	void editDest(int state);
	void editMover(int state);
	void showCollision(int state);
	void showGeovis(int state);
	void showTrigger(int state);
	void showSlider(int state);
	void showDest(int state);
	void showMover(int state);
	void showTilemap(int state);

private:
	QWidget* m_widget;
	QGridLayout* m_layout;

	QLabel* m_edit_label;
	QLabel* m_show_label;
	QLabel* m_collision_label;
	QLabel* m_geovis_label;
	QLabel* m_trigger_label;
	QLabel* m_slider_label;
	QLabel* m_dest_label;
	QLabel* m_mover_label;
	QLabel* m_tilemap_label;

	QCheckBox* m_edit_collision;
	QCheckBox* m_edit_geovis;
	QCheckBox* m_edit_trigger;
	QCheckBox* m_edit_slider;
	QCheckBox* m_edit_dest;
	QCheckBox* m_edit_mover;
	QCheckBox* m_show_collision;
	QCheckBox* m_show_geovis;
	QCheckBox* m_show_trigger;
	QCheckBox* m_show_slider;
	QCheckBox* m_show_dest;
	QCheckBox* m_show_mover;
	QCheckBox* m_show_tilemap;
};