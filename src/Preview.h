#pragma once

#include <QtGui>
#include <QGLWidget>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qglshaderprogram.h>

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Level.h"
#include "PolygonDef.h"
#include "VBO.h"

class GLPreview : public QGLWidget
{
	Q_OBJECT

public:
	GLPreview(QWidget* parent, Level* level);
	~GLPreview();

	void setTexture(QImage* texture);

protected:
	void paintEvent(QPaintEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void mouseMoveEvent(QMouseEvent* event);

	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

private:
	struct Shader
	{
		int position;
		int tex_coord;
		int color;
		int vp_matrix;
	};

	static const int LEVEL_VIS_WIDTH = 32;

	QString loadShader(QString filename);
	void loadTexture(QImage* texture);
	glm::vec2 toLevelCoords(glm::vec2& point);

	Level* m_level;

	QImage* m_texture;

	float m_viewport_width;
	float m_viewport_height;
	float m_viewport_aspect;

	GLuint m_shader;
	GLuint m_base_tex;
	QGLShaderProgram* m_level_program;
	Shader m_level_shader;

	VBO* m_vbback;


	bool m_panning;
	glm::vec2 m_scroll;
	glm::vec2 m_scroll_saved;
	glm::vec2 m_pan_point;
};



class PreviewWindow : public QDockWidget
{
	Q_OBJECT

public:
	PreviewWindow(QWidget* parent, Level* level);
	~PreviewWindow();

	void setTexture(QImage* texture);

protected:
	void closeEvent(QCloseEvent* event);

signals:
	void onClose();

public slots:
	void tileUpdated(int x, int y);

private:
	QMainWindow* m_window;

	GLPreview* m_widget;
	Level* m_level;
};