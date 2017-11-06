#include "TilePreview.h"


TilePreview::TilePreview(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_level = level;

	setMinimumWidth(200);
	setMaximumWidth(200);
	setMinimumHeight(300);
	setMinimumHeight(300);

	m_vbo = new VBO<HSVertex>(48);

	m_top_height = 1.0f;
	m_color = 0xff808080;

	m_tile_type = Tileset::TILE_FULL;
	m_top_type = Tileset::TOP_FLAT;
	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}

TilePreview::~TilePreview()
{
	delete m_vbo;
}

void TilePreview::initializeGL()
{
	initializeOpenGLFunctions();

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
	m_standard_shader.amb_coord = m_standard_program->attributeLocation("a_ambcoord");
	m_standard_shader.color = m_standard_program->attributeLocation("a_color");
	m_standard_shader.normal = m_standard_program->attributeLocation("a_normal");
	m_standard_shader.vp_matrix = m_standard_program->uniformLocation("m_vp_matrix");
	m_standard_shader.v_matrix = m_standard_program->uniformLocation("m_v_matrix");
	m_standard_shader.light = m_standard_program->uniformLocation("u_light");
	m_standard_shader.diff_sampler = m_standard_program->uniformLocation("s_color_texture");
	m_standard_shader.amb_sampler = m_standard_program->uniformLocation("s_ambient_texture");

	loadAmbientMap(m_level->getAO()->getMap());
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

	qglClearColor(QColor(0,0,0,0));
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


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_base_tex);
	//		glBindTexture(GL_TEXTURE_CUBE_MAP, m_env_tex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_ambient_tex);

	glUniform1i(m_standard_shader.diff_sampler, 0);
	glUniform1i(m_standard_shader.amb_sampler, 1);

	glUniform3f(m_standard_shader.light, 0.1, -0.7, 0.1);

	float* geo = (float*)m_vbo->getPointer();
	int vbsize = sizeof(HSVertex);

	m_standard_program->enableAttributeArray(m_standard_shader.position);
	m_standard_program->setAttributeArray(m_standard_shader.position, (GLfloat*)geo, 3, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.tex_coord);
	m_standard_program->setAttributeArray(m_standard_shader.tex_coord, (GLfloat*)geo + 3, 2, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.amb_coord);
	m_standard_program->setAttributeArray(m_standard_shader.amb_coord, (GLfloat*)geo + 5, 2, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.normal);
	m_standard_program->setAttributeArray(m_standard_shader.normal, (GLfloat*)geo + 7, 3, vbsize);
	m_standard_program->enableAttributeArray(m_standard_shader.color);
	m_standard_program->setAttributeArray(m_standard_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)geo + 40, 4, vbsize);

	glDrawArrays(GL_TRIANGLES, 0, m_vbo->getCapacity() * 3);

	m_standard_program->disableAttributeArray(m_standard_shader.position);
	m_standard_program->disableAttributeArray(m_standard_shader.tex_coord);
	m_standard_program->disableAttributeArray(m_standard_shader.amb_coord);
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

	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}


void TilePreview::loadAmbientMap(QImage* texture)
{
	makeCurrent();

	if (glIsTexture(m_ambient_tex))
		glDeleteTextures(1, &m_ambient_tex);
	glGenTextures(1, &m_ambient_tex);
	glBindTexture(GL_TEXTURE_2D, m_ambient_tex);

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
}


void TilePreview::setTileType(Tileset::TileType type)
{
	if (type != m_tile_type)
	{
		m_tile_type = type;
		updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
	}
}

void TilePreview::setTopType(Tileset::TopType type)
{
	if (type != m_top_type)
	{
		m_top_type = type;
		updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
	}
}

void TilePreview::setShadingType(Tileset::ShadingType type)
{
	if (type != m_shading_type)
	{
		m_shading_type = type;
		updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
	}
}

void TilePreview::updateGeo(const glm::vec2* top_points, const glm::vec2* side_points,
	const glm::vec2* sidetop_points, const glm::vec2* sidebot_points,
	Tileset::TileType tile_type,
	Tileset::TopType top_type,
	Tileset::ShadingType shading_type,
	float top_height,
	unsigned int color)
{
	int vbo_index = 0;

	glm::vec2 uv1 = top_points[0];
	glm::vec2 uv2 = top_points[1];
	glm::vec2 uv3 = top_points[2];
	glm::vec2 uv4 = top_points[3];
	glm::vec2 uv5 = top_points[4];
	glm::vec2 uv6 = top_points[5];

	glm::vec2 uvcen = glm::mix(uv1 + ((uv5 - uv1) * 0.5f), uv2 + ((uv4 - uv2) * 0.5f), 0.5f);

	glm::vec2 suv1 = side_points[0];
	glm::vec2 suv2 = side_points[1];
	glm::vec2 suv3 = side_points[2];
	glm::vec2 suv4 = side_points[3];

	Tilemap::TileDef tiledef;
	tiledef.floor_uvs[0] = top_points[0];
	tiledef.floor_uvs[1] = top_points[1];
	tiledef.floor_uvs[2] = top_points[2];
	tiledef.floor_uvs[3] = top_points[3];
	tiledef.floor_uvs[4] = top_points[4];
	tiledef.floor_uvs[5] = top_points[5];
	tiledef.floor_uvcen = glm::mix(uv1 + ((uv5 - uv1) * 0.5f), uv2 + ((uv4 - uv2) * 0.5f), 0.5f);

	tiledef.wallmid_uvs[0] = side_points[0];
	tiledef.wallmid_uvs[1] = side_points[1];
	tiledef.wallmid_uvs[2] = side_points[2];
	tiledef.wallmid_uvs[3] = side_points[3];

	tiledef.walltop_uvs[0] = sidetop_points[0];
	tiledef.walltop_uvs[1] = sidetop_points[1];
	tiledef.walltop_uvs[2] = sidetop_points[2];
	tiledef.walltop_uvs[3] = sidetop_points[3];

	tiledef.wallbot_uvs[0] = sidebot_points[0];
	tiledef.wallbot_uvs[1] = sidebot_points[1];
	tiledef.wallbot_uvs[2] = sidebot_points[2];
	tiledef.wallbot_uvs[3] = sidebot_points[3];

	tiledef.tiletype = tile_type;
	tiledef.toptype = top_type;
	tiledef.shading = shading_type;

	tiledef.tile_z = 1.0f;
	tiledef.top_height = top_height;
	tiledef.color = color;
	tiledef.tile_width = 1.0f;
	tiledef.tile_height = 1.4f;

	tiledef.tile_ao.floor = m_level->getAO()->getFloorTile(0);
	tiledef.tile_ao.wall_left = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_right = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_topleft = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_topright = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_botleft = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_botright = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_left = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_sideleft = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_sideright = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_midtop = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_midbot = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_centtop = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_centbot = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_corntl = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_corntr = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_cornbl = m_level->getAO()->getWallTile(0);
	tiledef.tile_ao.wall_cornbr = m_level->getAO()->getWallTile(0);

	Tilemap::makeVBOTile(m_vbo, 0, tiledef, 0, 0);


#if 0

		/*
		     p6
		p1         p5
		p2         p4
		     p3
		*/

	float botz = 0.1f;
	float topz = 0.1f;
	float z = 1.0f + botz + topz;

	float midz = 0.0f;
	if (top_type == Tileset::TOP_POINTY)
		midz = z + (top_height * 0.1f);
	else if (top_type == Tileset::TOP_FLAT)
		midz = z;

	glm::vec3 topt_p1 = glm::vec3(0.0f, 0.3f, z);
	glm::vec3 topt_p2 = glm::vec3(0.0f, 0.7f, z);
	glm::vec3 topt_p3 = glm::vec3(0.5f, 1.0f, z);
	glm::vec3 topt_p4 = glm::vec3(1.0f, 0.7f, z);
	glm::vec3 topt_p5 = glm::vec3(1.0f, 0.3f, z);
	glm::vec3 topt_p6 = glm::vec3(0.5f, 0.0f, z);
	glm::vec3 pcen = glm::vec3(0.5f, 0.5f, midz);

	glm::vec3 topb_p1 = glm::vec3(topt_p1.x, topt_p1.y, z - topz);
	glm::vec3 topb_p2 = glm::vec3(topt_p2.x, topt_p2.y, z - topz);
	glm::vec3 topb_p3 = glm::vec3(topt_p3.x, topt_p3.y, z - topz);
	glm::vec3 topb_p4 = glm::vec3(topt_p4.x, topt_p4.y, z - topz);
	glm::vec3 topb_p5 = glm::vec3(topt_p5.x, topt_p5.y, z - topz);
	glm::vec3 topb_p6 = glm::vec3(topt_p6.x, topt_p6.y, z - topz);

	glm::vec3 botb_p1 = glm::vec3(topt_p1.x, topt_p1.y, 0.0f);
	glm::vec3 botb_p2 = glm::vec3(topt_p2.x, topt_p2.y, 0.0f);
	glm::vec3 botb_p3 = glm::vec3(topt_p3.x, topt_p3.y, 0.0f);
	glm::vec3 botb_p4 = glm::vec3(topt_p4.x, topt_p4.y, 0.0f);
	glm::vec3 botb_p5 = glm::vec3(topt_p5.x, topt_p5.y, 0.0f);
	glm::vec3 botb_p6 = glm::vec3(topt_p6.x, topt_p6.y, 0.0f);

	glm::vec3 bott_p1 = glm::vec3(topt_p1.x, topt_p1.y, 0.0f + botz);
	glm::vec3 bott_p2 = glm::vec3(topt_p2.x, topt_p2.y, 0.0f + botz);
	glm::vec3 bott_p3 = glm::vec3(topt_p3.x, topt_p3.y, 0.0f + botz);
	glm::vec3 bott_p4 = glm::vec3(topt_p4.x, topt_p4.y, 0.0f + botz);
	glm::vec3 bott_p5 = glm::vec3(topt_p5.x, topt_p5.y, 0.0f + botz);
	glm::vec3 bott_p6 = glm::vec3(topt_p6.x, topt_p6.y, 0.0f + botz);

	const AmbientOcclusion::AOFloorTile& floorao = m_level->getAO()->getFloorTile(0);

	/*
	glm::vec2 amb_uv1 = glm::vec2(amb_tile_x, amb_tile_y + (0.3f * amb_tile_h));
	glm::vec2 amb_uv2 = glm::vec2(amb_tile_x, amb_tile_y + (0.7f * amb_tile_h));
	glm::vec2 amb_uv3 = glm::vec2(amb_tile_x + (0.5f * amb_tile_w), amb_tile_y + amb_tile_h);
	glm::vec2 amb_uv4 = glm::vec2(amb_tile_x + amb_tile_w, amb_tile_y + (0.7f * amb_tile_h));
	glm::vec2 amb_uv5 = glm::vec2(amb_tile_x + amb_tile_w, amb_tile_y + (0.3f * amb_tile_h));
	glm::vec2 amb_uv6 = glm::vec2(amb_tile_x + (0.5f * amb_tile_w), amb_tile_y);

	glm::vec2 amb_uvcen = glm::mix(amb_uv1 + ((amb_uv5 - amb_uv1) * 0.5f), amb_uv2 + ((amb_uv4 - amb_uv2) * 0.5f), 0.5f);
	*/

	const AmbientOcclusion::AOWallTile& wallao = m_level->getAO()->getWallTile(0);

	glm::vec2 amb_topt_uvl = wallao.uv[0];
	glm::vec2 amb_topt_uvr = wallao.uv[1];
	glm::vec2 amb_topb_uvl = glm::mix(wallao.uv[0], wallao.uv[3], 0.1f);
	glm::vec2 amb_topb_uvr = glm::mix(wallao.uv[1], wallao.uv[2], 0.1f);
	glm::vec2 amb_bott_uvl = glm::mix(wallao.uv[3], wallao.uv[0], 0.1f);
	glm::vec2 amb_bott_uvr = glm::mix(wallao.uv[2], wallao.uv[1], 0.1f);
	glm::vec2 amb_botb_uvl = wallao.uv[3];
	glm::vec2 amb_botb_uvr = wallao.uv[2];

	// TODOOOO
	/*
	glm::vec2 amb_suv1 = glm::vec2(0.0f, 0.0f);
	glm::vec2 amb_suv2 = glm::vec2(1.0f, 0.0f);
	glm::vec2 amb_suv3 = glm::vec2(1.0f, 1.0f);
	glm::vec2 amb_suv4 = glm::vec2(0.0f, 1.0f);
	*/

	glm::vec3 top_norm(0.0f, 0.0f, 1.0f);

	HSVertex tv1(topt_p1, uv1, floorao.uv[0], top_norm, color);
	HSVertex tv2(topt_p2, uv2, floorao.uv[1], top_norm, color);
	HSVertex tv3(topt_p3, uv3, floorao.uv[2], top_norm, color);
	HSVertex tv4(topt_p4, uv4, floorao.uv[3], top_norm, color);
	HSVertex tv5(topt_p5, uv5, floorao.uv[4], top_norm, color);
	HSVertex tv6(topt_p6, uv6, floorao.uv[5], top_norm, color);
	HSVertex tvcen(pcen, uvcen, floorao.center, top_norm, color);

	HSVertex left_m_v1(topb_p1, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex left_m_v2(topb_p2, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex left_m_v3(bott_p2, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex left_m_v4(bott_p1, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex left_t_v1(topt_p1, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex left_t_v2(topt_p2, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex left_t_v3(topb_p2, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex left_t_v4(topb_p1, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex left_b_v1(bott_p1, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex left_b_v2(bott_p2, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex left_b_v3(botb_p2, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex left_b_v4(botb_p1, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex topleft_m_v1(topb_p6, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex topleft_m_v2(topb_p1, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex topleft_m_v3(bott_p1, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex topleft_m_v4(bott_p6, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex topleft_t_v1(topt_p6, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex topleft_t_v2(topt_p1, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex topleft_t_v3(topb_p1, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex topleft_t_v4(topb_p6, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex topleft_b_v1(bott_p6, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex topleft_b_v2(bott_p1, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex topleft_b_v3(botb_p1, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex topleft_b_v4(botb_p6, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex topright_m_v1(topb_p5, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex topright_m_v2(topb_p6, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex topright_m_v3(bott_p6, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex topright_m_v4(bott_p5, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex topright_t_v1(topt_p5, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex topright_t_v2(topt_p6, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex topright_t_v3(topb_p6, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex topright_t_v4(topb_p5, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex topright_b_v1(bott_p5, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex topright_b_v2(bott_p6, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex topright_b_v3(botb_p6, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex topright_b_v4(botb_p5, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex right_m_v1(topb_p4, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex right_m_v2(topb_p5, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex right_m_v3(bott_p5, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex right_m_v4(bott_p4, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex right_t_v1(topt_p4, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex right_t_v2(topt_p5, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex right_t_v3(topb_p5, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex right_t_v4(topb_p4, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex right_b_v1(bott_p4, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex right_b_v2(bott_p5, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex right_b_v3(botb_p5, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex right_b_v4(botb_p4, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex botright_m_v1(topb_p3, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex botright_m_v2(topb_p4, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex botright_m_v3(bott_p4, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex botright_m_v4(bott_p3, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex botright_t_v1(topt_p3, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex botright_t_v2(topt_p4, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex botright_t_v3(topb_p4, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex botright_t_v4(topb_p3, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex botright_b_v1(bott_p3, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex botright_b_v2(bott_p4, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex botright_b_v3(botb_p4, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex botright_b_v4(botb_p3, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex botleft_m_v1(topb_p2, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex botleft_m_v2(topb_p3, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex botleft_m_v3(bott_p3, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex botleft_m_v4(bott_p2, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex botleft_t_v1(topt_p2, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex botleft_t_v2(topt_p3, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex botleft_t_v3(topb_p3, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex botleft_t_v4(topb_p2, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex botleft_b_v1(bott_p2, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex botleft_b_v2(bott_p3, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex botleft_b_v3(botb_p3, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex botleft_b_v4(botb_p2, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex sideleft_m_v1(topb_p3, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex sideleft_m_v2(topb_p6, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex sideleft_m_v3(bott_p6, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex sideleft_m_v4(bott_p3, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex sideleft_t_v1(topt_p3, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex sideleft_t_v2(topt_p6, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex sideleft_t_v3(topb_p6, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex sideleft_t_v4(topb_p3, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex sideleft_b_v1(bott_p3, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex sideleft_b_v2(bott_p6, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex sideleft_b_v3(botb_p6, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex sideleft_b_v4(botb_p3, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex sideright_m_v1(topb_p6, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex sideright_m_v2(topb_p3, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex sideright_m_v3(bott_p3, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex sideright_m_v4(bott_p6, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex sideright_t_v1(topt_p6, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex sideright_t_v2(topt_p3, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex sideright_t_v3(topb_p3, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex sideright_t_v4(topb_p6, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex sideright_b_v1(bott_p6, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex sideright_b_v2(bott_p3, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex sideright_b_v3(botb_p3, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex sideright_b_v4(botb_p6, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex midtop_m_v1(topb_p1, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex midtop_m_v2(topb_p5, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex midtop_m_v3(bott_p5, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex midtop_m_v4(bott_p1, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex midtop_t_v1(topt_p1, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex midtop_t_v2(topt_p5, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex midtop_t_v3(topb_p5, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex midtop_t_v4(topb_p1, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex midtop_b_v1(bott_p1, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex midtop_b_v2(bott_p5, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex midtop_b_v3(botb_p5, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex midtop_b_v4(botb_p1, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex midbot_m_v1(topb_p4, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex midbot_m_v2(topb_p2, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex midbot_m_v3(bott_p2, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex midbot_m_v4(bott_p4, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex midbot_t_v1(topt_p4, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex midbot_t_v2(topt_p2, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex midbot_t_v3(topb_p2, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex midbot_t_v4(topb_p4, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex midbot_b_v1(bott_p4, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex midbot_b_v2(bott_p2, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex midbot_b_v3(botb_p2, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex midbot_b_v4(botb_p4, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex centtop_m_v1(topb_p5, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex centtop_m_v2(topb_p1, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex centtop_m_v3(bott_p1, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex centtop_m_v4(bott_p5, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex centtop_t_v1(topt_p5, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex centtop_t_v2(topt_p1, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex centtop_t_v3(topb_p1, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex centtop_t_v4(topb_p5, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex centtop_b_v1(bott_p5, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex centtop_b_v2(bott_p1, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex centtop_b_v3(botb_p1, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex centtop_b_v4(botb_p5, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex centbot_m_v1(topb_p2, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex centbot_m_v2(topb_p4, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex centbot_m_v3(bott_p4, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex centbot_m_v4(bott_p2, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex centbot_t_v1(topt_p2, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex centbot_t_v2(topt_p4, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex centbot_t_v3(topb_p4, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex centbot_t_v4(topb_p2, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex centbot_b_v1(bott_p2, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex centbot_b_v2(bott_p4, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex centbot_b_v3(botb_p4, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex centbot_b_v4(botb_p2, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex corntl_m_v1(topb_p2, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex corntl_m_v2(topb_p6, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex corntl_m_v3(bott_p6, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex corntl_m_v4(bott_p2, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex corntl_t_v1(topt_p2, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex corntl_t_v2(topt_p6, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex corntl_t_v3(topb_p6, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex corntl_t_v4(topb_p2, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex corntl_b_v1(bott_p2, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex corntl_b_v2(bott_p6, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex corntl_b_v3(botb_p6, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex corntl_b_v4(botb_p2, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex corntr_m_v1(topb_p6, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex corntr_m_v2(topb_p4, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex corntr_m_v3(bott_p4, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex corntr_m_v4(bott_p6, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex corntr_t_v1(topt_p6, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex corntr_t_v2(topt_p4, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex corntr_t_v3(topb_p4, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex corntr_t_v4(topb_p6, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex corntr_b_v1(bott_p6, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex corntr_b_v2(bott_p4, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex corntr_b_v3(botb_p4, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex corntr_b_v4(botb_p6, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

	HSVertex cornbl_m_v1(topb_p3, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex cornbl_m_v2(topb_p1, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex cornbl_m_v3(bott_p1, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex cornbl_m_v4(bott_p3, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex cornbl_t_v1(topt_p3, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex cornbl_t_v2(topt_p1, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex cornbl_t_v3(topb_p1, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex cornbl_t_v4(topb_p3, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex cornbl_b_v1(bott_p3, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex cornbl_b_v2(bott_p1, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex cornbl_b_v3(botb_p1, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex cornbl_b_v4(botb_p3, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);
	
	HSVertex cornbr_m_v1(topb_p5, suv1, amb_topb_uvl, glm::vec3(), color);
	HSVertex cornbr_m_v2(topb_p3, suv2, amb_topb_uvr, glm::vec3(), color);
	HSVertex cornbr_m_v3(bott_p3, suv3, amb_bott_uvr, glm::vec3(), color);
	HSVertex cornbr_m_v4(bott_p5, suv4, amb_bott_uvl, glm::vec3(), color);
	HSVertex cornbr_t_v1(topt_p5, sidetop_points[3], amb_topt_uvl, glm::vec3(), color);
	HSVertex cornbr_t_v2(topt_p3, sidetop_points[2], amb_topt_uvr, glm::vec3(), color);
	HSVertex cornbr_t_v3(topb_p3, sidetop_points[1], amb_topb_uvr, glm::vec3(), color);
	HSVertex cornbr_t_v4(topb_p5, sidetop_points[0], amb_topb_uvl, glm::vec3(), color);
	HSVertex cornbr_b_v1(bott_p5, sidebot_points[3], amb_bott_uvl, glm::vec3(), color);
	HSVertex cornbr_b_v2(bott_p3, sidebot_points[2], amb_bott_uvr, glm::vec3(), color);
	HSVertex cornbr_b_v3(botb_p3, sidebot_points[1], amb_botb_uvr, glm::vec3(), color);
	HSVertex cornbr_b_v4(botb_p5, sidebot_points[0], amb_botb_uvl, glm::vec3(), color);

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

	switch (tile_type)
	{
		case Tileset::TILE_FULL:
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
		case Tileset::TILE_LEFT:
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
		case Tileset::TILE_RIGHT:
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
		case Tileset::TILE_TOP:
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
		case Tileset::TILE_BOTTOM:
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
		case Tileset::TILE_MID:
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
		case Tileset::TILE_CORNER_TL:
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
		case Tileset::TILE_CORNER_TR:
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
		case Tileset::TILE_CORNER_BL:
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
		case Tileset::TILE_CORNER_BR:
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
			m_vbo->degenTris(vbo_index, m_vbo->getCapacity());
			break;
		}
	}

	if (render_sides & RENDER_LEFT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, left_m_v1, left_m_v2, left_m_v3, left_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, left_t_v1, left_t_v2, left_t_v3, left_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, left_b_v1, left_b_v2, left_b_v3, left_b_v4);
	}
	if (render_sides & RENDER_TOPLEFT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topleft_m_v1, topleft_m_v2, topleft_m_v3, topleft_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topleft_t_v1, topleft_t_v2, topleft_t_v3, topleft_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topleft_b_v1, topleft_b_v2, topleft_b_v3, topleft_b_v4);
	}
	if (render_sides & RENDER_TOPRIGHT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topright_m_v1, topright_m_v2, topright_m_v3, topright_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topright_t_v1, topright_t_v2, topright_t_v3, topright_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, topright_b_v1, topright_b_v2, topright_b_v3, topright_b_v4);
	}
	if (render_sides & RENDER_RIGHT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, right_m_v1, right_m_v2, right_m_v3, right_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, right_t_v1, right_t_v2, right_t_v3, right_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, right_b_v1, right_b_v2, right_b_v3, right_b_v4);
	}
	if (render_sides & RENDER_BOTRIGHT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botright_m_v1, botright_m_v2, botright_m_v3, botright_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botright_t_v1, botright_t_v2, botright_t_v3, botright_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botright_b_v1, botright_b_v2, botright_b_v3, botright_b_v4);
	}
	if (render_sides & RENDER_BOTLEFT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botleft_m_v1, botleft_m_v2, botleft_m_v3, botleft_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botleft_t_v1, botleft_t_v2, botleft_t_v3, botleft_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, botleft_b_v1, botleft_b_v2, botleft_b_v3, botleft_b_v4);
	}
	if (render_sides & RENDER_SIDELEFT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideleft_m_v1, sideleft_m_v2, sideleft_m_v3, sideleft_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideleft_t_v1, sideleft_t_v2, sideleft_t_v3, sideleft_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideleft_b_v1, sideleft_b_v2, sideleft_b_v3, sideleft_b_v4);
	}
	if (render_sides & RENDER_SIDERIGHT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideright_m_v1, sideright_m_v2, sideright_m_v3, sideright_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideright_t_v1, sideright_t_v2, sideright_t_v3, sideright_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, sideright_b_v1, sideright_b_v2, sideright_b_v3, sideright_b_v4);
	}
	if (render_sides & RENDER_MIDTOP)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midtop_m_v1, midtop_m_v2, midtop_m_v3, midtop_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midtop_t_v1, midtop_t_v2, midtop_t_v3, midtop_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midtop_b_v1, midtop_b_v2, midtop_b_v3, midtop_b_v4);
	}
	if (render_sides & RENDER_MIDBOT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midbot_m_v1, midbot_m_v2, midbot_m_v3, midbot_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midbot_t_v1, midbot_t_v2, midbot_t_v3, midbot_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, midbot_b_v1, midbot_b_v2, midbot_b_v3, midbot_b_v4);
	}
	if (render_sides & RENDER_CENTER_TOP)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centtop_m_v1, centtop_m_v2, centtop_m_v3, centtop_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centtop_t_v1, centtop_t_v2, centtop_t_v3, centtop_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centtop_b_v1, centtop_b_v2, centtop_b_v3, centtop_b_v4);
	}
	if (render_sides & RENDER_CENTER_BOT)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centbot_m_v1, centbot_m_v2, centbot_m_v3, centbot_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centbot_t_v1, centbot_t_v2, centbot_t_v3, centbot_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, centbot_b_v1, centbot_b_v2, centbot_b_v3, centbot_b_v4);
	}
	if (render_sides & RENDER_CORNER_TL)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntl_m_v1, corntl_m_v2, corntl_m_v3, corntl_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntl_t_v1, corntl_t_v2, corntl_t_v3, corntl_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntl_b_v1, corntl_b_v2, corntl_b_v3, corntl_b_v4);
	}
	if (render_sides & RENDER_CORNER_TR)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntr_m_v1, corntr_m_v2, corntr_m_v3, corntr_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntr_t_v1, corntr_t_v2, corntr_t_v3, corntr_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, corntr_b_v1, corntr_b_v2, corntr_b_v3, corntr_b_v4);
	}
	if (render_sides & RENDER_CORNER_BL)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbl_m_v1, cornbl_m_v2, cornbl_m_v3, cornbl_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbl_t_v1, cornbl_t_v2, cornbl_t_v3, cornbl_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbl_b_v1, cornbl_b_v2, cornbl_b_v3, cornbl_b_v4);
	}
	if (render_sides & RENDER_CORNER_BR)
	{
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbr_m_v1, cornbr_m_v2, cornbr_m_v3, cornbr_m_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbr_t_v1, cornbr_t_v2, cornbr_t_v3, cornbr_t_v4);
		vbo_index += m_vbo->makeQuadPolyNorm(vbo_index, cornbr_b_v1, cornbr_b_v2, cornbr_b_v3, cornbr_b_v4);
	}

	if (vbo_index < m_vbo->getCapacity())
		m_vbo->degenTris(vbo_index, m_vbo->getCapacity() - vbo_index);
#endif
	
	update();
}

void TilePreview::setTopHeight(float height)
{
	m_top_height = height;

	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}

void TilePreview::setTopUVs(PolygonDef* uvs)
{
	assert(uvs->getNumPoints() <= 6);

	for (int i = 0; i < uvs->getNumPoints(); i++)
	{
		m_top_points[i] = uvs->getPoint(i);
	}

	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}

void TilePreview::setSideUVs(PolygonDef* uvs)
{
	assert(uvs->getNumPoints() == 4);

	for (int i = 0; i < 4; i++)
	{
		m_side_points[i] = uvs->getPoint(i);
	}

	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}

void TilePreview::setSideTopUVs(PolygonDef* uvs)
{
	assert(uvs->getNumPoints() == 4);

	for (int i = 0; i < 4; i++)
	{
		m_sidetop_points[i] = uvs->getPoint(i);
	}

	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}

void TilePreview::setSideBotUVs(PolygonDef* uvs)
{
	assert(uvs->getNumPoints() == 4);

	for (int i = 0; i < 4; i++)
	{
		m_sidebot_points[i] = uvs->getPoint(i);
	}

	updateGeo(m_top_points, m_side_points, m_sidetop_points, m_sidebot_points, m_tile_type, m_top_type, m_shading_type, m_top_height, m_color);
}


QImage TilePreview::makeThumbnail(PolygonDef* top_points, PolygonDef* side_points,
	PolygonDef* sidetop_points, PolygonDef* sidebot_points,
	Tileset::TileType tile_type,
	Tileset::TopType top_type,
	Tileset::ShadingType shading_type,
	float top_height,
	unsigned int color)
{
	glm::vec2 top[6];
	glm::vec2 side[4];
	glm::vec2 sidetop[4];
	glm::vec2 sidebot[4];

	for (int i = 0; i < top_points->getNumPoints(); i++)
	{
		top[i] = top_points->getPoint(i);
	}

	for (int i=0; i < side_points->getNumPoints(); i++)
	{
		side[i] = side_points->getPoint(i);
	}
	for (int i = 0; i < sidetop_points->getNumPoints(); i++)
	{
		sidetop[i] = sidetop_points->getPoint(i);
	}
	for (int i = 0; i < sidebot_points->getNumPoints(); i++)
	{
		sidebot[i] = sidebot_points->getPoint(i);
	}

	updateGeo(top, side, sidetop, sidebot, tile_type, top_type, shading_type, top_height, color);

	repaint();
	return grabFrameBuffer();
}