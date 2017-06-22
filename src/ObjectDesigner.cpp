#include "ObjectDesigner.h"

ObjectDesignerWidget::ObjectDesignerWidget(QWidget* parent, Level* level) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_level = level;

	setFocusPolicy(Qt::ClickFocus);

	m_polydef = new PolygonDef(8);
	m_polydef->reset();

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

	m_poly_closed = false;
	m_rect_drawing = false;

	m_selected_point = -1;

	m_line_dragging = false;
	m_line_point0 = -1;
	m_line_point1 = -1;

	m_move_dragging = false;

	m_show_grid = false;
	m_snap_grid = false;

	m_object_color = QColor(0, 0, 0, 255);
}

ObjectDesignerWidget::~ObjectDesignerWidget()
{
	delete m_polydef;
}


QString ObjectDesignerWidget::loadShader(QString filename)
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

void ObjectDesignerWidget::loadTexture(QImage* texture)
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


void ObjectDesignerWidget::initializeGL()
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

void ObjectDesignerWidget::paintGL()
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

	painter.end();

	doneCurrent();

}

void ObjectDesignerWidget::resizeGL(int width, int height)
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

void ObjectDesignerWidget::setTexture(QImage* texture)
{
	m_texture = texture;

	loadTexture(m_texture);

	update();
}

void ObjectDesignerWidget::setMode(OperationMode mode)
{
	m_mode = mode;

	switch (mode)
	{
		case MODE_DRAW_POLY:
			break;

		case MODE_DRAW_RECT:
			break;

		case MODE_EDIT_VERTICES:
			break;

		case MODE_EDIT_EDGES:
			m_line_dragging = false;
			break;
	}

	// always reset edited point
	m_selected_point = -1;

	update();
}

// convert uv coords to screen coordinates
glm::vec2 ObjectDesignerWidget::toScreenCoords(glm::vec2& point)
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
glm::vec2 ObjectDesignerWidget::toUVCoords(glm::vec2& point)
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

glm::vec2 ObjectDesignerWidget::snapToGrid(glm::vec2& point)
{
	float grid_size = ObjectDesigner::GRID_SIZE[m_grid_size];

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

void ObjectDesignerWidget::setScale(int scale)
{
}

void ObjectDesignerWidget::setZoom(int zoom)
{
	assert(zoom >= 0 && zoom < ObjectDesigner::NUM_ZOOM_LEVELS);

	m_zoom  = ObjectDesigner::ZOOM_LEVELS[zoom];

	m_scroll = glm::vec2(0.0f, 0.0f);

	update();
}

void ObjectDesignerWidget::setGrid(int grid)
{
	assert(grid >= 0 && grid < ObjectDesigner::NUM_GRID_SIZES);

	m_grid_size = grid;

	update();
}

void ObjectDesignerWidget::enableShowGrid(bool enable)
{
	m_show_grid = enable;
	update();
}

void ObjectDesignerWidget::enableSnapGrid(bool enable)
{
	m_snap_grid = enable;
}

void ObjectDesignerWidget::resetObject()
{
	m_polydef->reset();
	m_poly_closed = false;

	update();
}

void ObjectDesignerWidget::insertTile(QString& name)
{
	if (m_poly_closed)
	{
		glm::vec2 npoints[4];
		for (int i=0; i < 4; i++)
		{
			glm::vec2 p = m_polydef->getPoint(i);
			npoints[i] = p;
		}

		int id = m_level->insertTile(name.toStdString(), npoints, m_color);
		emit onInsertTile(id);

		m_polydef->reset();
		m_poly_closed = false;
	}

	update();
}

void ObjectDesignerWidget::mouseReleaseEvent(QMouseEvent* event)
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
			case MODE_DRAW_POLY:
			{
				glm::vec2 p = toUVCoords(glm::vec2(mouse_x, mouse_y));

				if (m_snap_grid)
					p = snapToGrid(p);

				if (!m_poly_closed)
				{
					m_polydef->insertPoint(p);
				}
				break;
			}

			case MODE_DRAW_RECT:
			{
				if (m_rect_drawing && !m_poly_closed)
				{
					glm::vec2 sp = m_rect_start_point;
					glm::vec2 ep = toUVCoords(glm::vec2(mouse_x, mouse_y));

					if (m_snap_grid)
						ep = snapToGrid(ep);

					glm::vec2 delta = ep - sp;

					if (delta.x != 0.0f && delta.y != 0.0f)
					{
						m_polydef->reset();

						if (delta.x < 0.0f && delta.y < 0.0f)		// end point = top left
						{
							m_polydef->insertPoint(glm::vec2(ep.x, ep.y));
							m_polydef->insertPoint(glm::vec2(ep.x, sp.y));
							m_polydef->insertPoint(glm::vec2(sp.x, sp.y));
							m_polydef->insertPoint(glm::vec2(sp.x, ep.y));
						}
						else if (delta.x > 0.0f && delta.y < 0.0f)	// end point = top right
						{
							m_polydef->insertPoint(glm::vec2(sp.x, ep.y));
							m_polydef->insertPoint(glm::vec2(sp.x, sp.y));
							m_polydef->insertPoint(glm::vec2(ep.x, sp.y));
							m_polydef->insertPoint(glm::vec2(ep.x, ep.y));
						}
						else if (delta.x < 0.0f && delta.y > 0.0f)	// end point = bottom left
						{
							m_polydef->insertPoint(glm::vec2(ep.x, sp.y));
							m_polydef->insertPoint(glm::vec2(ep.x, ep.y));
							m_polydef->insertPoint(glm::vec2(sp.x, ep.y));
							m_polydef->insertPoint(glm::vec2(sp.x, sp.y));
						}
						else if (delta.x > 0.0f && delta.y > 0.0f)	// end point = bottom right
						{
							m_polydef->insertPoint(glm::vec2(sp.x, sp.y));
							m_polydef->insertPoint(glm::vec2(sp.x, ep.y));
							m_polydef->insertPoint(glm::vec2(ep.x, ep.y));
							m_polydef->insertPoint(glm::vec2(ep.x, sp.y));
						}

						m_poly_closed = true;
					}

					m_rect_drawing = false;
				}
				break;
			}

			case MODE_MOVE:
			{
				glm::vec2 p = toUVCoords(glm::vec2(mouse_x, mouse_y));
				if (m_snap_grid)
					p = snapToGrid(p);

				glm::vec2 delta = p - m_move_reference;

				int num_points = m_polydef->getNumPoints();
				for (int i=0; i < num_points; i++)
				{
					glm::vec2 v = m_polydef->getPoint(i);
					m_polydef->edit(i, v + delta);
				}

				m_move_dragging = false;
				break;
			}

			case MODE_EDIT_VERTICES:
			{
				if (m_selected_point >= 0)
				{
					// validate and finalize point edit
					glm::vec2 old_p = m_polydef->getPoint(m_selected_point);

					glm::vec2 p = toUVCoords(glm::vec2(mouse_x, mouse_y));
					if (m_snap_grid)
						p = snapToGrid(p);
					
					m_polydef->edit(m_selected_point, p);

					bool valid = m_polydef->fullConvexTest();
				
					if (!valid)
					{
						// restore old point
						m_polydef->edit(m_selected_point, old_p);
					}

					m_selected_point = -1;
				}
				break;
			}

			case MODE_EDIT_EDGES:
			{
				if (m_line_dragging)
				{
					glm::vec2 p = toUVCoords(glm::vec2(mouse_x, mouse_y));
					if (m_snap_grid)
						p = snapToGrid(p);

					glm::vec2 delta = p - m_move_reference;

					glm::vec2 old_p0 = m_polydef->getPoint(m_line_point0);
					glm::vec2 old_p1 = m_polydef->getPoint(m_line_point1);

					m_polydef->edit(m_line_point0, old_p0 + delta);
					m_polydef->edit(m_line_point1, old_p1 + delta);

					bool valid = m_polydef->fullConvexTest();
					if (!valid)
					{
						// restore old points
						m_polydef->edit(m_line_point0, old_p0);
						m_polydef->edit(m_line_point1, old_p1);
					}

					m_line_dragging = false;
					m_line_point0 = -1;
					m_line_point1 = -1;
				}
				break;
			}
		}
	}

	update();
}

void ObjectDesignerWidget::mousePressEvent(QMouseEvent* event)
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
			case MODE_DRAW_RECT:
			{
				if (!m_rect_drawing && !m_poly_closed)
				{
					m_rect_start_point = toUVCoords(glm::vec2(mouse_x, mouse_y));

					if (m_snap_grid)
						m_rect_start_point = snapToGrid(m_rect_start_point);

					m_rect_drawing = true;
				}
				break;
			}

			case MODE_MOVE:
			{
				if (!m_move_dragging)
				{
					glm::vec2 mp = toUVCoords(glm::vec2(mouse_x, mouse_y));
					if (m_snap_grid)
						mp = snapToGrid(mp);

					bool inside = m_polydef->isPointInside(mp);
					if (inside)
					{
						m_move_reference = mp;
						m_move_dragging = true;
					}
				}
				break;
			}

			case MODE_EDIT_VERTICES:
			{
				if (m_poly_closed)
				{
					int num_points = m_polydef->getNumPoints();
	
					// find possible selected point
					m_selected_point = -1;
					for (int i=0; i < num_points; i++)
					{
						const glm::vec2& pp = toScreenCoords(m_polydef->getPoint(i));
						
						if (mouse_x >= (pp.x - POINT_CLICKING_THRESHOLD) &&
							mouse_x <= (pp.x + POINT_CLICKING_THRESHOLD) &&
							mouse_y >= (pp.y - POINT_CLICKING_THRESHOLD) &&
							mouse_y <= (pp.y + POINT_CLICKING_THRESHOLD))
						{
							m_selected_point = i;
							break;
						}
					}
				}
				break;
			}

			case MODE_EDIT_EDGES:
			{
				if (!m_line_dragging)
				{
					glm::vec2 mp = toUVCoords(glm::vec2(mouse_x, mouse_y));
					int num_points = m_polydef->getNumPoints();

					float threshold = 0.0f;

					// very ugly way to calculate threshold....
					if (num_points > 0)
					{
						glm::vec2 p1 = m_polydef->getPoint(0);
						const glm::vec2& pp = toScreenCoords(p1);
						glm::vec2 p2 = toUVCoords(glm::vec2(pp.x + POINT_CLICKING_THRESHOLD, pp.y + POINT_CLICKING_THRESHOLD));
						threshold = p2.x - p1.x;
					}

					for (int i=0; i < num_points; i++)
					{
						int point0 = -1;
						int point1 = -1;

						if (i == 0)
						{
							point0 = num_points - 1;
							point1 = 0;
						}
						else
						{
							point0 = i - 1;
							point1 = i;							
						}											

						if (m_polydef->isPointOnEdge(mp, point0, point1, threshold))
						{
							m_line_dragging = true;
							m_line_point0 = point0;
							m_line_point1 = point1;
							m_move_reference = mp;
							if (m_snap_grid)
								m_move_reference = snapToGrid(m_move_reference);
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

void ObjectDesignerWidget::mouseMoveEvent(QMouseEvent* event)
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

void ObjectDesignerWidget::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();

	switch (m_mode)
	{
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
	}

	update();
}


void ObjectDesignerWidget::renderDrawPolyMode(QPainter& painter)
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

void ObjectDesignerWidget::renderDrawRectMode(QPainter& painter)
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

void ObjectDesignerWidget::renderClosedPoly(QPainter& painter)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());
	glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

	if (m_snap_grid)
		mouse_lp = snapToGrid(mouse_lp);

	int num_points = m_polydef->getNumPoints();

	if (num_points >= 3)
	{
		QPointF ppoints[8];
		for (int i=0; i < num_points; i++)
		{
			glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));
			
			if (m_move_dragging)
			{
				glm::vec2 mouse_delta = toScreenCoords(m_move_reference) - glm::vec2(toScreenCoords(mouse_lp));
				p -= mouse_delta;
			}
			
			ppoints[i].setX(p.x);
			ppoints[i].setY(p.y);
		}

		if (m_selected_point >= 0)
		{
			glm::vec2 p = toScreenCoords(mouse_lp);
			ppoints[m_selected_point].setX(p.x);
			ppoints[m_selected_point].setY(p.y);
		}

		if (m_line_dragging)
		{
			glm::vec2 mouse_delta = toScreenCoords(m_move_reference) - glm::vec2(toScreenCoords(mouse_lp));

			ppoints[m_line_point0].setX(ppoints[m_line_point0].x() - mouse_delta.x);
			ppoints[m_line_point0].setY(ppoints[m_line_point0].y() - mouse_delta.y);
			
			ppoints[m_line_point1].setX(ppoints[m_line_point1].x() - mouse_delta.x);
			ppoints[m_line_point1].setY(ppoints[m_line_point1].y() - mouse_delta.y);
		}

		painter.setPen(m_closedline_color);
		painter.setBrush(QBrush(m_closedpoly_color));

		painter.drawPolygon(ppoints, num_points);


		// points
		for (int i=0; i < num_points; i++)
		{
			QColor color = m_point_color;

			if (i == m_selected_point)
			{
				color = m_errorline_color;
			}

			painter.fillRect(ppoints[i].x()-3, ppoints[i].y()-2, 5, 5, color);
		}
	}
}

void ObjectDesignerWidget::drawGrid(QPainter& painter)
{
	int w = width();
	int h = height();

	float grid_size = ObjectDesigner::GRID_SIZE[m_grid_size];	

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


void ObjectDesignerWidget::paintEvent(QPaintEvent* event)
{
	paintGL();
}

int ObjectDesignerWidget::numPoints()
{
	return m_polydef->getNumPoints();
}

void ObjectDesignerWidget::setColor(QColor color)
{
	m_object_color = color;

	unsigned cc = 0xff000000 | (color.red() << 0) | (color.green() << 8) | (color.blue() << 16);

	m_vbo[0].color = cc;
	m_vbo[1].color = cc;
	m_vbo[2].color = cc;
	m_vbo[3].color = cc;
}







const float ObjectDesigner::GRID_SIZE[ObjectDesigner::NUM_GRID_SIZES] = 
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

const float ObjectDesigner::ZOOM_LEVELS[ObjectDesigner::NUM_ZOOM_LEVELS] =
{
	0.5f,
	1.0f,
	2.0f,
	3.0f,
};

ObjectDesigner::ObjectDesigner(QWidget* parent, Level* level) : QDockWidget("Object Designer", parent, 0)
{
	m_level = level;

	m_widget = new ObjectDesignerWidget(parent, m_level);
	m_window = new QMainWindow(0);

	setFocusPolicy(Qt::ClickFocus);

	connect(m_widget, SIGNAL(onInsertTile(int)), this, SIGNAL(onInsertTile(int)));


	// edit tools
	m_toolgroup = new QActionGroup(this);

	m_draw_poly_action = new QAction(QIcon("polygon.png"), tr("Draw Poly"), this);
	m_draw_poly_action->setCheckable(true);
	connect(m_draw_poly_action, SIGNAL(triggered()), this, SLOT(setDrawPolyMode()));

	m_draw_rect_action = new QAction(QIcon("rect.png"), tr("Draw Rect"), this);
	m_draw_rect_action->setCheckable(true);
	connect(m_draw_rect_action, SIGNAL(triggered()), this, SLOT(setDrawRectMode()));

	m_move_action = new QAction(QIcon("move.png"), tr("Move Object"), this);
	m_move_action->setCheckable(true);
	connect(m_move_action, SIGNAL(triggered()), this, SLOT(setMoveMode()));

	m_edit_vertex_action = new QAction(QIcon("vertexedit.png"), tr("Edit Vertices"), this);
	m_edit_vertex_action->setCheckable(true);
	connect(m_edit_vertex_action, SIGNAL(triggered()), this, SLOT(setEditVertexMode()));

	m_edit_edge_action = new QAction(QIcon("edgeedit.png"), tr("Edit Edges"), this);
	m_edit_edge_action->setCheckable(true);
	connect(m_edit_edge_action, SIGNAL(triggered()), this, SLOT(setEditEdgeMode()));

	m_toolgroup->addAction(m_draw_poly_action);
	m_toolgroup->addAction(m_draw_rect_action);
	m_toolgroup->addAction(m_move_action);
	m_toolgroup->addAction(m_edit_vertex_action);
	m_toolgroup->addAction(m_edit_edge_action);

	// zoom tools
	m_zoom_box = new QComboBox();
	m_zoom_box->setWindowTitle("Zoom");
	for (int i=0; i < ObjectDesigner::NUM_ZOOM_LEVELS; i++)
	{
		m_zoom_box->addItem(tr("%1%").arg((int)(ObjectDesigner::ZOOM_LEVELS[i] * 100.0f)), QVariant(i + 1));
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
	for (int i=0; i < ObjectDesigner::NUM_GRID_SIZES; i++)
	{
		m_gridsize_combo->addItem(tr("%1").arg(ObjectDesigner::GRID_SIZE[i], 0, 'f', 2));
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
	m_edit_toolbar->addAction(m_draw_poly_action);
	m_edit_toolbar->addAction(m_draw_rect_action);
	m_edit_toolbar->addAction(m_move_action);
	m_edit_toolbar->addAction(m_edit_vertex_action);
	m_edit_toolbar->addAction(m_edit_edge_action);
	m_window->addToolBar(m_edit_toolbar);

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

	// draw poly enable initially
	m_draw_poly_action->setChecked(true);
	m_widget->setMode(ObjectDesignerWidget::MODE_DRAW_POLY);


	m_gridsize_combo->setCurrentIndex(3);
	m_widget->setGrid(3);
}

ObjectDesigner::~ObjectDesigner()
{
}


void ObjectDesigner::setDrawPolyMode()
{
	m_widget->setMode(ObjectDesignerWidget::MODE_DRAW_POLY);
}

void ObjectDesigner::setDrawRectMode()
{
	m_widget->setMode(ObjectDesignerWidget::MODE_DRAW_RECT);
}

void ObjectDesigner::setMoveMode()
{
	m_widget->setMode(ObjectDesignerWidget::MODE_MOVE);
}

void ObjectDesigner::setEditVertexMode()
{
	m_widget->setMode(ObjectDesignerWidget::MODE_EDIT_VERTICES);
}

void ObjectDesigner::setEditEdgeMode()
{
	m_widget->setMode(ObjectDesignerWidget::MODE_EDIT_EDGES);
}

void ObjectDesigner::reset()
{
	m_widget->resetObject();
}

void ObjectDesigner::insertTile()
{
	if (m_widget->numPoints() != 4)
	{
		QMessageBox box;		
		box.setText("Wrong number of points");
		box.setInformativeText("Tiles need to have 4 points.");
		box.setStandardButtons(QMessageBox::Ok);
		int ret = box.exec();				
	}
	else
	{
		bool ok;
		QString name = QInputDialog::getText(this, tr("Enter tile name"), tr("Name:"), QLineEdit::Normal, "", &ok);

		if (ok && !name.isEmpty())
		{
			m_widget->insertTile(name);
		}
	}
}



void ObjectDesigner::closeEvent(QCloseEvent* event)
{
	emit onClose();
}

void ObjectDesigner::setTexture(QImage* texture)
{
	m_widget->setTexture(texture);
}


void ObjectDesigner::toggleGrid()
{
	if (m_togglegrid_action->isChecked())
		emit m_widget->enableShowGrid(true);
	else
		emit m_widget->enableShowGrid(false);
}

void ObjectDesigner::snapGrid()
{
	if (m_snapgrid_action->isChecked())
		emit m_widget->enableSnapGrid(true);
	else
		emit m_widget->enableSnapGrid(false);
}

void ObjectDesigner::setGridSize(int size)
{
	emit m_widget->setGrid(size);
}

void ObjectDesigner::chooseColor()
{
	QColor result = QColorDialog::getColor(m_object_color, this, tr("Select object color"));
	if (result.isValid())
	{
		emit m_widget->setColor(result);
		m_color_button->setStyleSheet(tr("background-color: #%1%2%3").arg(result.red(), 2, 16, QChar('0')).arg(result.green(), 2, 16, QChar('0')).arg(result.blue(), 2, 16, QChar('0')));

		m_object_color = result;
	}
}