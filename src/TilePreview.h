#pragma once

#include <QtGui>
#include <QGLWidget>
#include <qopenglfunctions.h>
#include <qglshaderprogram.h>

#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Level.h"
#include "VBO.h"
#include "Tileset.h"
#include "Shaders.h"

class TilePreview : public QGLWidget, private QOpenGLFunctions
{
	Q_OBJECT

public:
	TilePreview(QWidget* parent, Level* level);
	~TilePreview();

	void setTexture(QImage* texture);
	void setTopUVs(PolygonDef* uvs);
	void setSideUVs(PolygonDef* uvs);
	void setSideTopUVs(PolygonDef* uvs);
	void setSideBotUVs(PolygonDef* uvs);
	void setTileType(Tileset::TileType type);
	void setTopHeight(float height);
	void setTopType(Tileset::TopType type);
	void setShadingType(Tileset::ShadingType type);

	QImage makeThumbnail(PolygonDef* top_points, PolygonDef* side_points,
		PolygonDef* sidetop_points, PolygonDef* sidebot_points,
		Tileset::TileType tile_type,
		Tileset::TopType top_type,
		Tileset::ShadingType shading_type,
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
		const glm::vec2* sidetop_points, const glm::vec2* sidebot_points,
		Tileset::TileType tile_type,
		Tileset::TopType top_type,
		Tileset::ShadingType shading_type,
		float top_height,
		unsigned int color);

	void loadAmbientMap(QImage* texture);

	Level* m_level;

	float m_viewport_width;
	float m_viewport_height;
	float m_viewport_aspect;

	QGLShaderProgram* m_standard_program;
	Shaders::HomestarShader m_standard_shader;

	VBO<HSVertex>* m_vbo;
	GLuint m_base_tex;
	GLuint m_ambient_tex;

	Tileset::TileType m_tile_type;
	Tileset::TopType m_top_type;
	Tileset::ShadingType m_shading_type;

	glm::vec2 m_top_points[6];
	glm::vec2 m_side_points[4];
	glm::vec2 m_sidetop_points[4];
	glm::vec2 m_sidebot_points[4];
	unsigned int m_color;
	float m_top_height;
};