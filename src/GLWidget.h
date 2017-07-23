#pragma once

#include <QtGui>
#include <QGLWidget>
#include <qglshaderprogram.h>

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>

#include "PolygonDef.h"
#include "Level.h"

class GLWidget : public QGLWidget
{
	Q_OBJECT

public:
	enum OperationMode
	{
		MODE_SELECT,
		MODE_MOVE,
		MODE_ROTATE,
		MODE_SCALE,
		MODE_DRAW_POLY,
		MODE_DRAW_RECT,
		MODE_TILEMAP,
		MODE_TILE_ZEDIT,
	};

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

	static const int NUM_GRID_SIZES = 6;
	static const float GRID_SIZE[NUM_GRID_SIZES];

	static const int NUM_ZOOM_LEVELS = 6;
	static const float ZOOM_LEVELS[NUM_ZOOM_LEVELS];

	GLWidget(QWidget* parent, Level* level);
	~GLWidget(void);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	glm::vec2 toScreenCoords(glm::vec2 point);
	glm::vec2 toLevelCoords(glm::vec2 point);
	void loadTexture(QImage* texture);
	void enableGrid(bool enable);
	void setSnapGrid(bool enable);
	void reset();
	void enableVisbox(bool enable);
	void configVisbox(float width, float height);

	void setEdgeData(std::vector<std::vector<int>> data);

signals:
	void onAddObject(int id);
	void onRemoveObject(int id);
	void onSelectObject(int id);
	void onDeselectObject();
	void onSetMode(GLWidget::OperationMode mode);
	void onTileUpdate(int x, int y);
	void onTileSelect(int x, int y);

public slots:
	void animate();
	void setMode(OperationMode mode);
	void select(int object_id);
	void deselect();
	void copy();
	void paste();
	void setGridSize(int size);
	void setCreateType(Level::ObjectType type);
	void setCreateTriggerType(int type);
	void setZoomLevel(int zoom);
	void setBGColor(QColor color);
	void setTileBrush(int tile);
	void setObjectColor(QColor color);
	void setTileBaseZ(float z);

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);
	void mouseReleaseEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void keyPressEvent(QKeyEvent* event);
	void paintEvent(QPaintEvent *e);

private:
	static const int LEVEL_VIS_WIDTH = 64;
	static const int POINT_CLICKING_THRESHOLD = 6;

	QString loadShader(QString filename);
	void renderDrawPolyMode(QPainter& painter);
	void renderDrawRectMode(QPainter& painter);
	void renderEditMode(QPainter& painter);
	void renderMoveMode(QPainter& painter);
	void renderOtherObjects(QPainter& painter);
	void renderTilemapPointer(QPainter& painter);
	void renderEdgeData(QPainter& painter);
	void selectObject(int object);
	void deselectObject();
	void objectToEditing(int object);
	void drawGrid(QPainter& painter);
	void drawGLGrid();
	glm::vec2 snapToGrid(glm::vec2 point);
	float makeScalingScale(float scale);
	float makeRotationAngle(float angle);
	void updateTileDrawLocation(const glm::vec2& mouse_lp);
	void tilemapDraw();
	void tilemapZDraw();
	void tilemapZEdit(int zmod);

	Level* m_level;
	QColor m_bgcolor;
	QColor m_validpoly_color;
	QColor m_errorpoly_color;
	QColor m_validline_color;
	QColor m_errorline_color;
	QColor m_point_color;

	GLuint m_shader;

	GLuint m_base_tex;

	float m_viewport_width;
	float m_viewport_height;
	float m_viewport_aspect;

	std::string m_error_string;

	QGLShaderProgram* m_level_program;
	QGLShaderProgram* m_grid_program;
	Shader m_level_shader;
	Shader m_grid_shader;

	OperationMode m_opmode;

	PolygonDef* m_polydef;

	Level::ObjectType m_create_type;
	int m_create_trigger_type;
	unsigned int m_create_poly_color;

	// select/edit stuff
	int m_selected_object;
	int m_selected_point;
	glm::vec2 m_saved_point;

	// dragging stuff
	bool m_dragging;
	glm::vec2 m_drag_point;
	glm::vec2 m_object_center;

	// grid stuff
	bool m_enable_grid;
	bool m_snap_grid;
	float *m_grid_vb;
	int m_grid_size;

	// pan/zoom stuff
	glm::vec2 m_scroll;
	glm::vec2 m_pan_point;
	glm::vec2 m_scroll_saved;
	bool m_panning;

	// copy/paste stuff
	Level::Object* m_saved_object;
	glm::vec2 m_copy_delta;

	// visbox stuff
	bool m_enable_visbox;
	float m_visbox_width;
	float m_visbox_height;

	// zoom stuff
	int m_zoom_level;

	bool m_tilemap_painting;
	int m_tile_brush;		// currently selected "brush"

	int m_tile_selx;
	int m_tile_sely;

	float m_tile_basez;

	std::vector<std::vector<int>> m_edgedata;
};
