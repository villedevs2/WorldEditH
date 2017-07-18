#include "Preview.h"




GLPreview::GLPreview(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_level = level;

	m_scroll = glm::vec2(0.0f, 0.0f);
	m_scroll_saved = m_scroll;

	m_vbback = new VBO(2);

	glm::vec3 p1(0.0f, 0.0, 0.0f);
	glm::vec3 p2(Tilemap::AREA_WIDTH, 0.0f, 0.0f);
	glm::vec3 p3(Tilemap::AREA_WIDTH, Tilemap::AREA_HEIGHT, 0.0f);
	glm::vec3 p4(0.0f, Tilemap::AREA_HEIGHT, 0.0f);

	m_vbback->makeQuad(0, p1, p2, p3, p4, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), 0xff404040);
}

GLPreview::~GLPreview()
{
	delete m_vbback;
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

	glm::vec2 cam_pos = m_scroll + glm::vec2(halfw, halfh);


	glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y + 0.0f, camera_distance);
	glm::vec3 eye = glm::vec3(cam_pos.x, cam_pos.y, 0.0f);

	glm::mat4 camera_proj_matrix = glm::frustum<float>(-size, size, size / aspect, -size / aspect, 0.01f, 100.0f);
	glm::mat4 camera_view_matrix = glm::lookAt(pos, eye, glm::vec3(0.0f, 1.0f, 0.0));
	glm::mat4 camera_vp_matrix = camera_proj_matrix * camera_view_matrix;
	

	QMatrix4x4 vp_mat = QMatrix4x4(glm::value_ptr(camera_vp_matrix));

	m_level_program->bind();

	m_level_program->setUniformValue(m_level_shader.vp_matrix, vp_mat);

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
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

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	
	
	/*
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
	*/


	
	glBindTexture(GL_TEXTURE_2D, m_base_tex);
	
	glm::vec2 tilemap_tl = toLevelCoords(glm::vec2(0, 0));
	glm::vec2 tilemap_br = toLevelCoords(glm::vec2(width(), height()));

	tilemap_tl.x *= m_level->getTileWidth();
	tilemap_tl.y *= m_level->getTileHeight();
	tilemap_br.x *= m_level->getTileWidth();
	tilemap_br.y *= m_level->getTileHeight();

	int xs = (int)(floor(tilemap_tl.x / Tilemap::BUCKET_WIDTH));
	int ys = (int)(floor(tilemap_tl.y / Tilemap::BUCKET_HEIGHT));
	int xe = (int)(ceil(tilemap_br.x / Tilemap::BUCKET_WIDTH));
	int ye = (int)(ceil(tilemap_br.y / Tilemap::BUCKET_HEIGHT));

	if (xs < 0)
		xs = 0;
	if (ys < 0)
		ys = 0;
	if (xe >(Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH))
		xe = Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH;
	if (ye >(Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT))
		ye = Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT;
	
	for (int j = ys; j < ye; j++)
	{
		for (int i = xs; i < xe; i++)
		{
			const Tilemap::Bucket* bucket = m_level->getTileBucket(i, j);
			if (bucket != nullptr)
			{
				float* geo = (float*)bucket->preview->getPointer();
				int vbsize = bucket->preview->getVertexSize();

				m_level_program->enableAttributeArray(m_level_shader.position);
				m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)geo, 3, vbsize);
				m_level_program->enableAttributeArray(m_level_shader.tex_coord);
				m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)geo + 3, 2, vbsize);
				m_level_program->enableAttributeArray(m_level_shader.color);
				m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)geo + 20, 4, vbsize);

				glDrawArrays(GL_TRIANGLES, 0, Tilemap::BUCKET_WIDTH*Tilemap::BUCKET_HEIGHT * 16 * 3);

				m_level_program->disableAttributeArray(m_level_shader.position);
				m_level_program->disableAttributeArray(m_level_shader.tex_coord);
				m_level_program->disableAttributeArray(m_level_shader.color);
			}
		}
	}
	
	glDisable(GL_DEPTH_TEST);	// depth must disabled, or QPainter stuff won't be visible

	painter.endNativePainting();

	/*
	painter.beginNativePainting();

	// opengl scene rendering
	// --------------------------------------------------------------------------
	glDisable(GL_CULL_FACE);

	qglClearColor(QColor(0, 64, 192));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	painter.endNativePainting();
	*/
	{
		glm::vec2 tl = toLevelCoords(glm::vec2(0, 0));
		glm::vec2 br = toLevelCoords(glm::vec2(width(), height()));

		tl.x *= m_level->getTileWidth();
		tl.y *= m_level->getTileHeight();
		br.x *= m_level->getTileWidth();
		br.y *= m_level->getTileHeight();

		int xs = (int)(floor(tl.x / Tilemap::BUCKET_WIDTH));
		int ys = (int)(floor(tl.y / Tilemap::BUCKET_HEIGHT));
		int xe = (int)(ceil(br.x / Tilemap::BUCKET_WIDTH));
		int ye = (int)(ceil(br.y / Tilemap::BUCKET_HEIGHT));

		painter.setPen(QColor(255, 255, 0, 255));
		painter.drawText(8, 32, tr("XS: %1, YS: %2, XE: %3, YE: %4").arg(xs).arg(ys).arg(xe).arg(ye));
		painter.drawText(8, 48, tr("TLX: %1, TLY: %2, BRX: %3, BRY: %4").arg(tl.x).arg(tl.y).arg(br.x).arg(br.y));
		painter.drawText(8, 64, tr("SX: %1, SY: %2").arg(m_scroll.x).arg(m_scroll.y));
	}

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

// convert screen coordinates to uv coords
glm::vec2 GLPreview::toLevelCoords(glm::vec2& point)
{
	float x = point.x;
	float y = point.y;

	float mult = 1.0f / ((float)(width()) / (float)(LEVEL_VIS_WIDTH));

	float sx = (x * mult) + m_scroll.x;
	float sy = (y * mult) + m_scroll.y;

	return glm::vec2(sx, sy);
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
	m_widget->update();
}


void PreviewWindow::setTexture(QImage* texture)
{
	m_widget->setTexture(texture);
}