#pragma once

#include <QtGui>
#include <QGLWidget>
#include <qglshaderprogram.h>

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Level.h"
#include "VBO.h"

class TilePreview : public QGLWidget
{
	Q_OBJECT

public:
	TilePreview(QWidget* parent, Level* level);
	~TilePreview();

	void setTexture(QImage* texture);
	void setTopUVs(PolygonDef* uvs);
	void setSideUVs(PolygonDef* uvs);
	void setTileType(Tilemap::TileType type);
	void setTopHeight(float height);
	void setTopType(Tilemap::TopType type);
	void setShadingType(Tilemap::ShadingType type);

	QImage makeThumbnail(PolygonDef* top_points, PolygonDef* side_points,
						Tilemap::TileType tile_type,
						Tilemap::TopType top_type,
						Tilemap::ShadingType shading_type,
						float top_height,
						unsigned int color);

protected:
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
		int v_matrix;
		int normal;
		int cam_pos;
	};

	QString loadShader(QString filename);
	void updateGeo(const glm::vec2* top_points, const glm::vec2* side_points,
					Tilemap::TileType tile_type,
					Tilemap::TopType top_type,
					Tilemap::ShadingType shading_type,
					float top_height,
					unsigned int color);

	float m_viewport_width;
	float m_viewport_height;
	float m_viewport_aspect;

	QGLShaderProgram* m_standard_program;
	Shader m_standard_shader;

	VBO* m_vbo;
	GLuint m_base_tex;

	Tilemap::TileType m_tile_type;
	Tilemap::TopType m_top_type;
	Tilemap::ShadingType m_shading_type;

	glm::vec2 m_top_points[6];
	glm::vec2 m_side_points[4];
	unsigned int m_color;
	float m_top_height;
};