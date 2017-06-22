#include "TextureEdit.h"

TexEditWidget::TexEditWidget(QWidget* parent, Level* level) : QWidget(parent)
{
	m_level = level;
	m_texture = NULL;
	m_selected_object = -1;

	m_polydef = new PolygonDef(8);
	m_polydef->reset();

	setMode(MODE_MOVE);

	m_dragging = false;

	m_scroll = glm::vec2(0.0f, 0.0f);
	m_panning = false;

	m_zoom = 1.0f;
}

TexEditWidget::~TexEditWidget()
{
	delete m_polydef;
}


void TexEditWidget::setZoom(int zoom)
{
	switch (zoom)
	{
		case 0:		m_zoom = 0.5f; break;
		case 1:		m_zoom = 1.0f; break;
		case 2:		m_zoom = 2.0f; break;
		case 3:		m_zoom = 3.0f; break;
		case 4:		m_zoom = 4.0f; break;
		case 5:		m_zoom = 5.0f; break;
		case 6:		m_zoom = 6.0f; break;
		case 7:		m_zoom = 7.0f; break;
		case 8:		m_zoom = 8.0f; break;
		default:	m_zoom = 1.0f; break;
	}

	m_scroll = glm::vec2(0.0f, 0.0f);

	update();
}


void TexEditWidget::setTexture(QImage* texture)
{
	m_texture = texture;

	update();
}

void TexEditWidget::selectObject(int object_id)
{
	int num_objs = m_level->numObjects();
	for (int i=0; i < num_objs; i++)
	{
		Level::Object* obj = m_level->getObject(i);
		if (obj->getId() == object_id)
		{
			m_selected_object = i;
			objectToEditing(m_selected_object);

			update();
			return;
		}
	}

	m_selected_object = -1;
	update();
}

void TexEditWidget::deselectObject()
{
	m_selected_object = -1;

	update();
}


void TexEditWidget::reset()
{
	if (m_selected_object >= 0)
	{
		objectToEditing(m_selected_object);
	}

	update();
}

void TexEditWidget::commit()
{
	if (m_selected_object >= 0)
	{
		glm::vec2 uvs[8];

		int num_points = m_polydef->getNumPoints();
		for (int i=0; i < num_points; i++)
		{
			uvs[i] = m_polydef->getPoint(i);
		}

		m_level->editObjectUVs(m_selected_object, uvs);
	}
}

void TexEditWidget::resetPan()
{
	m_scroll = glm::vec2(0.0f, 0.0f);

	update();
}


// convert uv coords to screen coordinates
glm::vec2 TexEditWidget::toScreenCoords(glm::vec2& point)
{
	/*
	float x = (point.x * m_area_texside) - m_area_xleft;
	float y = (point.y * m_area_texside) - m_area_ytop;

	return glm::vec2(x, y);
	*/

	float scale = m_zoom;

	float ww = (float)width();
	float hh = (float)height();
	float side = std::min(ww, hh);

	float mult = (float)(side);

	float x = (point.x + m_scroll.x) * mult * scale;
	float y = (point.y + m_scroll.y) * mult * scale;

	/*
	if (ww > hh)
		x += (ww - side) * 0.5f;
	else
		y += (hh - side) * 0.5f;
		*/

	return glm::vec2(x, y);
}

// convert screen coordinates to uv coords
glm::vec2 TexEditWidget::toUVCoords(glm::vec2& point)
{
	/*
	float x = (point.x + m_area_xleft) / m_area_texside;
	float y = (point.y + m_area_ytop) / m_area_texside;

	return glm::vec2(x, y);
	*/

	float scale = 1.0f / m_zoom;

	float ww = (float)width();
	float hh = (float)height();
	float side = std::min(ww, hh);

	float x = point.x * scale;
	float y = point.y * scale;
/*
	if (ww > hh)
		x -= (ww - side) * 0.5f;
	else
		y -= (hh - side) * 0.5f;
		*/

	float mult = 1.0f / side;

	x = (x * mult) - m_scroll.x;
	y = (y * mult) - m_scroll.y;

	return glm::vec2(x, y);
}

float TexEditWidget::makeScalingScale(float scale)
{
	float s = 1.0f + (scale / 1.0f);
	if (s < 0.05f) s = 0.05f;
	return s;
}

float TexEditWidget::makeRotationAngle(float angle)
{
	return angle * 90.0f;
}

void TexEditWidget::objectToEditing(int object)
{
	Level::Object* obj = m_level->getObject(object);

	m_polydef->reset();

	int num_points = obj->getNumPoints();
	for (int i=0; i < num_points; i++)
	{
		glm::vec2 p = obj->getUV(i);

		m_polydef->insertPoint(p);
	}
}

void TexEditWidget::setMode(OperationMode mode)
{
	m_opmode = mode;

	switch (mode)
	{
		case MODE_ROTATE:
		{
			int num_points = m_polydef->getNumPoints();

			if (num_points > 0)
			{
				float minx, maxx, miny, maxy;
				m_polydef->calculateBounds(&minx, &maxx, &miny, &maxy);

				m_rotate_center = glm::vec2(minx + ((maxx - minx) * 0.5f), miny + ((maxy - miny) * 0.5f));
			}
			break;
		}

		case MODE_SCALE:
		{
			int num_points = m_polydef->getNumPoints();

			if (num_points > 0)
			{
				float minx, maxx, miny, maxy;
				m_polydef->calculateBounds(&minx, &maxx, &miny, &maxy);

				m_scale_center = glm::vec2(minx + ((maxx - minx) * 0.5f), miny + ((maxy - miny) * 0.5f));
			}
			break;
		}
	}

	repaint();
}


void TexEditWidget::mouseReleaseEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	if (event->button() & Qt::LeftButton)
	{
		switch (m_opmode)
		{
			case MODE_MOVE:
			{
				if (m_dragging)
				{
					glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_x, mouse_y));
					glm::vec2 delta = m_drag_point - mouse_lp;

					int num_points = m_polydef->getNumPoints();
					for (int i=0; i < num_points; i++)
					{
						glm::vec2 p = m_polydef->getPoint(i);
						m_polydef->edit(i, p - delta);
					}

					m_dragging = false;
				}
				break;
			}

			case MODE_ROTATE:
			{
				if (m_dragging)
				{
					glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_x, mouse_y));
					glm::vec2 delta = m_drag_point - mouse_lp;

					int num_points = m_polydef->getNumPoints();
					for (int i=0; i < num_points; i++)
					{
						float angle = makeRotationAngle(-delta.x);
						glm::vec2 p = m_polydef->getPoint(i);
						glm::vec2 dp = (p - m_rotate_center);
						glm::vec2 tp = glm::rotate(dp, (float)(angle));

						m_polydef->edit(i, m_rotate_center + tp);
					}

					m_dragging = false;
				}
				break;
			}

			case MODE_SCALE:
			{
				if (m_dragging)
				{
					glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_x, mouse_y));
					glm::vec2 delta = m_drag_point - mouse_lp;

					int num_points = m_polydef->getNumPoints();
					for (int i=0; i < num_points; i++)
					{
						float scale = makeScalingScale(-delta.x);
						glm::vec2 p = m_polydef->getPoint(i);
						p = m_scale_center + ((p - m_scale_center) * scale);

						m_polydef->edit(i, p);
					}

					m_dragging = false;
				}
				break;
			}
		}
	}
	else if (event->button() & Qt::RightButton)
	{
		if (m_panning)
		{
			m_panning = false;
		}
	}

	update();
}

void TexEditWidget::mousePressEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	switch (m_opmode)
	{
		case MODE_MOVE:
		case MODE_ROTATE:
		case MODE_SCALE:
		{
			if (event->button() & Qt::LeftButton)
			{
				glm::vec2 mouse_uvp = toUVCoords(glm::vec2(mouse_x, mouse_y));
				bool inside = m_polydef->isPointInside(mouse_uvp);
				if (inside)
				{
					m_drag_point = mouse_uvp;
					m_dragging = true;
				}
			}
			else if (event->button() & Qt::RightButton)
			{
				if (!m_panning)
				{
					m_pan_point = toUVCoords(glm::vec2(mouse_x, mouse_y));
					m_scroll_saved = m_scroll;
					m_panning = true;
				}
			}
			break;
		}
	}

	update();
}

void TexEditWidget::keyReleaseEvent(QKeyEvent* event)
{
	// nothing to do here, key handling happens in parent widget

	update();
}

void TexEditWidget::mouseMoveEvent(QMouseEvent* event)
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



void TexEditWidget::paintEvent(QPaintEvent* event)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());
	QPainter painter;

	painter.begin(this);
	
	painter.fillRect(0, 0, width(), height(), QColor(0,0,0));

	float w = (float)(width());
	float h = (float)(height());

	m_area_texside = std::min(w, h) - 10;

	m_area_xleft = 0.0f - ((w - m_area_texside) / 2.0f);
	m_area_xright = w - ((w - m_area_texside) / 2.0f);
	m_area_ytop = 0.0f - ((h - m_area_texside) / 2.0f);
	m_area_ybottom = h - ((h - m_area_texside) / 2.0f);


	if (m_selected_object >= 0)
	{
		if (m_texture)
		{
			glm::vec2 tl = toScreenCoords(glm::vec2(0.0f, 0.0f));
			glm::vec2 br = toScreenCoords(glm::vec2(1.0f, 1.0f));
			painter.drawImage(QRect(tl.x, tl.y, br.x-tl.x, br.y-tl.y), *m_texture);
		}

		QColor color;
		QColor drag_color = QColor(224, 0, 0);
		QColor normal_color = QColor(0, 224, 64);

		if (m_dragging)
			color = drag_color;
		else
			color = normal_color;

		int num_points = m_polydef->getNumPoints();
		if (num_points >= 2)
		{
			glm::vec2 mouse_lp = toUVCoords(glm::vec2(mouse_p.x(), mouse_p.y()));
			glm::vec2 delta;
			
			if (m_dragging)
				delta = m_drag_point - mouse_lp;
			else
				delta = glm::vec2(0.0f, 0.0f);

			QPointF ppoints[8];

			for (int i=0; i < num_points; i++)
			{
				glm::vec2 p;
				switch (m_opmode)
				{
					case MODE_MOVE:
					{
						p = toScreenCoords(m_polydef->getPoint(i) - delta);
						break;
					}

					case MODE_ROTATE:
					{
						float angle = makeRotationAngle(-delta.x);
						glm::vec2 sp = m_polydef->getPoint(i);
						glm::vec2 dp = (sp - m_rotate_center);
						glm::vec2 tp = glm::rotate(dp, (float)(angle));

						p = toScreenCoords(m_rotate_center + tp);
						break;
					}

					case MODE_SCALE:
					{
						float scale = makeScalingScale(-delta.x);
						if (scale < 0.05f) scale = 0.05f;
						glm::vec2 sp = m_polydef->getPoint(i);
						sp = m_scale_center + ((sp - m_scale_center) * scale);

						p = toScreenCoords(sp);
						break;
					}
				}

				ppoints[i].setX(p.x);
				ppoints[i].setY(p.y);
			}

			// polygon
			painter.setPen(color);
			painter.setBrush(Qt::NoBrush);
			painter.drawConvexPolygon(ppoints, num_points);

			// points
			for (int i=0; i < num_points; i++)
			{
				painter.fillRect(ppoints[i].x()-2, ppoints[i].y()-2, 5, 5, color);
			}

			// draw rotation hint when rotating
			if (m_opmode == MODE_ROTATE && delta.x != 0.0f)
			{
				int pw = width() / 10;
				int ph = height() / 10;
				glm::vec2 center = toScreenCoords(m_rotate_center);

				QRect pierect = QRect(center.x - (pw / 2), center.y - (ph / 2), pw, ph);

				float scale = (delta.x * 90.0f) * 16.0f;
				painter.setPen(QColor(224, 0, 0));
				painter.setBrush(QColor(224, 0, 0, 128));
				painter.drawPie(pierect, 0, scale);
			}
		}
	}
	else
	{
		painter.setPen(QColor(255,255,255));
		painter.drawText(QRect(0, 0, width(), height()), Qt::AlignHCenter|Qt::AlignVCenter, "No selected object");
	}
	
	painter.end();
}








TextureEdit::TextureEdit(QWidget* parent, Level* level) : QDockWidget("Texture Edit", parent, 0)
{
	m_widget = new TexEditWidget(this, level);
	m_window = new QMainWindow(0);
	

	// make tools
	m_toolgroup = new QActionGroup(this);

	m_move_action = new QAction(QIcon("move.png"), tr("Move"), this);
	m_move_action->setCheckable(true);
	connect(m_move_action, SIGNAL(triggered()), this, SLOT(moveMode()));

	m_rotate_action = new QAction(QIcon("rotate.png"), tr("Rotate"), this);
	m_rotate_action->setCheckable(true);
	connect(m_rotate_action, SIGNAL(triggered()), this, SLOT(rotateMode()));

	m_scale_action = new QAction(QIcon("scale.png"), tr("Scale"), this);
	m_scale_action->setCheckable(true);
	connect(m_scale_action, SIGNAL(triggered()), this, SLOT(scaleMode()));

	m_reset_action = new QAction(tr("Reset"), this);
	connect(m_reset_action, SIGNAL(triggered()), this, SLOT(reset()));
	
	m_commit_action = new QAction(tr("Commit"), this);
	connect(m_commit_action, SIGNAL(triggered()), this, SLOT(commit()));

	m_toolgroup->addAction(m_move_action);
	m_toolgroup->addAction(m_rotate_action);
	m_toolgroup->addAction(m_scale_action);

	m_move_action->setChecked(true);


	m_zoom_box = new QComboBox();
	m_zoom_box->setWindowTitle("Zoom");
	m_zoom_box->addItem("50%", QVariant(1));
	m_zoom_box->addItem("100%", QVariant(2));
	m_zoom_box->addItem("200%", QVariant(3));
	m_zoom_box->addItem("300%", QVariant(4));
	m_zoom_box->addItem("400%", QVariant(5));
	m_zoom_box->addItem("500%", QVariant(6));
	m_zoom_box->addItem("600%", QVariant(7));
	m_zoom_box->addItem("700%", QVariant(8));
	m_zoom_box->addItem("800%", QVariant(9));
	m_zoom_box->setFocusPolicy(Qt::NoFocus);


	// add toolbar
	m_toolbar = new QToolBar(m_window);
	m_toolbar->addAction(m_move_action);
	m_toolbar->addAction(m_rotate_action);
	m_toolbar->addAction(m_scale_action);
	m_toolbar->addAction(m_reset_action);
	m_toolbar->addAction(m_commit_action);
	m_toolbar->addWidget(m_zoom_box);
	m_window->addToolBar(m_toolbar);


	connect(m_zoom_box, SIGNAL(currentIndexChanged(int)), m_widget, SLOT(setZoom(int)));

	m_window->setParent(this);
	setWidget(m_window);

	m_window->setCentralWidget(m_widget);
	m_window->setContextMenuPolicy(Qt::NoContextMenu);

	setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
	setFloating(true);

	this->setMinimumWidth(500);
	this->setMinimumHeight(500);
	this->setMaximumWidth(800);
	this->setMaximumHeight(800);

	// don't allow docking
	this->setAllowedAreas(0);

	this->setHidden(false);

	m_zoom_box->setCurrentIndex(1);
}

TextureEdit::~TextureEdit()
{
}

void TextureEdit::setTexture(QImage* texture)
{
	m_widget->setTexture(texture);
}

// Slots
// ----------------------------------------------------------------------------

void TextureEdit::select(int object_id)
{
	m_widget->setMode(TexEditWidget::MODE_MOVE);
	m_move_action->setChecked(true);

	m_widget->selectObject(object_id);
}

void TextureEdit::deselect()
{
	m_widget->setMode(TexEditWidget::MODE_MOVE);
	m_move_action->setChecked(true);

	m_widget->deselectObject();
}

void TextureEdit::remove(int object_id)
{
	deselect();
}

void TextureEdit::moveMode()
{
	m_widget->setMode(TexEditWidget::MODE_MOVE);
}

void TextureEdit::rotateMode()
{
	m_widget->setMode(TexEditWidget::MODE_ROTATE);
}

void TextureEdit::scaleMode()
{
	m_widget->setMode(TexEditWidget::MODE_SCALE);
}

void TextureEdit::reset()
{
	m_widget->reset();
}

void TextureEdit::commit()
{
	m_widget->commit();
}

// ----------------------------------------------------------------------------


void TextureEdit::closeEvent(QCloseEvent* event)
{
	emit onClose();
}

void TextureEdit::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();

	if (key == Qt::Key_Space)
	{
		m_widget->commit();
	}
	else if (key == Qt::Key_Home)
	{
		m_widget->resetPan();
	}
}