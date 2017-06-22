#pragma once

#include <QtGui>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qcombobox.h>

#include "Level.h"
#include "PolygonDef.h"

#include <gtx/rotate_vector.hpp>


class TexEditWidget : public QWidget
{
	Q_OBJECT

public:
	enum OperationMode
	{
		MODE_MOVE,
		MODE_ROTATE,
		MODE_SCALE,
	};

	TexEditWidget(QWidget* parent, Level* level);
	~TexEditWidget();
	void setTexture(QImage* texture);
	void selectObject(int object_id);
	void deselectObject();
	void setMode(OperationMode mode);
	void reset();
	void commit();
	void resetPan();

signals:

public slots:
	void setZoom(int zoom);

protected:
	void paintEvent(QPaintEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

private:
	glm::vec2 toScreenCoords(glm::vec2& point);
	glm::vec2 toUVCoords(glm::vec2& point);
	void objectToEditing(int object);
	float makeScalingScale(float scale);
	float makeRotationAngle(float angle);

	Level* m_level;
	QImage* m_texture;

	int m_selected_object;

	PolygonDef* m_polydef;

	float m_area_xleft;
	float m_area_xright;
	float m_area_ytop;
	float m_area_ybottom;
	float m_area_texside;

	OperationMode m_opmode;

	bool m_dragging;
	glm::vec2 m_drag_point;
	glm::vec2 m_scale_center;
	glm::vec2 m_rotate_center;

	glm::vec2 m_scroll;
	glm::vec2 m_pan_point;
	glm::vec2 m_scroll_saved;
	bool m_panning;

	float m_zoom;
};



class TextureEdit : public QDockWidget
{
	Q_OBJECT

public:
	TextureEdit(QWidget* parent, Level* level);
	~TextureEdit();
	void setTexture(QImage* texture);

protected:
	void closeEvent(QCloseEvent* event);
	void keyReleaseEvent(QKeyEvent* event);

signals:
	void onClose();

public slots:
	void select(int object_id);
	void deselect();
	void remove(int object_id);
	void moveMode();
	void rotateMode();
	void scaleMode();
	void reset();
	void commit();

private:
	QMainWindow* m_window;
	QToolBar* m_toolbar;
	QMenu* m_menu;
	TexEditWidget* m_widget;

	QActionGroup* m_toolgroup;
	QAction* m_move_action;
	QAction* m_rotate_action;
	QAction* m_scale_action;
	QAction* m_reset_action;
	QAction* m_commit_action;
	QComboBox* m_zoom_box;
};