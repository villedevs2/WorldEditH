#include "Preview.h"




GLPreview::GLPreview(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_level = level;

	m_scroll = glm::vec2(16.0f, 16.0f);
	m_scroll_saved = m_scroll;

	m_width = 0;
	m_height = 0;

	m_vb = nullptr;
	m_vbback = nullptr;

	resizeTilemap(50, 50);
}

GLPreview::~GLPreview()
{

}

QString GLPreview::loadShader(QString filename)
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

void GLPreview::loadTexture(QImage* texture)
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
}

void GLPreview::initializeGL()
{
	QString vs_file = loadShader("homestar_vs.glsl");
	QString fs_file = loadShader("homestar_fs.glsl");

	m_level_program = new QGLShaderProgram(this);
	m_level_program->addShaderFromSourceCode(QGLShader::Vertex, vs_file);
	QString log1 = m_level_program->log();
	m_level_program->addShaderFromSourceCode(QGLShader::Fragment, fs_file);
	QString log2 = m_level_program->log();
	m_level_program->link();

	QString error = m_level_program->log();
	std::string errors = error.toStdString();

	m_level_shader.position = m_level_program->attributeLocation("a_position");
	m_level_shader.tex_coord = m_level_program->attributeLocation("a_texcoord");
	m_level_shader.color = m_level_program->attributeLocation("a_color");
	m_level_shader.vp_matrix = m_level_program->uniformLocation("m_vp_matrix");

	float zh = 2.0f;
	
	//m_vb->makeQuad(0, p1, p2, p3, p4, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0xff808080);
	/*
	m_vb->makeQuad(2, tp1, tp2, tp3, tp4, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0xff808080);
	m_vb->makeQuad(4, bp1, bp2, tp2, tp1, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0xffffffff);
	m_vb->makeQuad(6, tp2, bp2, bp3, tp3, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0xff303030);
	m_vb->makeQuad(8, tp4, tp3, bp3, bp4, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0xff101010);
	*/
}

void GLPreview::paintGL()
{
	makeCurrent();

	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());

	QPainter painter;
	painter.begin(this);

	painter.beginNativePainting();

	// opengl scene rendering
	// --------------------------------------------------------------------------
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	qglClearColor(QColor(0,64,192));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	
	float aspect = (float)(width()) / (float)(height());
	float halfw = (float)((LEVEL_VIS_WIDTH) / 2);
	float halfh = halfw / aspect;

	const double fov = 60.0;

	float size = 0.01f * (float)tan((fov * M_PI / 180.0) / 2);

	float camera_distance = ((float)(LEVEL_VIS_WIDTH) / 2) / tan((fov * M_PI / 180.0) / 2);


	glm::vec3 pos = glm::vec3(m_scroll.x, m_scroll.y+0.0f, camera_distance);
	glm::vec3 eye = glm::vec3(m_scroll.x, m_scroll.y, 0.0f);

	glm::mat4 camera_proj_matrix = glm::frustum<float>(-size, size, size / aspect, -size / aspect, 0.01f, 100.0f);
	glm::mat4 camera_view_matrix = glm::lookAt(pos, eye, glm::vec3(0.0f, 1.0f, 0.0));
	glm::mat4 camera_vp_matrix = camera_proj_matrix * camera_view_matrix;
	

	QMatrix4x4 vp_mat = QMatrix4x4(glm::value_ptr(camera_vp_matrix));

	m_level_program->bind();

	m_level_program->setUniformValue(m_level_shader.vp_matrix, vp_mat);

	void* vbptr;
	int vbsize;
	int capacity;

	vbptr = m_vbback->getPointer();
	vbsize = m_vbback->getVertexSize();

	{
		m_level_program->enableAttributeArray(m_level_shader.position);
		m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)vbptr, 3, vbsize);
		m_level_program->enableAttributeArray(m_level_shader.tex_coord);
		m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)vbptr + 3, 2, vbsize);
		m_level_program->enableAttributeArray(m_level_shader.color);
		m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)vbptr + 20, 4, vbsize);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		m_level_program->disableAttributeArray(m_level_shader.position);
		m_level_program->disableAttributeArray(m_level_shader.tex_coord);
		m_level_program->disableAttributeArray(m_level_shader.color);
	}

	vbptr = m_vb->getPointer();
	vbsize = m_vb->getVertexSize();
	capacity = m_vb->getCapacity();

	if (capacity > 0)
	{
		m_level_program->enableAttributeArray(m_level_shader.position);
		m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)vbptr, 3, vbsize);
		m_level_program->enableAttributeArray(m_level_shader.tex_coord);
		m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)vbptr + 3, 2, vbsize);
		m_level_program->enableAttributeArray(m_level_shader.color);
		m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)vbptr + 20, 4, vbsize);

		glBindTexture(GL_TEXTURE_2D, m_base_tex);
		glDrawArrays(GL_TRIANGLES, 0, capacity*3);

		m_level_program->disableAttributeArray(m_level_shader.position);
		m_level_program->disableAttributeArray(m_level_shader.tex_coord);
		m_level_program->disableAttributeArray(m_level_shader.color);
	}



	painter.endNativePainting();
	painter.end();
	doneCurrent();
}

void GLPreview::resizeGL(int width, int height)
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

void GLPreview::setTexture(QImage* texture)
{
	m_texture = texture;
	loadTexture(m_texture);

	tesselateAll();

	update();
}

void GLPreview::mouseReleaseEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	if (event->button() & Qt::RightButton)
	{
		if (m_panning)
		{
			m_panning = false;
		}
	}

	update();
}

void GLPreview::mousePressEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	if (event->button() & Qt::RightButton)
	{
		if (!m_panning)
		{
			m_pan_point = toLevelCoords(glm::vec2(mouse_x, mouse_y));
			m_scroll_saved = m_scroll;
			m_panning = true;
		}
	}

	update();
}

void GLPreview::mouseMoveEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	
	if (m_panning)
	{
		glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));
		glm::vec2 delta = (mouse_p - m_pan_point) * 0.4f;
		m_scroll = m_scroll_saved - delta;
	}

	update();
}

void GLPreview::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();

	update();
}

void GLPreview::paintEvent(QPaintEvent* event)
{
	paintGL();
}

void GLPreview::tesselateTile(int x, int y)
{
	const int num_tris = 4;
	
	const float tile_width = 1.0f;
	const float tile_height = 1.4f;

	assert(x >= 0 && x < m_level->getTilemapWidth());
	assert(y >= 0 && y < m_level->getTilemapHeight());

	int ctile = m_level->readTilemapTile(x, y);
	int vb_index = ((y * m_width) + x) * num_tris;

	float tx1 = (float)(x)* tile_width;
	float tx2 = tx1 + tile_width;
	float ty1 = (float)(y) * (tile_height / 2);
	float ty2 = ty1 + (tile_height / 2);

	if (y & 1)
	{
		tx1 += tile_width / 2;
		tx2 += tile_width / 2;
	}

	if (ctile == Tilemap::TILE_EMPTY)
	{
		// make degen geo
		for (int i = 0; i < num_tris; i++)
		{
			m_vb->degenTri(vb_index + i);
		}
	}
	else
	{
		const Tilemap::Tile* tiledata = m_level->getTile(ctile);

		float z = m_level->readTilemapZ(x, y) * 0.1f;

		glm::vec2 vl = tiledata->top_points[1] - tiledata->top_points[0];
		glm::vec2 vr = tiledata->top_points[2] - tiledata->top_points[3];
		glm::vec2 vt = tiledata->top_points[3] - tiledata->top_points[0];
		glm::vec2 vb = tiledata->top_points[2] - tiledata->top_points[1];

		glm::vec2 uv1 = tiledata->top_points[0];
		glm::vec2 uv2 = tiledata->top_points[1];
		glm::vec2 uv3 = tiledata->top_points[2];
		glm::vec2 uv4 = tiledata->top_points[3];
		glm::vec2 uv5 = tiledata->top_points[4];
		glm::vec2 uv6 = tiledata->top_points[5];

		/*
		         p6
		    p1         p5
		    p2         p4
		         p3
		*/

		glm::vec3 p1 = glm::vec3(tx1, ty1 + (tile_height * (15.0 / 70.0)), z);
		glm::vec3 p2 = glm::vec3(tx1, ty1 + (tile_height * (35.0 / 70.0)), z);
		glm::vec3 p3 = glm::vec3(tx1 + (tile_width * 0.5), ty1 + (tile_height * (50.0 / 70.0)), z);
		glm::vec3 p4 = glm::vec3(tx2, ty1 + (tile_height * (35.0 / 70.0)), z);
		glm::vec3 p5 = glm::vec3(tx2, ty1 + (tile_height * (15.0 / 70.0)), z);
		glm::vec3 p6 = glm::vec3(tx1 + (tile_width * 0.5), ty1, z);

		switch (tiledata->type)
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

				m_vb->makeTri(vb_index + 0, p1, p6, p5, uv1, uv6, uv5, tiledata->color);
				m_vb->makeTri(vb_index + 1, p1, p5, p4, uv1, uv5, uv4, tiledata->color);
				m_vb->makeTri(vb_index + 2, p1, p4, p3, uv1, uv4, uv3, tiledata->color);
				m_vb->makeTri(vb_index + 3, p1, p3, p2, uv1, uv3, uv2, tiledata->color);
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
				m_vb->makeTri(vb_index + 0, p1, p6, p3, uv1, uv4, uv3, tiledata->color);
				m_vb->makeTri(vb_index + 1, p1, p3, p2, uv1, uv3, uv2, tiledata->color);
				m_vb->degenTri(vb_index + 2);
				m_vb->degenTri(vb_index + 3);
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
				m_vb->makeTri(vb_index + 0, p6, p5, p4, uv2, uv1, uv4, tiledata->color);
				m_vb->makeTri(vb_index + 1, p6, p4, p3, uv2, uv4, uv3, tiledata->color);
				m_vb->degenTri(vb_index + 2);
				m_vb->degenTri(vb_index + 3);
				break;
			}
			case Tilemap::TILE_TOP:
			{
			/*
			     /\
			    /__\
			*/
				m_vb->makeTri(vb_index + 0, p1, p6, p5, uv1, uv3, uv2, tiledata->color);
				m_vb->degenTri(vb_index + 1);
				m_vb->degenTri(vb_index + 2);
				m_vb->degenTri(vb_index + 3);
				break;
			}
			case Tilemap::TILE_BOTTOM:
			{
			/* ____
			   \  /
			    \/
			*/
				m_vb->makeTri(vb_index + 0, p2, p4, p3, uv1, uv3, uv2, tiledata->color);
				m_vb->degenTri(vb_index + 1);
				m_vb->degenTri(vb_index + 2);
				m_vb->degenTri(vb_index + 3);
				break;
			}
			case Tilemap::TILE_MID:
			{
			/*  ______
			    |    |
			    |____|
			*/
				m_vb->makeTri(vb_index + 0, p1, p5, p4, uv1, uv4, uv3, tiledata->color);
				m_vb->makeTri(vb_index + 1, p1, p4, p2, uv1, uv3, uv2, tiledata->color);
				m_vb->degenTri(vb_index + 2);
				m_vb->degenTri(vb_index + 3);
				break;
			}

			default:
			{
				// make degen geo
				for (int i = 0; i < num_tris; i++)
				{
					m_vb->degenTri(vb_index + i);
				}
				break;
			}
		}
	}

	update();
}

void GLPreview::tesselateAll()
{
	for (int j = 0; j < m_level->getTilemapHeight(); j++)
	{
		for (int i = 0; i < m_level->getTilemapWidth(); i++)
		{
			tesselateTile(i, j);
		}
	}
}

// convert screen coordinates to uv coords
glm::vec2 GLPreview::toLevelCoords(glm::vec2& point)
{
	float x = point.x;
	float y = point.y;

	float mult = 1.0f / ((float)(width()) / (float)(LEVEL_VIS_WIDTH));

	float sx = (x * mult) - m_scroll.x;
	float sy = (y * mult) - m_scroll.y;

	return glm::vec2(sx, sy);
}


void GLPreview::resizeTilemap(int width, int height)
{
	if (m_vb != nullptr)
		delete m_vb;

	m_vb = new VBO(width * height * 16);

	if (m_vbback != nullptr)
		delete m_vbback;

	m_vbback = new VBO(2);

	m_vbback->makeQuad(0, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(width, 0.0f, -1.0f), glm::vec3(width, height, -1.0f), glm::vec3(0.0f, height, -1.0f),
						  glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0x00000000);

	m_width = width;
	m_height = height;

	tesselateAll();
}







PreviewWindow::PreviewWindow(QWidget* parent, Level* level) : QDockWidget("Preview", parent, 0)
{
	m_level = level;

	m_widget = new GLPreview(parent, m_level);
	m_window = new QMainWindow(0);

	setFocusPolicy(Qt::ClickFocus);

	m_window->setParent(this);
	setWidget(m_window);
	m_window->setCentralWidget(m_widget);
	m_window->setContextMenuPolicy(Qt::NoContextMenu);

	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	this->setMinimumWidth(960);
	this->setMinimumHeight(540);
	this->setMaximumWidth(960);
	this->setMaximumHeight(540);

	// don't allow docking
	this->setAllowedAreas(0);

	this->setHidden(false);
}

PreviewWindow::~PreviewWindow()
{

}

void PreviewWindow::closeEvent(QCloseEvent* event)
{
	emit onClose();
}

void PreviewWindow::tileUpdated(int x, int y)
{
	m_widget->tesselateTile(x, y);
}


void PreviewWindow::setTexture(QImage* texture)
{
	m_widget->setTexture(texture);
}

void PreviewWindow::resizeTilemap(int width, int height)
{
	m_widget->resizeTilemap(width, height);
}