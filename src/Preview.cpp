#include "Preview.h"




GLPreview::GLPreview(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_scroll = glm::vec2(0.0f, 0.0f);
	m_scroll_saved = glm::vec2(0.0f, 0.0f);
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

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
	m_level_shader.location = m_level_program->uniformLocation("v_location");
	m_level_shader.scale = m_level_program->uniformLocation("v_scale");
	m_level_shader.vp_matrix = m_level_program->uniformLocation("m_vp_matrix");

	m_vbo[0].pos = glm::vec3(0.0f, 0.0f, 0.0f);		m_vbo[0].uv = glm::vec2(0.0f, 0.0f);		m_vbo[0].color = 0xffffffff;
	m_vbo[1].pos = glm::vec3(0.0f, 32.0f, 0.0f);	m_vbo[1].uv = glm::vec2(0.0f, 1.0f);		m_vbo[1].color = 0xffffffff;
	m_vbo[2].pos = glm::vec3(32.0f, 32.0f, 0.0f);	m_vbo[2].uv = glm::vec2(1.0f, 1.0f);		m_vbo[2].color = 0xffffffff;

	m_vbo[3].pos = glm::vec3(0.0f, 0.0f, 0.0f);		m_vbo[3].uv = glm::vec2(0.0f, 0.0f);		m_vbo[3].color = 0xffffffff;
	m_vbo[4].pos = glm::vec3(32.0f, 0.0f, 0.0f);	m_vbo[4].uv = glm::vec2(1.0f, 0.0f);		m_vbo[4].color = 0xffffffff;
	m_vbo[5].pos = glm::vec3(32.0f, 32.0f, 0.0f);	m_vbo[5].uv = glm::vec2(1.0f, 1.0f);		m_vbo[5].color = 0xffffffff;

	// --
	m_vbo[6].pos = glm::vec3(4.0f, 5.0f, 0.0f);		m_vbo[6].uv = glm::vec2(0.0f, 1.0f);		m_vbo[6].color = 0xffffffff;
	m_vbo[7].pos = glm::vec3(4.0f, 4.0f, 0.0f);		m_vbo[7].uv = glm::vec2(1.0f, 1.0f);		m_vbo[7].color = 0xffffffff;
	m_vbo[8].pos = glm::vec3(4.0f, 4.0f, 10.0f);	m_vbo[8].uv = glm::vec2(1.0f, 0.0f);		m_vbo[8].color = 0xffffffff;
	m_vbo[9].pos = glm::vec3(4.0f, 5.0f, 0.0f);		m_vbo[9].uv = glm::vec2(0.0f, 1.0f);		m_vbo[9].color = 0xffffffff;
	m_vbo[10].pos = glm::vec3(4.0f, 5.0f, 10.0f);	m_vbo[10].uv = glm::vec2(0.0f, 0.0f);		m_vbo[10].color = 0xffffffff;
	m_vbo[11].pos = glm::vec3(4.0f, 4.0f, 10.0f);	m_vbo[11].uv = glm::vec2(1.0f, 0.0f);		m_vbo[11].color = 0xffffffff;

	// --
	m_vbo[12].pos = glm::vec3(3.0f, 5.0f, 0.0f);	m_vbo[12].uv = glm::vec2(0.0f, 1.0f);		m_vbo[12].color = 0xffffffff;
	m_vbo[13].pos = glm::vec3(4.0f, 5.0f, 0.0f);	m_vbo[13].uv = glm::vec2(1.0f, 1.0f);		m_vbo[13].color = 0xffffffff;
	m_vbo[14].pos = glm::vec3(4.0f, 5.0f, 10.0f);	m_vbo[14].uv = glm::vec2(1.0f, 0.0f);		m_vbo[14].color = 0xffffffff;
	m_vbo[15].pos = glm::vec3(3.0f, 5.0f, 0.0f);	m_vbo[15].uv = glm::vec2(0.0f, 1.0f);		m_vbo[15].color = 0xffffffff;
	m_vbo[16].pos = glm::vec3(3.0f, 5.0f, 10.0f);	m_vbo[16].uv = glm::vec2(0.0f, 0.0f);		m_vbo[16].color = 0xffffffff;
	m_vbo[17].pos = glm::vec3(4.0f, 5.0f, 10.0f);	m_vbo[17].uv = glm::vec2(1.0f, 0.0f);		m_vbo[17].color = 0xffffffff;

	// --
	m_vbo[18].pos = glm::vec3(3.0f, 4.0f, 0.0f);	m_vbo[18].uv = glm::vec2(0.0f, 1.0f);		m_vbo[18].color = 0xffffffff;
	m_vbo[19].pos = glm::vec3(4.0f, 4.0f, 10.0f);	m_vbo[19].uv = glm::vec2(1.0f, 0.0f);		m_vbo[19].color = 0xffffffff;
	m_vbo[20].pos = glm::vec3(4.0f, 4.0f, 0.0f);	m_vbo[20].uv = glm::vec2(1.0f, 1.0f);		m_vbo[20].color = 0xffffffff;
	m_vbo[21].pos = glm::vec3(3.0f, 4.0f, 0.0f);	m_vbo[21].uv = glm::vec2(0.0f, 1.0f);		m_vbo[21].color = 0xffffffff;
	m_vbo[22].pos = glm::vec3(3.0f, 4.0f, 10.0f);	m_vbo[22].uv = glm::vec2(0.0f, 0.0f);		m_vbo[22].color = 0xffffffff;
	m_vbo[23].pos = glm::vec3(4.0f, 4.0f, 10.0f);	m_vbo[23].uv = glm::vec2(1.0f, 0.0f);		m_vbo[23].color = 0xffffffff;

	// --
	m_vbo[24].pos = glm::vec3(3.0f, 4.0f, 10.0f);	m_vbo[24].uv = glm::vec2(0.0f, 0.0f);		m_vbo[24].color = 0xffffffff;
	m_vbo[25].pos = glm::vec3(4.0f, 4.0f, 10.0f);	m_vbo[25].uv = glm::vec2(1.0f, 0.0f);		m_vbo[25].color = 0xffffffff;
	m_vbo[26].pos = glm::vec3(4.0f, 5.0f, 10.0f);	m_vbo[26].uv = glm::vec2(1.0f, 1.0f);		m_vbo[26].color = 0xffffffff;
	m_vbo[27].pos = glm::vec3(3.0f, 4.0f, 10.0f);	m_vbo[27].uv = glm::vec2(0.0f, 0.0f);		m_vbo[27].color = 0xffffffff;
	m_vbo[28].pos = glm::vec3(3.0f, 5.0f, 10.0f);	m_vbo[28].uv = glm::vec2(0.0f, 1.0f);		m_vbo[28].color = 0xffffffff;
	m_vbo[29].pos = glm::vec3(4.0f, 5.0f, 10.0f);	m_vbo[29].uv = glm::vec2(1.0f, 1.0f);		m_vbo[29].color = 0xffffffff;
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
	glDisable(GL_DEPTH_TEST);

	qglClearColor(QColor(0,64,192));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	
	float aspect = (float)(width()) / (float)(height());
	float halfw = (float)((LEVEL_VIS_WIDTH) / 2);
	float halfh = halfw / aspect;

	float size = 0.01f * (float)tan((100.0 * M_PI / 180.0) / 2);


	glm::vec3 pos = glm::vec3(20.0f, 20.0f, 60.0f);
	glm::vec3 eye = glm::vec3(0.0f, 40.0f, 0.0f);

	//glm::mat4 camera_proj_matrix = glm::frustum<float>(size, size, size / aspect, size / aspect, 0.01f, 100.0f);
	glm::mat4 camera_proj_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	glm::mat4 camera_view_matrix = glm::lookAt(pos, eye, glm::vec3(0.0f, 1.0f, 0.0));
	glm::mat4 camera_vp_matrix = camera_proj_matrix * camera_view_matrix;



	// setup matrices
	float level_vp_mat[4] = { 1.0f / halfw, 0.0f,
		0.0f, 1.0f / halfh };

	float level_rot_mat[4] = { 1.0f, 0.0f,
		0.0f, 1.0f };
	

	QMatrix4x4 vp_mat = QMatrix4x4(glm::value_ptr(camera_vp_matrix));

	m_level_program->bind();

	m_level_program->setUniformValue(m_level_shader.vp_matrix, vp_mat);
	m_level_program->setUniformValue(m_level_shader.location, m_scroll.x, m_scroll.y);

	m_level_program->enableAttributeArray(m_level_shader.position);
	m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)m_vbo, 3, sizeof(VBO));
	m_level_program->enableAttributeArray(m_level_shader.tex_coord);
	m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)m_vbo + 3, 2, sizeof(VBO));
	m_level_program->enableAttributeArray(m_level_shader.color);
	m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)m_vbo + 20, 4, sizeof(VBO));

	glBindTexture(GL_TEXTURE_2D, m_base_tex);
	glDrawArrays(GL_TRIANGLES, 0, 30);

	m_level_program->disableAttributeArray(m_level_shader.position);
	m_level_program->disableAttributeArray(m_level_shader.tex_coord);
	m_level_program->disableAttributeArray(m_level_shader.color);
	



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
		glm::vec2 delta = (m_pan_point - mouse_p) * 0.4f;
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
	update();
}

void GLPreview::tesselateAll()
{

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