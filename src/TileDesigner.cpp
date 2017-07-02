#include "TileDesigner.h"

TileDesignerWidget::TileDesignerWidget(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_level = level;

	setFocusPolicy(Qt::ClickFocus);

	m_poly[0] = new PolygonDef(6);
	m_poly[0]->reset();

	m_poly[1] = new PolygonDef(4);
	m_poly[1]->reset();

	m_poly_top_default[0] = new PolygonDef(6);
	m_poly_top_default[1] = new PolygonDef(6);
	m_poly_top_default[2] = new PolygonDef(6);
	m_poly_top_default[3] = new PolygonDef(6);
	m_poly_top_default[4] = new PolygonDef(6);
	m_poly_top_default[5] = new PolygonDef(6);
	m_poly_side_default = new PolygonDef(4);

	float tw = 0.125f;
	float th = 0.125f;
	float tx1 = 0.0f;
	float tx2 = 0.5f * tw;
	float tx3 = 1.0f * tw;
	float ty1 = 0.0f;
	float ty2 = (15.0 / 50.0) * th;
	float ty3 = (35.0 / 50.0) * th;
	float ty4 = (50.0 / 50.0) * th;

	m_poly_top_default[0]->insertPoint(glm::vec2(tx1, ty2));
	m_poly_top_default[0]->insertPoint(glm::vec2(tx1, ty3));
	m_poly_top_default[0]->insertPoint(glm::vec2(tx2, ty4));
	m_poly_top_default[0]->insertPoint(glm::vec2(tx3, ty3));
	m_poly_top_default[0]->insertPoint(glm::vec2(tx3, ty2));
	m_poly_top_default[0]->insertPoint(glm::vec2(tx2, ty1));

	m_poly_top_default[1]->insertPoint(glm::vec2(tx1, ty2));
	m_poly_top_default[1]->insertPoint(glm::vec2(tx1, ty3));
	m_poly_top_default[1]->insertPoint(glm::vec2(tx2, ty4));
	m_poly_top_default[1]->insertPoint(glm::vec2(tx2, ty1));

	m_poly_top_default[2]->insertPoint(glm::vec2(tx3, ty2));
	m_poly_top_default[2]->insertPoint(glm::vec2(tx2, ty1));
	m_poly_top_default[2]->insertPoint(glm::vec2(tx2, ty4));
	m_poly_top_default[2]->insertPoint(glm::vec2(tx3, ty3));

	m_poly_top_default[3]->insertPoint(glm::vec2(tx1, ty2));
	m_poly_top_default[3]->insertPoint(glm::vec2(tx3, ty2));
	m_poly_top_default[3]->insertPoint(glm::vec2(tx2, ty1));

	m_poly_top_default[4]->insertPoint(glm::vec2(tx1, ty3));	
	m_poly_top_default[4]->insertPoint(glm::vec2(tx2, ty4));
	m_poly_top_default[4]->insertPoint(glm::vec2(tx3, ty3));

	m_poly_top_default[5]->insertPoint(glm::vec2(tx1, ty2));
	m_poly_top_default[5]->insertPoint(glm::vec2(tx1, ty3));
	m_poly_top_default[5]->insertPoint(glm::vec2(tx3, ty3));
	m_poly_top_default[5]->insertPoint(glm::vec2(tx3, ty2));

	m_poly_side_default->insertPoint(glm::vec2(0.5f, 0.0f));
	m_poly_side_default->insertPoint(glm::vec2(0.5f, 0.25f));
	m_poly_side_default->insertPoint(glm::vec2(0.625f, 0.25f));
	m_poly_side_default->insertPoint(glm::vec2(0.625f, 0.0f));

	m_zoom = 1.0f;
	m_scroll = glm::vec2(0.0f, 0.0f);

	m_texture = NULL;

	m_color = 0xffffffff;

	m_validpoly_color = QColor(0, 224, 0, 128);
	m_errorpoly_color = QColor(224, 0, 0, 128);
	m_validline_color = QColor(0, 224, 0);
	m_errorline_color = QColor(224, 0, 0);
	m_point_color = QColor(224, 224, 224);
	m_closedpoly_color = QColor(224, 160, 0, 128);
	m_closedline_color = QColor(224, 160, 0, 128);

	m_bgcolor = QColor(48, 48, 48, 255);

	m_panning = false;

	m_move_dragging = false;

	m_show_grid = false;
	m_snap_grid = false;

	m_object_color = QColor(0, 0, 0, 255);

	m_poly[0]->copy(m_poly_top_default[0]);
	m_poly[1]->copy(m_poly_side_default);

	m_selected_poly = -1;
	m_current_tile_type = 0;
}

TileDesignerWidget::~TileDesignerWidget()
{
	delete m_poly[0];
	delete m_poly[1];

	for (int i = 0; i < 6; i++)
	{
		delete m_poly_top_default[i];		
	}
	delete m_poly_side_default;
}


QString TileDesignerWidget::loadShader(QString filename)
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

void TileDesignerWidget::loadTexture(QImage* texture)
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


void TileDesignerWidget::initializeGL()
{
	QString level_vs_file = loadShader("level_vs.glsl");
	QString level_fs_file = loadShader("level_fs.glsl");

	m_level_program = new QGLShaderProgram(this);
	m_level_program->addShaderFromSourceCode(QGLShader::Vertex, level_vs_file);
	m_level_program->addShaderFromSourceCode(QGLShader::Fragment, level_fs_file);
	m_level_program->link();

	QString error = m_level_program->log();
	std::string errors = error.toStdString();

	m_level_shader.position = m_level_program->attributeLocation("a_position");
	m_level_shader.tex_coord = m_level_program->attributeLocation("a_texcoord");
	m_level_shader.color = m_level_program->attributeLocation("a_color");
	m_level_shader.location = m_level_program->uniformLocation("v_location");
	m_level_shader.scale = m_level_program->uniformLocation("v_scale");
	m_level_shader.vp_matrix = m_level_program->uniformLocation("m_vp_matrix");
	m_level_shader.rot_matrix = m_level_program->uniformLocation("m_rot_matrix");

	m_vbo[0].pos = glm::vec3(0.0f, 0.0f, 0.0f);		m_vbo[0].uv = glm::vec2(0.0f, 0.0f);		m_vbo[0].color = 0xffffffff;
	m_vbo[1].pos = glm::vec3(0.0f, 1.0f, 0.0f);		m_vbo[1].uv = glm::vec2(0.0f, 1.0f);		m_vbo[1].color = 0xffffffff;
	m_vbo[2].pos = glm::vec3(1.0f, 1.0f, 0.0f);		m_vbo[2].uv = glm::vec2(1.0f, 1.0f);		m_vbo[2].color = 0xffffffff;
	m_vbo[3].pos = glm::vec3(1.0f, 0.0f, 0.0f);		m_vbo[3].uv = glm::vec2(1.0f, 0.0f);		m_vbo[3].color = 0xffffffff;
}

void TileDesignerWidget::paintGL()
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

	qglClearColor(m_bgcolor);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);

	float aspect = (float)(width()) / (float)(height());
	float halfh = 0.5f / m_zoom;
	float halfw = halfh * aspect;
	

	// setup matrices
	float level_vp_mat[4] = { 1.0f / halfw, 0.0f,
		0.0f, 1.0f / halfh };

	float level_rot_mat[4] = { 1.0f, 0.0f,
		0.0f, 1.0f };

	QMatrix2x2 vp_mat = QMatrix2x2(level_vp_mat);
	QMatrix2x2 rot_mat = QMatrix2x2(level_rot_mat);

	m_level_program->bind();

	m_level_program->setUniformValue(m_level_shader.vp_matrix, vp_mat);
	m_level_program->setUniformValue(m_level_shader.rot_matrix, rot_mat);
	m_level_program->setUniformValue(m_level_shader.location, m_scroll.x, m_scroll.y);
	m_level_program->setUniformValue(m_level_shader.scale, 1.0f, 1.0f);

	m_level_program->enableAttributeArray(m_level_shader.position);
	m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)m_vbo, 3, sizeof(VBO));
	m_level_program->enableAttributeArray(m_level_shader.tex_coord);
	m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)m_vbo + 3, 2, sizeof(VBO));
	m_level_program->enableAttributeArray(m_level_shader.color);
	m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)m_vbo + 20, 4, sizeof(VBO));

	glBindTexture(GL_TEXTURE_2D, m_base_tex);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	m_level_program->disableAttributeArray(m_level_shader.position);
	m_level_program->disableAttributeArray(m_level_shader.tex_coord);
	m_level_program->disableAttributeArray(m_level_shader.color);

	painter.endNativePainting();

	/*
	float w = (float)(width());
	float h = (float)(height());


	float area_texside = std::min(w, h) - 10;

	float area_xleft = 0.0f - ((w - area_texside) / 2.0f);
	float area_xright = w - ((w - area_texside) / 2.0f);
	float area_ytop = 0.0f - ((h - area_texside) / 2.0f);
	float area_ybottom = h - ((h - area_texside) / 2.0f);

	if (m_texture)
	{
		glm::vec2 tl = toScreenCoords(glm::vec2(0.0f, 0.0f));
		glm::vec2 br = toScreenCoords(glm::vec2(1.0f, 1.0f));
		painter.drawImage(QRect(tl.x, tl.y, br.x - tl.x, br.y - tl.y), *m_texture);
	}
	*/

	if (m_show_grid)
		drawGrid(painter);

	/*
	if (!m_poly_closed && (m_mode == MODE_DRAW_POLY || m_mode == MODE_DRAW_RECT))
	{
		if (m_mode == MODE_DRAW_POLY)
			renderDrawPolyMode(painter);
		else if (m_mode == MODE_DRAW_RECT)
			renderDrawRectMode(painter);
	}
	else
	{
		renderClosedPoly(painter);
	}
	*/

	for (int i = 0; i < 2; i++)
	{
		renderPoly(painter, i);
	}

	painter.end();

	doneCurrent();

}

void TileDesignerWidget::resizeGL(int width, int height)
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

void TileDesignerWidget::setTexture(QImage* texture)
{
	m_texture = texture;

	loadTexture(m_texture);

	update();
}

void TileDesignerWidget::setMode(OperationMode mode)
{
	m_mode = mode;

	switch (mode)
	{
	}

	update();
}

// convert uv coords to screen coordinates
glm::vec2 TileDesignerWidget::toScreenCoords(glm::vec2& point)
{
	float scale = m_zoom;

	float ww = (float)width();
	float hh = (float)height();
	float side = std::min(ww, hh);

	float mult = (float)(side);

	float x = (point.x + m_scroll.x) * mult * scale;
	float y = (point.y + m_scroll.y) * mult * scale;

	return glm::vec2(x, y);
}

// convert screen coordinates to uv coords
glm::vec2 TileDesignerWidget::toUVCoords(glm::vec2& point)
{
	float scale = 1.0f / m_zoom;

	float ww = (float)width();
	float hh = (float)height();
	float side = std::min(ww, hh);

	float x = point.x * scale;
	float y = point.y * scale;

	float mult = 1.0f / side;

	x = (x * mult) - m_scroll.x;
	y = (y * mult) - m_scroll.y;

	return glm::vec2(x, y);
}

glm::vec2 TileDesignerWidget::snapToGrid(glm::vec2& point)
{
	float grid_size = TileDesigner::GRID_SIZE[m_grid_size];

	float tx = point.x / grid_size;
	float ty = point.y / grid_size;
	
	float x, y;
	float xfrac = modf(tx, &x);
	float yfrac = modf(ty, &y);
	if (xfrac >= 0.5f) x++;
	if (yfrac >= 0.5f) y++;

	x *= grid_size;
	y *= grid_size;

	return glm::vec2(x, y);
}

void TileDesignerWidget::setScale(int scale)
{
}

void TileDesignerWidget::setZoom(int zoom)
{
	assert(zoom >= 0 && zoom < TileDesigner::NUM_ZOOM_LEVELS);

	m_zoom  = TileDesigner::ZOOM_LEVELS[zoom];

	m_scroll = glm::vec2(0.0f, 0.0f);

	update();
}

void TileDesignerWidget::setGrid(int grid)
{
	assert(grid >= 0 && grid < TileDesigner::NUM_GRID_SIZES);

	m_grid_size = grid;

	update();
}

void TileDesignerWidget::enableShowGrid(bool enable)
{
	m_show_grid = enable;
	update();
}

void TileDesignerWidget::enableSnapGrid(bool enable)
{
	m_snap_grid = enable;
}

void TileDesignerWidget::setTileType(int type)
{
	m_poly[0]->copy(m_poly_top_default[type]);
	m_poly[1]->copy(m_poly_side_default);

	m_current_tile_type = type;

	m_selected_poly = -1;

	update();
}

void TileDesignerWidget::resetObject(int objects)
{
	if (objects & POLY_TOP)
	{
		m_poly[0]->copy(m_poly_top_default[m_current_tile_type]);
	}
	if (objects & POLY_SIDE)
	{
		m_poly[1]->copy(m_poly_side_default);
	}

	m_selected_poly = -1;

	update();
}

void TileDesignerWidget::insertTile(QString& name)
{
	/*
	glm::vec2 npoints[4];
	for (int i=0; i < 4; i++)
	{
		glm::vec2 p = m_polydef->getPoint(i);
		npoints[i] = p;
	}

	int id = m_level->insertTile(name.toStdString(), npoints, m_color, Tilemap::TILE_FULL);
	emit onInsertTile(id);

	m_polydef->reset();
	m_poly_closed = false;
	*/
	update();
}

void TileDesignerWidget::mouseReleaseEvent(QMouseEvent* event)
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

	if (event->button() & Qt::LeftButton)
	{
		switch (m_mode)
		{
			case MODE_MOVE:
			{
				if (m_selected_poly >= 0)
				{
					glm::vec2 p = toUVCoords(glm::vec2(mouse_x, mouse_y));
					if (m_snap_grid)
						p = snapToGrid(p);

					glm::vec2 delta = p - m_move_reference;

					int num_points = m_poly[m_selected_poly]->getNumPoints();
					for (int i = 0; i < num_points; i++)
					{
						glm::vec2 v = m_poly[m_selected_poly]->getPoint(i);
						m_poly[m_selected_poly]->edit(i, v + delta);
					}
				}

				m_selected_poly = -1;

				m_move_dragging = false;
				break;
			}
		}
	}

	update();
}

void TileDesignerWidget::mousePressEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	if (event->button() & Qt::RightButton)
	{
		if (!m_panning)
		{
			m_pan_point = toUVCoords(glm::vec2(mouse_x, mouse_y));
			m_scroll_saved = m_scroll;
			m_panning = true;
		}
	}

	if (event->button() & Qt::LeftButton)
	{
		switch (m_mode)
		{
			case MODE_MOVE:
			{
				if (!m_move_dragging)
				{
					glm::vec2 mp = toUVCoords(glm::vec2(mouse_x, mouse_y));
					if (m_snap_grid)
						mp = snapToGrid(mp);

					for (int i = 0; i < 2; i++)
					{
						bool inside = m_poly[i]->isPointInside(mp);
						if (inside)
						{
							m_move_reference = mp;
							m_move_dragging = true;
							m_selected_poly = i;
							break;
						}
					}
				}
				
				break;
			}
		}
	}

	update();
}

void TileDesignerWidget::mouseMoveEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	if (m_panning)
	{
		glm::vec2 mouse_p = toUVCoords(glm::vec2(mouse_x, mouse_y));
		glm::vec2 delta = (m_pan_point - mouse_p) * 0.4f;
		m_scroll = m_scroll_saved - delta;
	}

	update();
}

void TileDesignerWidget::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();

	switch (m_mode)
	{
		/*
		case MODE_DRAW_POLY:
		{
			if (key == Qt::Key_Escape)
			{
				if (!m_poly_closed)
				{
					m_polydef->reset();
					m_poly_closed = false;
				}
			}
			else if (key == Qt::Key_Backspace)
			{
				if (!m_poly_closed)
				{
					m_polydef->deleteLatest();
				}
			}
			else if (key == Qt::Key_Space)
			{
				int num_points = m_polydef->getNumPoints();
				if (num_points >= 3 && m_polydef->fullConvexTest())
				{
					m_poly_closed = true;
				}
			}
			break;
		}

		case MODE_EDIT_VERTICES:
		{
			if (key == Qt::Key_Escape || key == Qt::Key_Backspace)
			{
				m_selected_point = -1;
			}
			break;
		}

		case MODE_EDIT_EDGES:
		{			
			if (key == Qt::Key_Escape || key == Qt::Key_Backspace)
			{
				m_line_dragging = false;
				m_line_point0 = -1;
				m_line_point1 = -1;
			}		
			break;
		}
		*/
	}

	update();
}

/*
void TileDesignerWidget::renderDrawPolyMode(QPainter& painter)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());

	glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

	bool valid_convex = m_polydef->convexTest(mouse_lp);

	QColor fillcolor;
	QColor linecolor;
	if (!m_poly_closed)
	{
		if (valid_convex)
		{
			fillcolor = m_validpoly_color;
			linecolor = m_validline_color;
		}
		else
		{
			fillcolor = m_errorpoly_color;
			linecolor = m_errorline_color;
		}
	}
	else
	{
		fillcolor = m_closedpoly_color;
		linecolor = m_closedline_color;
	}

	int num_points = m_polydef->getNumPoints();

	if (num_points >= 2)
	{
		// polygon
		QPointF ppoints[9];
		for (int i=0; i < num_points; i++)
		{
			glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));
			ppoints[i].setX(p.x);
			ppoints[i].setY(p.y);
		}

		glm::vec2 endp = mouse_lp;

		if (m_snap_grid)
			endp = snapToGrid(endp);

		endp = toScreenCoords(endp);

		ppoints[num_points].setX(endp.x);
		ppoints[num_points].setY(endp.y);

		painter.setPen(linecolor);
		painter.setBrush(QBrush(fillcolor));

		if (!m_poly_closed)
			painter.drawPolygon(ppoints, num_points+1);
		else
			painter.drawPolygon(ppoints, num_points);
	}

	// points
	for (int i=0; i < num_points; i++)
	{
		glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));

		painter.fillRect(p.x-3, p.y-2, 5, 5, m_point_color);
	}

	// draw line to mouse pointer if not at max
	if (num_points > 0 && num_points < m_polydef->getCapacity() && !m_poly_closed)
	{
		glm::vec2 p = toScreenCoords(m_polydef->getPoint(num_points-1));
		glm::vec2 endp = mouse_lp;

		if (m_snap_grid)
			endp = snapToGrid(endp);

		endp = toScreenCoords(endp);
		
		painter.setPen(linecolor);
		painter.drawLine(QPoint(p.x, p.y), QPoint(endp.x, endp.y));
	}
}

void TileDesignerWidget::renderDrawRectMode(QPainter& painter)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());
	glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

	if (m_snap_grid)
		mouse_lp = snapToGrid(mouse_lp);

	if (m_rect_drawing)
	{
		glm::vec2 start_p = toScreenCoords(m_rect_start_point);
		glm::vec2 end_p = toScreenCoords(mouse_lp);

		painter.setPen(m_validline_color);
		painter.setBrush(QBrush(m_validpoly_color));
		painter.drawRect(QRect(start_p.x, start_p.y, end_p.x - start_p.x, end_p.y - start_p.y));

		// points
		painter.fillRect(start_p.x-3, start_p.y-2, 5, 5, m_point_color);
		painter.fillRect(start_p.x-3, end_p.y-2, 5, 5, m_point_color);
		painter.fillRect(end_p.x-3, end_p.y-2, 5, 5, m_point_color);
		painter.fillRect(end_p.x-3, start_p.y-2, 5, 5, m_point_color);
	}
}
*/

void TileDesignerWidget::renderPoly(QPainter& painter, int polynum)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());
	glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

	if (m_snap_grid)
		mouse_lp = snapToGrid(mouse_lp);

	int num_points = m_poly[polynum]->getNumPoints();

	if (num_points >= 3)
	{
		QPointF ppoints[8];
		for (int i=0; i < num_points; i++)
		{
			glm::vec2 p = toScreenCoords(m_poly[polynum]->getPoint(i));
			
			if (m_move_dragging && polynum == m_selected_poly)
			{
				glm::vec2 mouse_delta = toScreenCoords(m_move_reference) - glm::vec2(toScreenCoords(mouse_lp));
				p -= mouse_delta;
			}
			
			ppoints[i].setX(p.x);
			ppoints[i].setY(p.y);
		}

		if (polynum == m_selected_poly)
		{
			painter.setPen(m_validline_color);
			painter.setBrush(QBrush(m_validpoly_color));
		}
		else
		{
			painter.setPen(m_closedline_color);
			painter.setBrush(QBrush(m_closedpoly_color));
		}

		painter.drawPolygon(ppoints, num_points);


		// points
		for (int i=0; i < num_points; i++)
		{
			QColor color = m_point_color;

			painter.fillRect(ppoints[i].x()-3, ppoints[i].y()-2, 5, 5, color);
		}
	}
}

void TileDesignerWidget::drawGrid(QPainter& painter)
{
	int w = width();
	int h = height();

	float grid_size = TileDesigner::GRID_SIZE[m_grid_size];	

	glm::vec2 tl = toUVCoords(glm::vec2(0, 0));
	glm::vec2 br = toUVCoords(glm::vec2(w, h));

	glm::vec2 sg1 = toScreenCoords(glm::vec2(grid_size, grid_size));
	glm::vec2 sg2 = toScreenCoords(glm::vec2(0.0f, 0.0f));
	glm::vec2 sg = sg1 - sg2;

	int num_x = (int)((float)(w) / (floor)(sg.x)) + 1;
	int num_y = (int)((float)(h) / (floor)(sg.x)) + 1;

	
	glm::vec2 orig = toScreenCoords(snapToGrid(tl));

	float x = orig.x;
	float y = orig.y;

	painter.setPen(QColor(144, 144, 144, 255));

	for (int i=0; i < num_x; i++)
	{
		painter.drawLine(x, 0, x, h);
		x += sg.x;
	}

	for (int i=0; i < num_y; i++)
	{
		painter.drawLine(0, y, w, y);
		y += sg.x;
	}

	/*
	QString text = tr("h %1 sg %2 num %3").arg(h).arg(sg.x).arg(num_y);
	painter.setPen(QColor(255, 0, 0));
	painter.drawText(8, 16, text);
	*/
}


void TileDesignerWidget::paintEvent(QPaintEvent* event)
{
	paintGL();
}

void TileDesignerWidget::setColor(QColor color)
{
	m_object_color = color;

	unsigned cc = 0xff000000 | (color.red() << 0) | (color.green() << 8) | (color.blue() << 16);

	m_vbo[0].color = cc;
	m_vbo[1].color = cc;
	m_vbo[2].color = cc;
	m_vbo[3].color = cc;
}







const float TileDesigner::GRID_SIZE[TileDesigner::NUM_GRID_SIZES] = 
{
	0.00390625f,
	0.0078125f,
	0.015625f,
	0.03125f,
	0.0625f,
	0.125f,
	0.250f,
	0.5f
};

const float TileDesigner::ZOOM_LEVELS[TileDesigner::NUM_ZOOM_LEVELS] =
{
	0.5f,
	1.0f,
	2.0f,
	3.0f,
};

TileDesigner::TileDesigner(QWidget* parent, Level* level) : QDockWidget("Tile Designer", parent, 0)
{
	m_level = level;

	m_widget = new TileDesignerWidget(parent, m_level);
	m_window = new QMainWindow(0);

	setFocusPolicy(Qt::ClickFocus);

	connect(m_widget, SIGNAL(onInsertTile(int)), this, SIGNAL(onInsertTile(int)));


	// edit tools
	m_toolgroup = new QActionGroup(this);

	m_move_action = new QAction(QIcon("move.png"), tr("Move"), this);
	m_move_action->setCheckable(true);
	connect(m_move_action, SIGNAL(triggered()), this, SLOT(setMoveMode()));

	m_rotate_action = new QAction(QIcon("rotate.png"), tr("Rotate"), this);
	m_rotate_action->setCheckable(true);
	connect(m_rotate_action, SIGNAL(triggered()), this, SLOT(setRotateMode()));

	m_scale_action = new QAction(QIcon("scale.png"), tr("Scale"), this);
	m_scale_action->setCheckable(true);
	connect(m_scale_action, SIGNAL(triggered()), this, SLOT(setScaleMode()));
		
	m_toolgroup->addAction(m_move_action);
	m_toolgroup->addAction(m_rotate_action);
	m_toolgroup->addAction(m_scale_action);


	// tile tools
	m_tilegroup = new QActionGroup(this);

	m_tiletype_combo = new QComboBox();
	m_tiletype_combo->setWindowTitle("Tile type");
	m_tiletype_combo->addItem("Full", QVariant(0));
	m_tiletype_combo->addItem("Left", QVariant(1));
	m_tiletype_combo->addItem("Right", QVariant(2));
	m_tiletype_combo->addItem("Top", QVariant(3));
	m_tiletype_combo->addItem("Bottom", QVariant(4));
	m_tiletype_combo->addItem("Middle", QVariant(5));
	m_tiletype_combo->setFocusPolicy(Qt::NoFocus);
	connect(m_tiletype_combo, SIGNAL(currentIndexChanged(int)), m_widget, SLOT(setTileType(int)));

	m_tiletype_label = new QLabel("Tile type:");
	m_tiletype_label->setMinimumWidth(50);
	QBoxLayout* tiletype_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	tiletype_layout->setSpacing(2);
	tiletype_layout->setMargin(1);
	tiletype_layout->addWidget(m_tiletype_label);
	tiletype_layout->addWidget(m_tiletype_combo);
	m_tiletype_widget = new QWidget;
	m_tiletype_widget->setMaximumHeight(30);
	m_tiletype_widget->setLayout(tiletype_layout);


	// zoom tools
	m_zoom_box = new QComboBox();
	m_zoom_box->setWindowTitle("Zoom");
	for (int i=0; i < TileDesigner::NUM_ZOOM_LEVELS; i++)
	{
		m_zoom_box->addItem(tr("%1%").arg((int)(TileDesigner::ZOOM_LEVELS[i] * 100.0f)), QVariant(i + 1));
	}
	m_zoom_box->setFocusPolicy(Qt::NoFocus);

	m_zoomlevel_label = new QLabel("Zoom:");
	m_zoomlevel_label->setMinimumWidth(30);
	QBoxLayout* zoomlevel_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	zoomlevel_layout->setSpacing(2);
	zoomlevel_layout->setMargin(1);
	zoomlevel_layout->addWidget(m_zoomlevel_label);
	zoomlevel_layout->addWidget(m_zoom_box);
	m_zoomlevel_widget = new QWidget;
	m_zoomlevel_widget->setMaximumHeight(30);
	m_zoomlevel_widget->setLayout(zoomlevel_layout);


	// grid tools
	m_togglegrid_action = new QAction(QIcon("grid.png"), tr("Enable Grid"), this);
	m_togglegrid_action->setCheckable(true);
	connect(m_togglegrid_action, SIGNAL(triggered()), this, SLOT(toggleGrid()));

	m_snapgrid_action = new QAction(QIcon("snapgrid.png"), tr("Snap to grid"), this);
	m_snapgrid_action->setCheckable(true);
	connect(m_snapgrid_action, SIGNAL(triggered()), this, SLOT(snapGrid()));

	m_gridsize_combo = new QComboBox();
	m_gridsize_combo->setWindowTitle("Grid Size");
	for (int i=0; i < TileDesigner::NUM_GRID_SIZES; i++)
	{
		m_gridsize_combo->addItem(tr("%1").arg(TileDesigner::GRID_SIZE[i], 0, 'f', 2));
	}
	connect(m_gridsize_combo, SIGNAL(activated(int)), this, SLOT(setGridSize(int)));
	m_gridsize_label = new QLabel("Grid:");
	m_gridsize_label->setMinimumWidth(30);
	QBoxLayout* gridsize_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	gridsize_layout->setSpacing(2);
	gridsize_layout->setMargin(1);
	gridsize_layout->addWidget(m_gridsize_label);
	gridsize_layout->addWidget(m_gridsize_combo);
	m_gridsize_widget = new QWidget;
	m_gridsize_widget->setMaximumHeight(30);
	m_gridsize_widget->setLayout(gridsize_layout);


	// control tools
	m_reset_button = new QPushButton(tr("Reset"), this);
	m_reset_button->setFocusPolicy(Qt::NoFocus);
	connect(m_reset_button, SIGNAL(clicked()), this, SLOT(reset()));

	m_inserttile_button = new QPushButton(tr("Insert Tile"), this);
	m_inserttile_button->setFocusPolicy(Qt::NoFocus);
	connect(m_inserttile_button, SIGNAL(clicked()), this, SLOT(insertTile()));

	m_color_button = new QPushButton("", this);
	m_color_button->setFocusPolicy(Qt::NoFocus);
	m_color_button->setStyleSheet(tr("background-color: #%1%2%3").arg(m_object_color.red(), 2, 16, QChar('0')).arg(m_object_color.green(), 2, 16, QChar('0')).arg(m_object_color.blue(), 2, 16, QChar('0')));
	m_color_button->setMaximumHeight(25);
	m_color_button->setMaximumWidth(25);
	connect(m_color_button, SIGNAL(clicked()), this, SLOT(chooseColor()));
	m_color_label = new QLabel("Color:");
	QBoxLayout* color_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	color_layout->setSpacing(2);
	color_layout->setMargin(1);
	color_layout->addWidget(m_color_label);
	color_layout->addWidget(m_color_button);

	m_color_widget = new QWidget;
	m_color_widget->setMaximumHeight(30);
	m_color_widget->setLayout(color_layout);




	// add toolbar
	m_edit_toolbar = new QToolBar(m_window);
	m_edit_toolbar->addAction(m_move_action);
	m_edit_toolbar->addAction(m_rotate_action);
	m_edit_toolbar->addAction(m_scale_action);
	m_window->addToolBar(m_edit_toolbar);

	m_tile_toolbar = new QToolBar(m_window);
	m_tile_toolbar->addWidget(m_tiletype_widget);
	m_window->addToolBar(m_tile_toolbar);

	m_zoom_toolbar = new QToolBar(m_window);
	m_zoom_toolbar->addWidget(m_zoomlevel_widget);
	m_window->addToolBar(m_zoom_toolbar);

	m_grid_toolbar = new QToolBar(m_window);
	m_grid_toolbar->addAction(m_togglegrid_action);
	m_grid_toolbar->addAction(m_snapgrid_action);
	m_grid_toolbar->addWidget(m_gridsize_widget);
	m_window->addToolBar(m_grid_toolbar);

	m_color_toolbar = new QToolBar(m_window);
	m_color_toolbar->addWidget(m_color_widget);
	m_window->addToolBar(m_color_toolbar);

	m_control_toolbar = new QToolBar(m_window);
	m_control_toolbar->addWidget(m_reset_button);
	m_control_toolbar->addWidget(m_inserttile_button);
	m_window->addToolBar(m_control_toolbar);

	connect(m_zoom_box, SIGNAL(currentIndexChanged(int)), m_widget, SLOT(setZoom(int)));

	m_window->setParent(this);
	setWidget(m_window);
	m_window->setCentralWidget(m_widget);
	m_window->setContextMenuPolicy(Qt::NoContextMenu);
	
	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	this->setMinimumWidth(1400);
	this->setMinimumHeight(1000);
	this->setMaximumWidth(1800);
	this->setMaximumHeight(1500);

	// don't allow docking
	this->setAllowedAreas(0);

	this->setHidden(false);

	// default zoom = 100%
	m_zoom_box->setCurrentIndex(1);

	// move mode enable initially
	m_move_action->setChecked(true);
	m_widget->setMode(TileDesignerWidget::MODE_MOVE);


	m_gridsize_combo->setCurrentIndex(3);
	m_widget->setGrid(3);
}

TileDesigner::~TileDesigner()
{
}

void TileDesigner::setMoveMode()
{
	m_widget->setMode(TileDesignerWidget::MODE_MOVE);
}

void TileDesigner::reset()
{
	m_widget->resetObject(TileDesignerWidget::POLY_TOP | TileDesignerWidget::POLY_SIDE);
}

void TileDesigner::insertTile()
{
	bool ok;
	QString name = QInputDialog::getText(this, tr("Enter tile name"), tr("Name:"), QLineEdit::Normal, "", &ok);

	if (ok && !name.isEmpty())
	{
		m_widget->insertTile(name);
	}
}



void TileDesigner::closeEvent(QCloseEvent* event)
{
	emit onClose();
}

void TileDesigner::setTexture(QImage* texture)
{
	m_widget->setTexture(texture);
}


void TileDesigner::toggleGrid()
{
	if (m_togglegrid_action->isChecked())
		emit m_widget->enableShowGrid(true);
	else
		emit m_widget->enableShowGrid(false);
}

void TileDesigner::snapGrid()
{
	if (m_snapgrid_action->isChecked())
		emit m_widget->enableSnapGrid(true);
	else
		emit m_widget->enableSnapGrid(false);
}

void TileDesigner::setGridSize(int size)
{
	emit m_widget->setGrid(size);
}

void TileDesigner::chooseColor()
{
	QColor result = QColorDialog::getColor(m_object_color, this, tr("Select object color"));
	if (result.isValid())
	{
		emit m_widget->setColor(result);
		m_color_button->setStyleSheet(tr("background-color: #%1%2%3").arg(result.red(), 2, 16, QChar('0')).arg(result.green(), 2, 16, QChar('0')).arg(result.blue(), 2, 16, QChar('0')));

		m_object_color = result;
	}
}