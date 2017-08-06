#pragma once

#include <QtGui>
#include <QGLWidget>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qmenu.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qcolordialog.h>
#include <qglshaderprogram.h>
#include <qspinbox.h>

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>

#include "Level.h"
#include "PolygonDef.h"

class TileDesignerWidget : public QGLWidget
{
	Q_OBJECT

	friend class TileDesigner;

public:
	enum PolyObject
	{
		POLY_TOP = 0x1,
		POLY_SIDE = 0x2,
	};

	TileDesignerWidget(QWidget* parent, Level* level);
	~TileDesignerWidget();

	void setTexture(QImage* texture);

signals:
	void onInsertTile(int tile_id);
	void onReplaceTile(int tile_id);
	void onSelectPoly(int poly);

public slots:
	void setZoom(int zoom);
	void setGrid(int grid);
	void enableShowGrid(bool enable);
	void enableSnapGrid(bool enable);
	void resetObject(int objects);
	void insertTile(QString& name);
	void replaceTile(QString& name, int index);
	void setColor(QColor color);
	void setTileType(int type);
	void setScale(double scale);
	void setRotate(int angle);
	void setTopType(Tilemap::TopType type);
	void setTopHeight(double height);
	void setShadingType(Tilemap::ShadingType type);

protected:
	void paintEvent(QPaintEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void transformPoly(PolygonDef* def, PolygonDef* srcdef, glm::vec2& pos, float rot, float scale);

	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

private:
	struct Shader
	{
		int location;
		int scale;
		int position;
		int tex_coord;
		int color;
		int vp_matrix;
		int rot_matrix;
	};

	struct VBO
	{
		glm::vec3 pos;
		glm::vec2 uv;
		unsigned int color;
	};

	QString loadShader(QString filename);
	void loadTexture(QImage* texture);

	glm::vec2 toScreenCoords(glm::vec2& point);
	glm::vec2 toUVCoords(glm::vec2& point);
	glm::vec2 snapToGrid(glm::vec2& point);

	void renderPoly(QPainter& painter, int polynum);
	void drawGrid(QPainter& painter);

	static const int POINT_CLICKING_THRESHOLD = 6;

	QColor m_bgcolor;
	QColor m_validpoly_color;
	QColor m_errorpoly_color;
	QColor m_validline_color;
	QColor m_errorline_color;
	QColor m_point_color;
	QColor m_closedpoly_color;
	QColor m_closedline_color;

	QColor m_object_color;

	QImage* m_texture;

	float m_viewport_width;
	float m_viewport_height;
	float m_viewport_aspect;

	GLuint m_shader;
	GLuint m_base_tex;
	QGLShaderProgram* m_level_program;
	Shader m_level_shader;

	int m_current_tile_type;
	int m_selected_poly;

	PolygonDef* m_poly[2];

	PolygonDef* m_poly_top_default[10];
	PolygonDef* m_poly_side_default;

	Level* m_level;

	VBO m_vbo[4];

	unsigned int m_color;
	
	float m_zoom;

	bool m_panning;
	glm::vec2 m_scroll;
	glm::vec2 m_scroll_saved;
	glm::vec2 m_pan_point;

	bool m_move_dragging;
	glm::vec2 m_move_reference;

	glm::vec2 m_position[2];
	float m_angle[2];
	float m_scale[2];

	bool m_snap_grid;
	bool m_show_grid;
	int m_grid_size;

	Tilemap::ShadingType m_shading_type;
	Tilemap::TopType m_top_type;
	float m_top_height;
};



class TileDesigner : public QDockWidget
{
	Q_OBJECT

public:
	TileDesigner(QWidget* parent, Level* level);
	~TileDesigner();

	void setTexture(QImage* texture);

	static const int NUM_GRID_SIZES = 8;
	static const float GRID_SIZE[NUM_GRID_SIZES];

	static const int NUM_ZOOM_LEVELS = 4;
	static const float ZOOM_LEVELS[NUM_ZOOM_LEVELS];

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void onClose();
	void onInsertTile(int tile_id);
	void onReplaceTile(int tile_id);

public slots:
	void reset();
	void insertTile();
	void replaceTile();
	void toggleGrid();
	void snapGrid();
	void setGridSize(int size);
	void chooseColor();
	void tileSelected(int tile);
	void setScale(double scale);
	void setRotate(int angle);
	void polySelected(int poly);
	void setTopType(int type);
	void setShadingType(int type);
	void setTopHeight(double height);

private:
	QMainWindow* m_window;	
	QMenu* m_menu;
	TileDesignerWidget* m_widget;

	QToolBar* m_tile_toolbar;
	QToolBar* m_zoom_toolbar;
	QToolBar* m_grid_toolbar;
	QToolBar* m_control_toolbar;
	QToolBar* m_color_toolbar;

	QActionGroup* m_tilegroup;
	QWidget* m_tiletype_widget;
	QLabel* m_tiletype_label;
	QComboBox* m_tiletype_combo;
	
	QActionGroup* m_zoomgroup;
	QWidget* m_zoomlevel_widget;
	QLabel* m_zoomlevel_label;
	QComboBox* m_zoom_box;

	QWidget* m_rotate_widget;
	QLabel* m_rotate_label;
	QSpinBox* m_rotate_spin;

	QWidget* m_scale_widget;
	QLabel* m_scale_label;
	QDoubleSpinBox* m_scale_spin;

	QActionGroup* m_gridgroup;
	QAction* m_togglegrid_action;
	QAction* m_snapgrid_action;
	QLabel* m_gridsize_label;
	QComboBox* m_gridsize_combo;
	QWidget* m_gridsize_widget;

	QActionGroup* m_controlgroup;
	QPushButton* m_reset_button;
	QPushButton* m_inserttile_button;
	QPushButton* m_replacetile_button;
	
	QWidget* m_color_widget;
	QLabel* m_color_label;
	QPushButton* m_color_button;

	QLabel* m_toptype_label;
	QComboBox* m_toptype_combo;
	QWidget* m_toptype_widget;

	QLabel* m_topheight_label;
	QDoubleSpinBox* m_topheight_spin;
	QWidget* m_topheight_widget;

	QLabel* m_shading_label;
	QComboBox* m_shading_combo;
	QWidget* m_shading_widget;

	Level* m_level;

	QColor m_object_color;

	int m_selected_tile;
};