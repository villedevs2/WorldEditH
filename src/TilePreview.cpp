#include "TilePreview.h"


TilePreview::TilePreview(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	setMinimumWidth(200);
	setMaximumWidth(200);
	setMinimumHeight(300);
	setMinimumHeight(300);

	m_vbo = new VBO(20);

	m_tile_type = Tilemap::TILE_FULL;
	updateGeo();

	m_side_points[0] = glm::vec2(0.0f, 0.0f);
	m_side_points[1] = glm::vec2(1.0f, 0.0f);
	m_side_points[2] = glm::vec2(1.0f, 1.0f);
	m_side_points[3] = glm::vec2(0.0f, 1.0f);

	m_top_height =1.0f;
	m_color = 0xff808080;
}

TilePreview::~TilePreview()
{
	delete m_vbo;
}

void TilePreview::initializeGL()
{
	QString error;
	std::string errors;

	QString hs_vs_file = loadShader("homestar_vs.glsl");
	QString hs_fs_file = loadShader("homestar_fs.glsl");

	m_standard_program = new QGLShaderProgram(this);
	m_standard_program->addShaderFromSourceCode(QGLShader::Vertex, hs_vs_file);
	m_standard_program->addShaderFromSourceCode(QGLShader::Fragment, hs_fs_file);
	m_standard_program->link();

	error = m_standard_program->log();
	errors = error.toStdString();

	m_standard_shader.position = m_standard_program->attributeLocation("a_position");
	m_standard_shader.tex_coord = m_standard_program->attributeLocation("a_texcoord");
	m_standard_shader.color = m_standard_program->attributeLocation("a_color");
	m_standard_shader.normal = m_standard_program->attributeLocation("a_normal");
	m_standard_shader.vp_matrix = m_standard_program->uniformLocation("m_vp_matrix");
	m_standard_shader.v_matrix = m_standard_program->uniformLocation("m_v_matrix");
}

QString TilePreview::loadShader(QString filename)
{
	QFile file;
	QByteArray bytes;

	file.setFileName(filename);
	file.open(QIODevice::ReadOnly);
	bytes = file.readAll();
	QString shader(bytes);
	file.close();

	return bytes;
}

void TilePreview::paintGL()
{
	makeCurrent();

	QPainter painter;
	painter.begin(this);
	painter.beginNativePainting();

	qglClearColor(QColor(0,0,255));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);

	float aspect = (float)(width()) / (float)(height());
//	float halfw = (float)(2.0f);
//	float halfh = halfw / aspect;

	const double fov = 30.0;
	const double near_plane = 0.01;
	const double far_plane = 100.0;

	float size = near_plane * (float)tan((fov * M_PI / 180.0) / 2);

	glm::vec3 pos = glm::vec3(1.5f, 2.8f, 3.3f);
	glm::vec3 eye = glm::vec3(0.5f, 0.5f, 1.0f);

	glm::vec3 side(1.0f, 0.0f, 0.0f);
	glm::vec3 n = glm::normalize(glm::cross(pos - eye, side));

	glm::mat4 camera_proj_matrix = glm::frustum<float>(-size, size, size / aspect, -size / aspect, near_plane, far_plane);
	glm::mat4 camera_view_matrix = glm::lookAt(pos, eye, glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 camera_vp_matrix = camera_proj_matrix * camera_view_matrix;


	QMatrix4x4 vp_mat = QMatrix4x4(glm::value_ptr(camera_vp_matrix));
	QMatrix4x4 v_mat = QMatrix4x4(glm::value_ptr(camera_view_matrix));

	m_standard_program->bind();

	m_standard_program->setUniformValue(m_standard_shader.vp_matrix, vp_mat);
	m_standard_program->setUniformValue(m_standard_shader.v_matrix, v_mat);

	//m_standard_program->setUniformValue(m_standard_shader.cam_pos, QVector3D(pos.x, pos.y, pos.z));

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);


	glBindTexture(GL_TEXTURE_2D, m_base_tex);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, m_env_tex);

	float* geo = (float*)m_vbo->getPointer();
	int vbsize = m_vbo->getVertexSize();

	m_standard_program->enableAttributeArray(m_standard_shader.position);
	m_standard_program->setAttributeArray(m_standard_shader.position, (GLfloat*)geo, 3, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.tex_coord);
	m_standard_program->setAttributeArray(m_standard_shader.tex_coord, (GLfloat*)geo + 3, 2, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.normal);
	m_standard_program->setAttributeArray(m_standard_shader.normal, (GLfloat*)geo + 5, 3, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.color);
	m_standard_program->setAttributeArray(m_standard_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)geo + 32, 4, vbsize);

	glDrawArrays(GL_TRIANGLES, 0, 20 * 3);
	
	m_standard_program->disableAttributeArray(m_standard_shader.position);
	m_standard_program->disableAttributeArray(m_standard_shader.tex_coord);
	m_standard_program->disableAttributeArray(m_standard_shader.normal);
	m_standard_program->disableAttributeArray(m_standard_shader.color);


	painter.endNativePainting();
	painter.end();

	doneCurrent();
}

void TilePreview::resizeGL(int width, int height)
{
	makeCurrent();

	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);

	m_viewport_width = width;
	m_viewport_height = height;
	m_viewport_aspect = m_viewport_width / m_viewport_height;

	doneCurrent();

	update();
}


void TilePreview::setTexture(QImage* texture)
{
	makeCurrent();

	if (texture == nullptr)
		return;

	if (glIsTexture(m_base_tex))
		glDeleteTextures(1, &m_base_tex);
	glGenTextures(1, &m_base_tex);
	glBindTexture(GL_TEXTURE_2D, m_base_tex);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	int width = texture->width();
	int height = texture->height();

	char *pixels = new char[width * height * 4];

	int index = 0;
	for (int j = 0; j < height; j++)
	{
		QRgb *scan = (QRgb*)texture->scanLine(j);

		for (int i = 0; i < width; i++)
		{
			int r = qRed(scan[i]);
			int g = qGreen(scan[i]);
			int b = qBlue(scan[i]);
			int a = qAlpha(scan[i]);

			pixels[index + 0] = r;
			pixels[index + 1] = g;
			pixels[index + 2] = b;
			pixels[index + 3] = a;
			index += 4;
		}
	}

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels);

	delete[] pixels;

	doneCurrent();

	updateGeo();
}

void TilePreview::setTileType(Tilemap::TileType type)
{
	if (type != m_tile_type)
	{
		m_tile_type = type;
		updateGeo();
	}
}

void TilePreview::setTopType(Tilemap::TopType type)
{
	if (type != m_top_type)
	{
		m_top_type = type;
		updateGeo();
	}
}

void TilePreview::setShadingType(Tilemap::ShadingType type)
{
	if (type != m_shading_type)
	{
		m_shading_type = type;
		updateGeo();
	}
}

void TilePreview::updateGeo()
{
	int vbo_index = 0;

	glm::vec2 uv1 = m_top_points[0];
	glm::vec2 uv2 = m_top_points[1];
	glm::vec2 uv3 = m_top_points[2];
	glm::vec2 uv4 = m_top_points[3];
	glm::vec2 uv5 = m_top_points[4];
	glm::vec2 uv6 = m_top_points[5];

	glm::vec2 uvcen = glm::mix(uv1 + ((uv5 - uv1) * 0.5f), uv2 + ((uv4 - uv2) * 0.5f), 0.5f);

	glm::vec2 suv1 = m_side_points[0];
	glm::vec2 suv2 = m_side_points[1];
	glm::vec2 suv3 = m_side_points[2];
	glm::vec2 suv4 = m_side_points[3];

		/*
		     p6
		p1         p5
		p2         p4
		     p3
		*/

	float z = 1.0f;

	float midz = 0.0f;
	if (m_top_type == Tilemap::TOP_POINTY)
		midz = z + (m_top_height * 0.1f);
	else if (m_top_type == Tilemap::TOP_FLAT)
		midz = z;

	glm::vec3 p1 = glm::vec3(0.0f, 0.3f, z);
	glm::vec3 p2 = glm::vec3(0.0f, 0.7f, z);
	glm::vec3 p3 = glm::vec3(0.5f, 1.0f, z);
	glm::vec3 p4 = glm::vec3(1.0f, 0.7f, z);
	glm::vec3 p5 = glm::vec3(1.0f, 0.3f, z);
	glm::vec3 p6 = glm::vec3(0.5f, 0.0f, z);

	glm::vec3 pcen = glm::vec3(0.5f, 0.5f, midz);

	glm::vec3 bp1 = glm::vec3(p1.x, p1.y, 0.0f);
	glm::vec3 bp2 = glm::vec3(p2.x, p2.y, 0.0f);
	glm::vec3 bp3 = glm::vec3(p3.x, p3.y, 0.0f);
	glm::vec3 bp4 = glm::vec3(p4.x, p4.y, 0.0f);
	glm::vec3 bp5 = glm::vec3(p5.x, p5.y, 0.0f);
	glm::vec3 bp6 = glm::vec3(p6.x, p6.y, 0.0f);

	glm::vec3 top_norm(0.0f, 0.0f, 1.0f);

	VBO::Vertex tv1(p1, uv1, top_norm, m_color);
	VBO::Vertex tv2(p2, uv2, top_norm, m_color);
	VBO::Vertex tv3(p3, uv3, top_norm, m_color);
	VBO::Vertex tv4(p4, uv4, top_norm, m_color);
	VBO::Vertex tv5(p5, uv5, top_norm, m_color);
	VBO::Vertex tv6(p6, uv6, top_norm, m_color);
	VBO::Vertex tvcen(pcen, uvcen, top_norm, m_color);

	VBO::Vertex left_v1(p1, suv1, glm::vec3(), m_color);
	VBO::Vertex left_v2(p2, suv2, glm::vec3(), m_color);
	VBO::Vertex left_v3(bp2, suv3, glm::vec3(), m_color);
	VBO::Vertex left_v4(bp1, suv4, glm::vec3(), m_color);

	VBO::Vertex topleft_v1(p6, suv1, glm::vec3(), m_color);
	VBO::Vertex topleft_v2(p1, suv2, glm::vec3(), m_color);
	VBO::Vertex topleft_v3(bp1, suv3, glm::vec3(), m_color);
	VBO::Vertex topleft_v4(bp6, suv4, glm::vec3(), m_color);

	VBO::Vertex topright_v1(p5, suv1, glm::vec3(), m_color);
	VBO::Vertex topright_v2(p6, suv2, glm::vec3(), m_color);
	VBO::Vertex topright_v3(bp6, suv3, glm::vec3(), m_color);
	VBO::Vertex topright_v4(bp5, suv4, glm::vec3(), m_color);

	VBO::Vertex right_v1(p4, suv1, glm::vec3(), m_color);
	VBO::Vertex right_v2(p5, suv2, glm::vec3(), m_color);
	VBO::Vertex right_v3(bp5, suv3, glm::vec3(), m_color);
	VBO::Vertex right_v4(bp4, suv4, glm::vec3(), m_color);

	VBO::Vertex botright_v1(p3, suv1, glm::vec3(), m_color);
	VBO::Vertex botright_v2(p4, suv2, glm::vec3(), m_color);
	VBO::Vertex botright_v3(bp4, suv3, glm::vec3(), m_color);
	VBO::Vertex botright_v4(bp3, suv4, glm::vec3(), m_color);

	VBO::Vertex botleft_v1(p2, suv1, glm::vec3(), m_color);
	VBO::Vertex botleft_v2(p3, suv2, glm::vec3(), m_color);
	VBO::Vertex botleft_v3(bp3, suv3, glm::vec3(), m_color);
	VBO::Vertex botleft_v4(bp2, suv4, glm::vec3(), m_color);

	VBO::Vertex sideleft_v1(p3, suv1, glm::vec3(), m_color);
	VBO::Vertex sideleft_v2(p6, suv2, glm::vec3(), m_color);
	VBO::Vertex sideleft_v3(bp6, suv3, glm::vec3(), m_color);
	VBO::Vertex sideleft_v4(bp3, suv4, glm::vec3(), m_color);

	VBO::Vertex sideright_v1(p6, suv1, glm::vec3(), m_color);
	VBO::Vertex sideright_v2(p3, suv2, glm::vec3(), m_color);
	VBO::Vertex sideright_v3(bp3, suv3, glm::vec3(), m_color);
	VBO::Vertex sideright_v4(bp6, suv4, glm::vec3(), m_color);

	VBO::Vertex midtop_v1(p1, suv1, glm::vec3(), m_color);
	VBO::Vertex midtop_v2(p5, suv2, glm::vec3(), m_color);
	VBO::Vertex midtop_v3(bp5, suv3, glm::vec3(), m_color);
	VBO::Vertex midtop_v4(bp1, suv4, glm::vec3(), m_color);

	VBO::Vertex midbot_v1(p4, suv1, glm::vec3(), m_color);
	VBO::Vertex midbot_v2(p2, suv2, glm::vec3(), m_color);
	VBO::Vertex midbot_v3(bp2, suv3, glm::vec3(), m_color);
	VBO::Vertex midbot_v4(bp4, suv4, glm::vec3(), m_color);

	VBO::Vertex centtop_v1(p5, suv1, glm::vec3(), m_color);
	VBO::Vertex centtop_v2(p1, suv2, glm::vec3(), m_color);
	VBO::Vertex centtop_v3(bp1, suv3, glm::vec3(), m_color);
	VBO::Vertex centtop_v4(bp5, suv4, glm::vec3(), m_color);

	VBO::Vertex centbot_v1(p2, suv1, glm::vec3(), m_color);
	VBO::Vertex centbot_v2(p4, suv2, glm::vec3(), m_color);
	VBO::Vertex centbot_v3(bp4, suv3, glm::vec3(), m_color);
	VBO::Vertex centbot_v4(bp2, suv4, glm::vec3(), m_color);

	VBO::Vertex corntl_v1(p2, suv1, glm::vec3(), m_color);
	VBO::Vertex corntl_v2(p6, suv2, glm::vec3(), m_color);
	VBO::Vertex corntl_v3(bp6, suv3, glm::vec3(), m_color);
	VBO::Vertex corntl_v4(bp2, suv4, glm::vec3(), m_color);

	VBO::Vertex corntr_v1(p6, suv1, glm::vec3(), m_color);
	VBO::Vertex corntr_v2(p4, suv2, glm::vec3(), m_color);
	VBO::Vertex corntr_v3(bp4, suv3, glm::vec3(), m_color);
	VBO::Vertex corntr_v4(bp6, suv4, glm::vec3(), m_color);

	VBO::Vertex cornbl_v1(p3, suv1, glm::vec3(), m_color);
	VBO::Vertex cornbl_v2(p1, suv2, glm::vec3(), m_color);
	VBO::Vertex cornbl_v3(bp1, suv3, glm::vec3(), m_color);
	VBO::Vertex cornbl_v4(bp3, suv4, glm::vec3(), m_color);

	VBO::Vertex cornbr_v1(p5, suv1, glm::vec3(), m_color);
	VBO::Vertex cornbr_v2(p3, suv2, glm::vec3(), m_color);
	VBO::Vertex cornbr_v3(bp3, suv3, glm::vec3(), m_color);
	VBO::Vertex cornbr_v4(bp5, suv4, glm::vec3(), m_color);

	enum
	{
		RENDER_LEFT = 0x1,
		RENDER_TOPLEFT = 0x2,
		RENDER_TOPRIGHT = 0x4,
		RENDER_RIGHT = 0x8,
		RENDER_BOTRIGHT = 0x10,
		RENDER_BOTLEFT = 0x20,
		RENDER_SIDELEFT = 0x40,
		RENDER_SIDERIGHT = 0x80,
		RENDER_MIDTOP = 0x100,
		RENDER_MIDBOT = 0x200,
		RENDER_CORNER_TL = 0x400,
		RENDER_CORNER_TR = 0x800,
		RENDER_CORNER_BL = 0x1000,
		RENDER_CORNER_BR = 0x2000,
		RENDER_CENTER_TOP = 0x4000,
		RENDER_CENTER_BOT = 0x8000,
	};

	int render_sides = 0;

	switch (m_tile_type)
	{
		case Tilemap::TILE_FULL:
		{
			/*
			     /\
			    /  \
			   |    |
			   |    |
		     	\  /
			     \/
			*/
			
			m_vbo->makeTriPolyNorm(vbo_index++, tv1, tv6, tvcen);
			m_vbo->makeTriPolyNorm(vbo_index++, tv6, tv5, tvcen);
			m_vbo->makeTriPolyNorm(vbo_index++, tv5, tv4, tvcen);
			m_vbo->makeTriPolyNorm(vbo_index++, tv4, tv3, tvcen);
			m_vbo->makeTriPolyNorm(vbo_index++, tv3, tv2, tvcen);
			m_vbo->makeTriPolyNorm(vbo_index++, tv2, tv1, tvcen);
			
			render_sides |= RENDER_LEFT;
			render_sides |= RENDER_TOPLEFT;
			render_sides |= RENDER_TOPRIGHT;
			render_sides |= RENDER_RIGHT;
			render_sides |= RENDER_BOTRIGHT;
			render_sides |= RENDER_BOTLEFT;
			break;
		}
		case Tilemap::TILE_LEFT:
		{
			/*
			      /|
			     / |
			    |  |
			    |  |
			     \ |
			      \|
			*/

			m_vbo->makeTri(vbo_index++, tv1, tv6, tv3);
			m_vbo->makeTri(vbo_index++, tv1, tv3, tv2);

			render_sides |= RENDER_LEFT;
			render_sides |= RENDER_TOPLEFT;
			render_sides |= RENDER_BOTLEFT;
			render_sides |= RENDER_SIDELEFT;
			break;
		}
		case Tilemap::TILE_RIGHT:
		{
			/*
			    |\
			    | \
			    |  |
			    |  |
			    | /
			    |/
			*/

			m_vbo->makeTri(vbo_index++, tv6, tv5, tv4);
			m_vbo->makeTri(vbo_index++, tv6, tv4, tv3);

			render_sides |= RENDER_TOPRIGHT;
			render_sides |= RENDER_RIGHT;
			render_sides |= RENDER_BOTRIGHT;
			render_sides |= RENDER_SIDERIGHT;
			break;
		}
		case Tilemap::TILE_TOP:
		{
			/*
			     /\
			    /__\
			*/

			m_vbo->makeTri(vbo_index++, tv1, tv6, tv5);

			render_sides |= RENDER_TOPLEFT;
			render_sides |= RENDER_TOPRIGHT;
			render_sides |= RENDER_MIDTOP;
			break;
		}
		case Tilemap::TILE_BOTTOM:
		{
			/*  ____
			    \  /
			     \/
			*/

			m_vbo->makeTri(vbo_index++, tv2, tv4, tv3);

			render_sides |= RENDER_BOTLEFT;
			render_sides |= RENDER_BOTRIGHT;
			render_sides |= RENDER_MIDBOT;
			break;
		}
		case Tilemap::TILE_MID:
		{
			/*  ______
			    |    |
			    |____|
			*/

			m_vbo->makeTri(vbo_index++, tv1, tv5, tv4);
			m_vbo->makeTri(vbo_index++, tv1, tv4, tv2);

			render_sides |= RENDER_LEFT;
			render_sides |= RENDER_RIGHT;
			render_sides |= RENDER_CENTER_TOP;
			render_sides |= RENDER_CENTER_BOT;
			break;
		}
		case Tilemap::TILE_CORNER_TL:
		{
			/*
			     /.
			    / .
			   | .
			   |.
			*/

			m_vbo->makeTri(vbo_index++, tv1, tv6, tv2);

			render_sides |= RENDER_LEFT;
			render_sides |= RENDER_TOPLEFT;
			render_sides |= RENDER_CORNER_TL;
			break;
		}
		case Tilemap::TILE_CORNER_TR:
		{
			/*
			    .\
			    . \
			     . |
			      .|
			*/

			m_vbo->makeTri(vbo_index++, tv6, tv5, tv4);

			render_sides |= RENDER_TOPRIGHT;
			render_sides |= RENDER_RIGHT;
			render_sides |= RENDER_CORNER_TR;
			break;
		}
		case Tilemap::TILE_CORNER_BL:
		{
			/*
			    |.
			    | .
			    \ .
			     \.
			*/

			m_vbo->makeTri(vbo_index++, tv1, tv3, tv2);

			render_sides |= RENDER_LEFT;
			render_sides |= RENDER_BOTLEFT;
			render_sides |= RENDER_CORNER_BL;
			break;
		}
		case Tilemap::TILE_CORNER_BR:
		{
			/*
			     .|
			    . |
			   . /
			   ./
			*/

			m_vbo->makeTri(vbo_index++, tv3, tv5, tv4);

			render_sides |= RENDER_RIGHT;
			render_sides |= RENDER_BOTRIGHT;
			render_sides |= RENDER_CORNER_BR;
			break;
		}

		default:
		{
			// make degen geo
			m_vbo->degenTris(vbo_index, 20);
			break;
		}
	}

	if (render_sides & RENDER_LEFT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, left_v1, left_v2, left_v3, left_v4);
	if (render_sides & RENDER_TOPLEFT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topleft_v1, topleft_v2, topleft_v3, topleft_v4);
	if (render_sides & RENDER_TOPRIGHT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topright_v1, topright_v2, topright_v3, topright_v4);
	if (render_sides & RENDER_RIGHT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, right_v1, right_v2, right_v3, right_v4);
	if (render_sides & RENDER_BOTRIGHT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botright_v1, botright_v2, botright_v3, botright_v4);
	if (render_sides & RENDER_BOTLEFT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botleft_v1, botleft_v2, botleft_v3, botleft_v4);
	if (render_sides & RENDER_SIDELEFT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideleft_v1, sideleft_v2, sideleft_v3, sideleft_v4);
	if (render_sides & RENDER_SIDERIGHT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideright_v1, sideright_v2, sideright_v3, sideright_v4);
	if (render_sides & RENDER_MIDTOP)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midtop_v1, midtop_v2, midtop_v3, midtop_v4);
	if (render_sides & RENDER_MIDBOT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midbot_v1, midbot_v2, midbot_v3, midbot_v4);
	if (render_sides & RENDER_CENTER_TOP)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centtop_v1, centtop_v2, centtop_v3, centtop_v4);
	if (render_sides & RENDER_CENTER_BOT)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centbot_v1, centbot_v2, centbot_v3, centbot_v4);
	if (render_sides & RENDER_CORNER_TL)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntl_v1, corntl_v2, corntl_v3, corntl_v4);
	if (render_sides & RENDER_CORNER_TR)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntr_v1, corntr_v2, corntr_v3, corntr_v4);
	if (render_sides & RENDER_CORNER_BL)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbl_v1, cornbl_v2, cornbl_v3, cornbl_v4);
	if (render_sides & RENDER_CORNER_BR)
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbr_v1, cornbr_v2, cornbr_v3, cornbr_v4);

	if (vbo_index < 20)
		m_vbo->degenTris(vbo_index, 20 - vbo_index);
	
	update();
}

void TilePreview::setTopHeight(float height)
{
	m_top_height = height;

	updateGeo();
}

void TilePreview::setTopUVs(PolygonDef* uvs)
{
	assert(uvs->getNumPoints() <= 6);

	for (int i = 0; i < uvs->getNumPoints(); i++)
	{
		m_top_points[i] = uvs->getPoint(i);
	}

	updateGeo();
}

void TilePreview::setSideUVs(PolygonDef* uvs)
{
	assert(uvs->getNumPoints() == 4);

	for (int i = 0; i < 4; i++)
	{
		m_side_points[i] = uvs->getPoint(i);
	}

	updateGeo();
}