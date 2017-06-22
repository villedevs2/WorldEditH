#include "GLWidget.h"

const float GLWidget::GRID_SIZE[GLWidget::NUM_GRID_SIZES] = 
{
	0.1f,
	0.25f,
	0.5f,
	1.0f,
	2.0f,
	4.0f,
};

const float GLWidget::ZOOM_LEVELS[GLWidget::NUM_ZOOM_LEVELS] =
{
	0.1f,
	0.25f,
	0.5f,
	1.0f,
	2.0f,
	3.0f,
};

GLWidget::GLWidget(QWidget *parent, Level* level)
				   : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
	m_bgcolor = QColor(0, 0, 0, 0);
	m_opmode = MODE_SELECT;

	m_grid_vb = new float[16384];

	m_polydef = new PolygonDef(8);

	setFocusPolicy(Qt::StrongFocus);

	m_level = level;

	m_validpoly_color = QColor(224, 224, 224, 128);
	m_errorpoly_color = QColor(224, 0, 0, 128);
	m_validline_color = QColor(224, 224, 224);
	m_errorline_color = QColor(224, 0, 0);
	m_point_color = QColor(224, 224, 224);

	m_selected_object = -1;
	m_selected_point = -1;

	m_dragging = false;
	m_panning = false;

	m_saved_object = new Level::Object(-1, (glm::vec2*)NULL, (glm::vec2*)NULL, 0, Level::OBJECT_TYPE_TRIGGER, std::string(""), 0);

	m_grid_size = 0;

	m_visbox_width = 32.0f;
	m_visbox_height = 20.0f;

	m_tilemap_width = 1.0f;
	m_tilemap_height = 1.0f;
	m_tilemap_xstart = 0;
	m_tilemap_xend = 50;
	m_tilemap_ystart = 0;
	m_tilemap_yend = 50;

	m_filter = 0;
	m_display_filter = 0;

	m_tile_brush = -1;

	m_create_poly_color = 0xffffffff;

	reset();
}

GLWidget::~GLWidget(void)
{
	delete m_polydef;
	delete m_saved_object;

	delete [] m_grid_vb;
}


QSize GLWidget::minimumSizeHint() const
{
	return QSize(200, 100);
}

QSize GLWidget::sizeHint() const
{
	return QSize(1600, 400);
}

void GLWidget::reset()
{
	m_scroll = glm::vec2(0.0f, 0.0f);
	m_panning = false;

	m_saved_object->reset();

	m_create_type = Level::OBJECT_TYPE_TRIGGER;
	m_create_trigger_type = 0;
}


// SLOTS
// ----------------------------------------------------------------------------

void GLWidget::animate()
{
	if (m_level->isVBOUpdated())
	{
		update();

		m_level->resetVBOUpdate();
	}
}

void GLWidget::setGridSize(int size)
{
	assert(size >= 0 && size < GLWidget::NUM_GRID_SIZES);

	m_grid_size = size;

	update();
}

void GLWidget::setZoomLevel(int zoom)
{
	assert(zoom >= 0 && zoom < GLWidget::NUM_ZOOM_LEVELS);

	m_zoom_level = zoom;

	update();
}

void GLWidget::setMode(OperationMode mode)
{
	m_opmode = mode;

	switch (mode)
	{
		case MODE_DRAW_POLY:
		{
			deselectObject();
			break;
		}

		case MODE_DRAW_RECT:
		{
			deselectObject();
			break;
		}

		case MODE_SELECT:
		{
			deselectObject();
			break;
		}

		case MODE_MOVE:
		case MODE_ROTATE:
		case MODE_SCALE:
		{
			deselectObject();
			break;
		}

		case MODE_TILEMAP:
		{
			deselectObject();
			m_tilemap_painting = false;
			break;
		}
	}

	emit onSetMode(mode);

	update();
}

void GLWidget::select(int object_id)
{
	if (m_opmode == MODE_SELECT ||
		m_opmode == MODE_MOVE ||
		m_opmode == MODE_ROTATE ||
		m_opmode == MODE_SCALE)
	{
		m_selected_object = m_level->getIndexById(object_id);
		if (m_selected_object >= 0)
		{
			m_selected_point = -1;
			objectToEditing(m_selected_object);
		}
	}

	update();
}

void GLWidget::deselect()
{
	m_selected_object = -1;
	m_selected_point = -1;
	m_polydef->reset();

	update();
}

void GLWidget::copy()
{
	if (m_selected_object >= 0)
	{
		Level::Object* obj = m_level->getObject(m_selected_object);

		// get minimum point for mouse delta
		float minx, miny;
		minx = obj->getPoint(0).x;
		miny = obj->getPoint(0).y;
		for (int i=0; i < obj->getNumPoints(); i++)
		{
			glm::vec2 p = obj->getPoint(i);
			if (p.x < minx)
				minx = p.x;
			if (p.y < miny)
				miny = p.y;
		}

		m_copy_delta = glm::vec2(minx, miny);

		m_saved_object->copy(*obj);
	}
}

void GLWidget::paste()
{
	int num_points = m_saved_object->getNumPoints();
	if (num_points > 0)
	{
		std::string name = m_saved_object->getName();
		Level::ObjectType type = m_saved_object->getType();

		QPoint mp = mapFromGlobal(QCursor::pos());
		glm::vec2 mouse_p = toLevelCoords(glm::vec2(mp.x(), mp.y()));

		glm::vec2 mouse_delta = mouse_p - m_copy_delta;

		if (m_snap_grid)
			mouse_delta = snapToGrid(mouse_delta);

		glm::vec2 points[8], uvs[8];

		for (int i=0; i < num_points; i++)
		{
			points[i] = m_saved_object->getPoint(i) + mouse_delta;		// nudge down right so objects are not on top of each other
			uvs[i] = m_saved_object->getUV(i);
		}

		unsigned int color = m_saved_object->getColor();

		int id = m_level->insertObject(points, uvs, num_points, type, name, color);
		emit onAddObject(id);

		Level::Object* obj = m_level->getObjectById(id);
		if (obj != NULL)
		{
			for (int i=0; i < Level::Object::NUM_PARAMS; i++)
			{
				obj->setParam(i, m_saved_object->getParam(i));
			}

			obj->setZ(m_saved_object->getZ());
		}
	}

	update();
}

void GLWidget::setCreateType(Level::ObjectType type)
{
	m_create_type = type;
}

void GLWidget::setCreateTriggerType(int type)
{
	m_create_trigger_type = type;
}

void GLWidget::enableFilter(int filter)
{
	if (filter != m_filter)
		deselectObject();

	m_filter |= filter;
}

void GLWidget::disableFilter(int filter)
{
	if (filter != m_filter)
		deselectObject();

	m_filter &= ~filter;
}

void GLWidget::enableDisplays(int filter)
{
	m_display_filter |= filter;
}

void GLWidget::disableDisplays(int filter)
{
	m_display_filter &= ~filter;
}

void GLWidget::setBGColor(QColor color)
{
	m_bgcolor = color;

	update();
}

void GLWidget::setObjectColor(QColor color)
{
	m_create_poly_color = 0xff000000 | (color.blue() << 16) | (color.green() << 8) | color.red();

	update();
}

void GLWidget::setTilemapConfig(int xstart, int xend, int ystart, int yend, float tile_width, float tile_height)
{
	m_tilemap_xstart = xstart;
	m_tilemap_xend = xend;
	m_tilemap_ystart = ystart;
	m_tilemap_yend = yend;

	m_tilemap_width = tile_width;
	m_tilemap_height = tile_height;

	update();
}

void GLWidget::setTileBrush(int tile)
{
	assert(tile >= -1 && tile < m_level->getNumTiles());

	m_tile_brush = tile;
}

// ----------------------------------------------------------------------------



void GLWidget::selectObject(int object)
{
	assert(object >= 0 && object < m_level->numObjects());

	m_selected_object = object;
	m_selected_point = -1;
	m_polydef->reset();

	int id = m_level->getObject(m_selected_object)->getId();
	emit onSelectObject(id);

	update();
}

void GLWidget::deselectObject()
{
	m_selected_object = -1;
	m_selected_point = -1;
	m_polydef->reset();

	emit onDeselectObject();

	update();
}


// transfer level object to onscreen editing polygon
void GLWidget::objectToEditing(int object)
{
	Level::Object* obj = m_level->getObject(object);

	m_polydef->reset();
	for (int i=0; i < obj->getNumPoints(); i++)
	{
		m_polydef->insertPoint(obj->getPoint(i));
	}
}







glm::vec2 GLWidget::toLevelCoords(glm::vec2 point)
{
	float x = point.x;
	float y = point.y;

	float mult = 1.0f / ((float)(width()) / (float)(LEVEL_VIS_WIDTH / ZOOM_LEVELS[m_zoom_level]));

	float sx = (x * mult) - m_scroll.x;
	float sy = (y * mult) - m_scroll.y;

	return glm::vec2(sx, sy);
}

glm::vec2 GLWidget::toScreenCoords(glm::vec2 point)
{
	float x = point.x;
	float y = point.y;

	float mult = (float)(width()) / (float)(LEVEL_VIS_WIDTH / ZOOM_LEVELS[m_zoom_level]);

	float lx = (x + m_scroll.x) * mult;
	float ly = (y + m_scroll.y) * mult;

	return glm::vec2(lx, ly);
}


glm::vec2 GLWidget::snapToGrid(glm::vec2 point)
{
	float grid_size = GRID_SIZE[m_grid_size];

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

float GLWidget::makeScalingScale(float scale)
{
	float ret = 1.0f + (scale / 16.0f);
	if (ret < 0.03f) ret = 0.03f;		// can't scale down below 3%
	return ret;
}

float GLWidget::makeRotationAngle(float angle)
{
	return angle * 500.0f * M_PI / 180.0f;
}


// returns true if the object passes the filtering (= enabled)
bool GLWidget::filterObject(Level::Object* obj)
{
	if (m_filter & (1 << (obj->getType()-1)))
		return true;
	else
		return false;
}



void GLWidget::tilemapDraw(glm::vec2 mouse_lp)
{
	float tx1 = (float)(m_tilemap_xstart) * m_tilemap_width;
	float ty1 = (float)(m_tilemap_ystart) * m_tilemap_width;
	float tx2 = (float)(m_tilemap_xend) * m_tilemap_height;
	float ty2 = (float)(m_tilemap_yend) * m_tilemap_height;

	if (mouse_lp.x >= tx1 && mouse_lp.x < tx2 &&
		mouse_lp.y >= ty1 && mouse_lp.y < ty2)
	{
		float fx = mouse_lp.x / m_tilemap_width;
		float fy = mouse_lp.y / m_tilemap_height;

		int tile_x, tile_y;
		if (fx >= 0.0f)
			tile_x = (int)(fx);
		else
			tile_x = (int)(fx - 1.0f);

		if (fy >= 0.0f)
			tile_y = (int)(fy);
		else
			tile_y = (int)(fy - 1.0f);

		m_level->editTilemap(tile_x, tile_y, m_tile_brush);
	}
}




void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	switch (m_opmode)
	{
		case MODE_DRAW_POLY:
		{
			if (event->button() & Qt::LeftButton)
			{
				glm::vec2 p = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				if (m_snap_grid)
					p = snapToGrid(p);

				m_polydef->insertPoint(p);
			}
			break;
		}


		case MODE_DRAW_RECT:
		{
			if (event->button() & Qt::LeftButton)
			{
				if (m_dragging)
				{
					glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));

					if (m_snap_grid)
						mouse_p = snapToGrid(mouse_p);

					glm::vec2 delta = mouse_p - m_drag_point;

					if (delta.x != 0.0f && delta.y != 0.0f)
					{
						glm::vec2 points[4];
						glm::vec2 uvs[4];

						if (delta.x < 0.0f && delta.y < 0.0f)		// end point = top left
						{
							points[0] = glm::vec2(mouse_p.x, mouse_p.y);
							points[1] = glm::vec2(mouse_p.x, m_drag_point.y);
							points[2] = glm::vec2(m_drag_point.x, m_drag_point.y);
							points[3] = glm::vec2(m_drag_point.x, mouse_p.y);
						}
						else if (delta.x > 0.0f && delta.y < 0.0f)	// end point = top right
						{
							points[0] = glm::vec2(m_drag_point.x, mouse_p.y);
							points[1] = glm::vec2(m_drag_point.x, m_drag_point.y);
							points[2] = glm::vec2(mouse_p.x, m_drag_point.y);
							points[3] = glm::vec2(mouse_p.x, mouse_p.y);
						}
						else if (delta.x < 0.0f && delta.y > 0.0f)	// end point = bottom left
						{
							points[0] = glm::vec2(mouse_p.x, m_drag_point.y);
							points[1] = glm::vec2(mouse_p.x, mouse_p.y);
							points[2] = glm::vec2(m_drag_point.x, mouse_p.y);
							points[3] = glm::vec2(m_drag_point.x, m_drag_point.y);
						}
						else if (delta.x > 0.0f && delta.y > 0.0f)	// end point = bottom right
						{
							points[0] = glm::vec2(m_drag_point.x, m_drag_point.y);
							points[1] = glm::vec2(m_drag_point.x, mouse_p.y);
							points[2] = glm::vec2(mouse_p.x, mouse_p.y);
							points[3] = glm::vec2(mouse_p.x, m_drag_point.y);
						}

						float minx = points[0].x;
						float maxx = minx;
						float miny = points[0].y;
						float maxy = miny;

						// find min/max
						for (int i=0; i < 4; i++)
						{
							glm::vec2 p = points[i];

							if (p.x < minx)
								minx = p.x;
							if (p.x > maxx)
								maxx = p.x;
							if (p.y < miny)
								miny = p.y;
							if (p.y > maxy)
								maxy = p.y;
						}

						float scale = std::max(maxx - minx, maxy - miny);
						if (scale == 0.0f ) scale = 1.0f;

						// process uvs
						for (int i=0; i < 4; i++)
						{
							float u = (points[i].x - minx) / scale;
							float v = (points[i].y - miny) / scale;

							uvs[i] = glm::vec2(u, v);
						}
						
						std::string name = "Unknown";
						switch (m_create_type)
						{
							case Level::OBJECT_TYPE_DESTRUCTIBLE:	name = "Destructible"; break;
							case Level::OBJECT_TYPE_MOVER:			name = "Mover"; break;
							case Level::OBJECT_TYPE_TRIGGER:
							{								
								name = m_level->getTriggerName(m_create_trigger_type);
								break;
							}
							case Level::OBJECT_TYPE_ENEMY:			name = "Enemy"; break;
						}

						// generate polygon
						int id = m_level->insertObject(points, uvs, 4, m_create_type, name, m_create_poly_color);
						emit onAddObject(id);

						if (m_create_type == Level::OBJECT_TYPE_TRIGGER)
						{
							Level::Object* obj = m_level->getObjectById(id);
							Level::Object::Param param;
							param.i = m_create_trigger_type;
							obj->setParam(0, param);
						}
					}
					// reset for next polygon
					m_polydef->reset();

					m_dragging = false;
				}
			}
			break;
		}


		case MODE_SELECT:
		{
			if (event->button() & Qt::LeftButton)
			{
				glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				int num_objs = m_level->numObjects();

				if (m_selected_object < 0)
				{
					deselectObject();

					for (int i=0; i < num_objs; i++)
					{
						Level::Object* obj = m_level->getObject(i);
						
						if (obj->pointInside(mouse_p, 0.25f) && filterObject(obj))
						{
							// show the editing polygon if we have an object
							selectObject(i);

							objectToEditing(i);
							break;
						}
					}
				}

				if (m_selected_point >= 0)
				{
					// validate and finalize point edit
					bool valid = m_polydef->fullConvexTest();
				
					if (!valid)
					{
						// restore edit
						m_polydef->edit(m_selected_point, m_saved_point);
					}

					m_selected_point = -1;
				}

			}
			break;
		}

		case MODE_MOVE:
		case MODE_ROTATE:
		case MODE_SCALE:
		{
			if (event->button() & Qt::LeftButton)
			{
				// finish drag
				if (m_selected_object >= 0 && m_dragging)
				{
					int num_points = m_polydef->getNumPoints();
					glm::vec2 points[8];

					glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));

					if (m_snap_grid)
						mouse_p = snapToGrid(mouse_p);

					glm::vec2 delta = mouse_p - m_drag_point;

					// process vertices
					for (int i=0; i < num_points; i++)
					{
						switch (m_opmode)
						{
							case MODE_MOVE:
							{
								points[i] = m_polydef->getPoint(i) + delta;
								break;
							}
							case MODE_ROTATE:
							{
								float angle = makeRotationAngle(delta.x);
								glm::vec2 pp = m_polydef->getPoint(i);
								glm::vec2 dp = (pp - m_object_center);
								points[i] = glm::rotate(dp, (float)(angle)) + m_object_center;
								break;
							}
							case MODE_SCALE:
							{
								float scale = makeScalingScale(delta.x);
								glm::vec2 pp = m_polydef->getPoint(i);
								points[i] = m_object_center + ((pp - m_object_center) * scale);
								break;
							}
						}
					}

					// edit polygon
					m_level->editObjectGeo(m_selected_object, points);
					m_dragging = false;

					// update editing polygon
					objectToEditing(m_selected_object);
				}

				// select
				if (!m_dragging)
				{
					glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));

					int previous_sel = m_selected_object;

					deselectObject();

					std::vector<int> selection;
					selection.clear();

					int num_objs = m_level->numObjects();
					for (int i=0; i < num_objs; i++)
					{
						Level::Object* obj = m_level->getObject(i);

						if (obj->pointInside(mouse_p, 0.25f) && filterObject(obj))
						{
							selection.push_back(i);
						}
					}

					int selobj = -1;
					if (selection.size() == 1)
					{
						selobj = selection[0];
					}
					else if (selection.size() > 1)
					{
						if (selection[0] == previous_sel)
							selobj = selection[1];
						else
							selobj = selection[0];
					}

					if (selobj >= 0)
					{
						// show the editing polygon if we have an object
						selectObject(selobj);
						objectToEditing(selobj);

						m_drag_point = mouse_p;
						m_dragging = false;
					}
				}
			}
			break;
		}

		case MODE_TILEMAP:
		{
			if (event->button() & Qt::LeftButton)
			{
				m_tilemap_painting = false;
			}
			break;
		}
	}

	if ((event->button() & Qt::RightButton) && m_panning)
	{
		m_panning = false;
	}

	update();
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	switch (m_opmode)
	{
		case MODE_SELECT:		// object point editing
		{
			if (event->button() & Qt::LeftButton)
			{
				if (m_selected_object >= 0)
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
							m_saved_point = m_polydef->getPoint(i);
							break;
						}
					}
				}
			}

			break;
		}

		case MODE_MOVE:			// object dragging
		case MODE_ROTATE:		// object rotation
		case MODE_SCALE:		// object scaling
		{
			if (event->button() & Qt::LeftButton)
			{
				// only start dragging if we are pointing inside the currently selected object
				if (m_selected_object >= 0)
				{
					glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));
					Level::Object* obj = m_level->getObject(m_selected_object);
						
					if (obj->pointInside(mouse_p, 0.25f) && filterObject(obj))
					{
						m_drag_point = mouse_p;

						if (m_snap_grid)
							m_drag_point = snapToGrid(m_drag_point);

						// get object center for rotate/scale
						m_object_center = obj->getBoundingMin() + ((obj->getBoundingMax() - obj->getBoundingMin()) * 0.5f);

						m_dragging = true;
					}
				}
					
			}

			break;
		}

		case MODE_DRAW_RECT:		// rectangle drawing
		{
			if (event->button() & Qt::LeftButton)
			{
				m_drag_point = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				if (m_snap_grid)
					m_drag_point = snapToGrid(m_drag_point);

				m_dragging = true;
			}
			break;
		}
		
		case MODE_TILEMAP:
		{
			if (event->button() & Qt::LeftButton)
			{
				glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				tilemapDraw(mouse_lp);
				m_tilemap_painting = true;
			}
			break;
		}
	}

	if (event->button() & Qt::RightButton && !m_panning)
	{
		m_scroll_saved = m_scroll;
		m_pan_point = toLevelCoords(glm::vec2(mouse_x, mouse_y));
		m_panning = true;
	}

	update();
}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
	QPoint point = this->mapFromGlobal(event->globalPos());

	float mouse_x = point.x();
	float mouse_y = point.y();

	switch (m_opmode)
	{
		case MODE_SELECT:
		{
			if (m_selected_point >= 0)
			{
				glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				if (m_snap_grid)
					mouse_p = snapToGrid(mouse_p);

				m_polydef->edit(m_selected_point, mouse_p);
			}
			break;
		}

		case MODE_TILEMAP:
		{
			if (m_tilemap_painting)
			{
				glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_x, mouse_y));
				tilemapDraw(mouse_lp);
			}

			/*
			//if (event->button() & Qt::LeftButton)
			if (m_tilemap_painting)
			{
				glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				float tx1 = (float)(m_tilemap_xstart) * m_tilemap_size;
				float ty1 = (float)(m_tilemap_ystart) * m_tilemap_size;
				float tx2 = (float)(m_tilemap_xend) * m_tilemap_size;
				float ty2 = (float)(m_tilemap_yend) * m_tilemap_size;

				if (mouse_lp.x >= tx1 && mouse_lp.x < tx2 &&
					mouse_lp.y >= ty1 && mouse_lp.y < ty2)
				{
			//		float x = (mouse_lp.x - tx1) / (tx2 - tx1);
			//		float y = (mouse_lp.y - ty1) / (ty2 - ty1);

			//		int tile_x = (int)((x / m_tilemap_size) * (float)(m_tilemap_xend - m_tilemap_xstart));
			//		int tile_y = (int)((y / m_tilemap_size) * (float)(m_tilemap_yend - m_tilemap_ystart));

					int tile_x = (int)(mouse_lp.x / m_tilemap_size);
					int tile_y = (int)(mouse_lp.y / m_tilemap_size);

					m_level->editTilemap(tile_x, tile_y, 1);
				}
			}
			*/
			break;
		}
	}

	if (m_panning)
	{
		glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));
		glm::vec2 delta = (m_pan_point - mouse_p) * 0.4f;

		m_scroll = m_scroll_saved - delta;
	}

	update();
}

void GLWidget::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();

	switch (m_opmode)
	{
		case MODE_DRAW_POLY:
		{
			if (key == Qt::Key_Escape)
			{
				m_polydef->reset();
			}
			else if (key == Qt::Key_Backspace)
			{
				m_polydef->deleteLatest();
			}
			else if (key == Qt::Key_Space)
			{
				int num_points = m_polydef->getNumPoints();

				// closing must retain convexity and have at least 3 points
				if (num_points >= 3 && m_polydef->fullConvexTest())
				{
					glm::vec2 points[8];
					glm::vec2 uvs[8];

					glm::vec2 p0 = m_polydef->getPoint(0);

					float minx = p0.x;
					float maxx = minx;
					float miny = p0.y;
					float maxy = miny;

					// find min/max
					for (int i=0; i < num_points; i++)
					{
						glm::vec2 p = m_polydef->getPoint(i);

						if (p.x < minx)
							minx = p.x;
						if (p.x > maxx)
							maxx = p.x;
						if (p.y < miny)
							miny = p.y;
						if (p.y > maxy)
							maxy = p.y;
					}

					float scale = std::max(maxx - minx, maxy - miny);
					if (scale == 0.0f ) scale = 1.0f;

					// process vertices
					for (int i=0; i < num_points; i++)
					{
						glm::vec2 p = m_polydef->getPoint(i);
						points[i] = p;

						float u = (p.x - minx) / scale;
						float v = (p.y - miny) / scale;

						uvs[i] = glm::vec2(u, v);
					}

					std::string name = "Unknown";
					switch (m_create_type)
					{
						case Level::OBJECT_TYPE_DESTRUCTIBLE:	name = "Destructible"; break;
						case Level::OBJECT_TYPE_MOVER:			name = "Mover"; break;
						case Level::OBJECT_TYPE_TRIGGER:
						{
							name = m_level->getTriggerName(m_create_trigger_type);
							break;
						}
						case Level::OBJECT_TYPE_ENEMY:			name = "Enemy"; break;
					}

					// generate polygon
					int id = m_level->insertObject(points, uvs, num_points, m_create_type, name, m_create_poly_color);
					emit onAddObject(id);

					if (m_create_type == Level::OBJECT_TYPE_TRIGGER)
					{
						Level::Object* obj = m_level->getObjectById(id);
						Level::Object::Param param;
						param.i = m_create_trigger_type;
						obj->setParam(0, param);
					}

					// reset for next polygon
					m_polydef->reset();
				}
			}
			break;
		}

		case MODE_DRAW_RECT:
		{
			if (key == Qt::Key_Escape ||
				key == Qt::Key_Backspace)
			{
				// cancel drawing
				m_dragging = false;
			}
			break;
		}

		case MODE_SELECT:
		{
			if (key == Qt::Key_Escape)
			{
				// discard all edits
				
				// restore edit
				if (m_selected_point >= 0)
					m_polydef->edit(m_selected_point, m_saved_point);

				deselectObject();
			}
			else if (key == Qt::Key_Delete)
			{
				if (m_selected_object >= 0)
				{
					Level::Object* obj = m_level->getObject(m_selected_object);
					emit onRemoveObject(obj->getId());

					m_level->removeObject(m_selected_object);
					
					deselectObject();
				}
			}
			else if (key == Qt::Key_Space)
			{
				if (m_selected_object >= 0)
				{
					// validate and finalize edits
					bool valid = m_polydef->fullConvexTest();
					if (valid)
					{
						int num_points = m_polydef->getNumPoints();

						// closing must retain convexity and have at least 3 points
						if (num_points >= 3 && m_polydef->fullConvexTest())
						{
							glm::vec2 points[8];
							glm::vec2 uvs[8];

							glm::vec2 p0 = m_polydef->getPoint(0);

							float minx = p0.x;
							float maxx = minx;
							float miny = p0.y;
							float maxy = miny;
	
							// find min/max
							for (int i=0; i < num_points; i++)
							{
								glm::vec2 p = m_polydef->getPoint(i);

								if (p.x < minx)
									minx = p.x;
								if (p.x > maxx)
									maxx = p.x;
								if (p.y < miny)
									miny = p.y;
								if (p.y > maxy)
									maxy = p.y;
							}

							float scale = std::max(maxx - minx, maxy - miny);
							if (scale == 0.0f ) scale = 1.0f;

							// process vertices
							for (int i=0; i < num_points; i++)
							{
								glm::vec2 p = m_polydef->getPoint(i);
								points[i] = p;

								float u = (p.x - minx) / scale;
								float v = (p.y - miny) / scale;

								uvs[i] = glm::vec2(u, v);
							}

							// edit polygon
							m_level->editObjectGeo(m_selected_object, points, uvs);
						}
					}

					deselectObject();
				}
			}

			break;
		}

		case MODE_MOVE:
		case MODE_ROTATE:
		case MODE_SCALE:
		{
			if (key == Qt::Key_Escape)
			{
				deselectObject();
				m_dragging = false;
			}
			else if (key == Qt::Key_Delete)
			{
				if (m_selected_object >= 0)
				{
					Level::Object* obj = m_level->getObject(m_selected_object);
					emit onRemoveObject(obj->getId());

					m_level->removeObject(m_selected_object);
					
					deselectObject();
				}
			}
			break;
		}
	}

	if (key == Qt::Key_Home)
	{
		m_scroll = glm::vec2(0.0f, 0.0f);
	}

	update();
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();

	if (key == Qt::Key_Left)
	{
		m_scroll += glm::vec2(1.0f, 0.0f);
	}
	else if (key == Qt::Key_Right)
	{
		m_scroll += glm::vec2(-1.0f, 0.0f);
	}
	else if (key == Qt::Key_Up)
	{
		m_scroll += glm::vec2(0.0f, 1.0f);
	}
	else if (key == Qt::Key_Down)
	{
		m_scroll += glm::vec2(0.0f, -1.0f);
	}

	update();
}


QString GLWidget::loadShader(QString filename)
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

void GLWidget::loadTexture(QImage* texture)
{
	makeCurrent();

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
	for (int j=0; j < height; j++)
	{
		QRgb *scan = (QRgb*)texture->scanLine(j);
		
		for (int i=0; i < width; i++)
		{
			int r = qRed(scan[i]);
			int g = qGreen(scan[i]);
			int b = qBlue(scan[i]);
			int a = qAlpha(scan[i]);

			pixels[index+0] = r;
			pixels[index+1] = g;
			pixels[index+2] = b;
			pixels[index+3] = a;
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

	delete [] pixels;

	doneCurrent();
}

void GLWidget::initializeGL()
{
	QString level_vs_file = loadShader("level_vs.glsl");
	QString level_fs_file = loadShader("level_fs.glsl");
	QString level_coll_fs_file = loadShader("level_coll_fs.glsl");
	QString grid_vs_file = loadShader("grid_vs.glsl");
	QString grid_fs_file = loadShader("grid_fs.glsl");

	m_level_program = new QGLShaderProgram(this);
	m_level_program->addShaderFromSourceCode(QGLShader::Vertex, level_vs_file);
	m_level_program->addShaderFromSourceCode(QGLShader::Fragment, level_fs_file);
	m_level_program->link();

	QString error = m_level_program->log();
	std::string errors = error.toStdString();

	m_level_shader.position		= m_level_program->attributeLocation("a_position");
	m_level_shader.tex_coord	= m_level_program->attributeLocation("a_texcoord");
	m_level_shader.color		= m_level_program->attributeLocation("a_color");
	m_level_shader.location		= m_level_program->uniformLocation("v_location");
	m_level_shader.scale		= m_level_program->uniformLocation("v_scale");
	m_level_shader.vp_matrix	= m_level_program->uniformLocation("m_vp_matrix");
	m_level_shader.rot_matrix	= m_level_program->uniformLocation("m_rot_matrix");


	m_grid_program = new QGLShaderProgram(this);
	m_grid_program->addShaderFromSourceCode(QGLShader::Vertex, grid_vs_file);
	m_grid_program->addShaderFromSourceCode(QGLShader::Fragment, grid_fs_file);
	m_grid_program->link();

	error = m_grid_program->log();
	errors = error.toStdString();

	m_grid_shader.position		= m_grid_program->attributeLocation("a_position");
	m_grid_shader.color			= m_grid_program->attributeLocation("a_color");
	m_grid_shader.location		= m_grid_program->uniformLocation("v_location");
	m_grid_shader.scale			= m_grid_program->uniformLocation("v_scale");
	m_grid_shader.vp_matrix		= m_grid_program->uniformLocation("m_vp_matrix");
	m_grid_shader.rot_matrix	= m_grid_program->uniformLocation("m_rot_matrix");
}


void GLWidget::renderDrawPolyMode(QPainter& painter)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());

	glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

	bool valid_convex = m_polydef->convexTest(mouse_lp);

	QColor fillcolor;
	QColor linecolor;
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
		painter.drawPolygon(ppoints, num_points+1);
	}

	// points
	for (int i=0; i < num_points; i++)
	{
		glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));

		painter.fillRect(p.x-3, p.y-2, 5, 5, m_point_color);
	}

	// draw line to mouse pointer if not at max
	if (num_points > 0 && num_points < m_polydef->getCapacity())
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

void GLWidget::renderDrawRectMode(QPainter& painter)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());
	glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

	if (m_snap_grid)
		mouse_lp = snapToGrid(mouse_lp);

	if (m_dragging)
	{
		glm::vec2 start_p = toScreenCoords(m_drag_point);
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

void GLWidget::renderEditMode(QPainter& painter)
{
	if (m_selected_object >= 0)
	{
		int num_points = m_polydef->getNumPoints();

		if (num_points >= 2)
		{
			// polygon
			QPointF ppoints[8];
			for (int i=0; i < num_points; i++)
			{
				glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));
				ppoints[i].setX(p.x);
				ppoints[i].setY(p.y);
			}

			painter.setPen(QColor(224, 224, 0));
			painter.setBrush(QBrush(QColor(160, 160, 0, 128)));
			painter.drawConvexPolygon(ppoints, num_points);
		}

		// points
		for (int i=0; i < num_points; i++)
		{
			glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));

			QColor color;

			if (i == m_selected_point)
				color = QColor(224, 0, 0);
			else
				color = QColor(224, 224, 0);
	
			painter.fillRect(p.x-2, p.y-2, 5, 5, color);
		}
	}
}

void GLWidget::renderMoveMode(QPainter& painter)
{
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());

	if (m_selected_object >= 0)
	{
		glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_p.x(), mouse_p.y()));

		if (m_snap_grid)
			mouse_lp = snapToGrid(mouse_lp);

		int num_points = m_polydef->getNumPoints();

		float delta_x = 0.0f;
		float delta_y = 0.0f;

		glm::vec2 delta = glm::vec2(0.0f, 0.0f);

		if (m_dragging)
		{
			delta = mouse_lp - m_drag_point;
		}

		if (num_points >= 2)
		{
			// polygon
			QPointF ppoints[8];
			for (int i=0; i < num_points; i++)
			{
				if (!m_dragging)
				{
					glm::vec2 p = toScreenCoords(m_polydef->getPoint(i));
					ppoints[i].setX(p.x);
					ppoints[i].setY(p.y);
				}
				else
				{
					glm::vec2 p;

					switch (m_opmode)
					{
						case MODE_MOVE:
						{
							p = toScreenCoords(m_polydef->getPoint(i) + delta);
							break;
						}
						case MODE_ROTATE:
						{
							float angle = makeRotationAngle(delta.x);
							glm::vec2 pp = m_polydef->getPoint(i);
							glm::vec2 dp = (pp - m_object_center);
							glm::vec2 tp = glm::rotate(dp, (float)(angle)) + m_object_center;
							p = toScreenCoords(tp);
							break;
						}
						case MODE_SCALE:
						{
							float scale = makeScalingScale(delta.x);
							glm::vec2 pp = m_polydef->getPoint(i);
							pp = m_object_center + ((pp - m_object_center) * scale);
							p = toScreenCoords(pp);
							break;
						}
					}

					ppoints[i].setX(p.x);
					ppoints[i].setY(p.y);
				}
			}

			painter.setPen(QColor(0, 224, 0));
			if (m_dragging)
				painter.setBrush(QBrush(QColor(0, 160, 0, 128)));
			else
				painter.setBrush(Qt::NoBrush);
			painter.drawConvexPolygon(ppoints, num_points);

			// points
			for (int i=0; i < num_points; i++)
			{
				painter.fillRect(ppoints[i].x()-2, ppoints[i].y()-2, 5, 5, QColor(0, 224, 0));
			}
		}
	}
}

void GLWidget::drawGLGrid()
{
	int w = width();
	int h = height();

	float aspect = (float)(w) / (float)(h);

	float grid_size = GRID_SIZE[m_grid_size];
	int num_x = (int)(((float)(LEVEL_VIS_WIDTH / GLWidget::ZOOM_LEVELS[m_zoom_level]) / grid_size) + 0.5f) + 1;
	int num_y = (int)((((float)(LEVEL_VIS_WIDTH / GLWidget::ZOOM_LEVELS[m_zoom_level]) / aspect) / grid_size) + 0.5f) + 1;

	if (num_x > 1000) num_x = 1000;
	if (num_y > 1000) num_y = 1000;

	glm::vec2 tl = toLevelCoords(glm::vec2(0, 0));
	glm::vec2 br = toLevelCoords(glm::vec2(w, h));

	glm::vec2 orig = snapToGrid(tl);

	int index = 0;

	float x = orig.x;
	float y = orig.y;

	float grid_color;

	for (int i=0; i < num_x; i++)
	{
		if (x == 0.0f)
			grid_color = 0.75f;
		else
			grid_color = 0.5f;

		m_grid_vb[index++] = x;
		m_grid_vb[index++] = tl.y;
		m_grid_vb[index++] = grid_color;
		m_grid_vb[index++] = x;
		m_grid_vb[index++] = br.y;
		m_grid_vb[index++] = grid_color;

		x += grid_size;
	}

	for (int i=0; i < num_y; i++)
	{
		if (y == 0.0f)
			grid_color = 0.75f;
		else
			grid_color = 0.5;

		m_grid_vb[index++] = tl.x;
		m_grid_vb[index++] = y;
		m_grid_vb[index++] = grid_color;
		m_grid_vb[index++] = br.x;
		m_grid_vb[index++] = y;
		m_grid_vb[index++] = grid_color;

		y += grid_size;
	}

	glDepthMask(GL_FALSE);

	assert(index >= 0 && index < 16384);

	m_grid_program->enableAttributeArray(m_grid_shader.position);
	m_grid_program->setAttributeArray(m_grid_shader.position, (GLfloat*)m_grid_vb, 2, 12);
	m_grid_program->enableAttributeArray(m_grid_shader.color);
	m_grid_program->setAttributeArray(m_grid_shader.color, GL_FLOAT, (GLfloat*)m_grid_vb+2, 1, 12);

	glDrawArrays(GL_LINES, 0, (num_x + num_y) * 2);

	m_grid_program->disableAttributeArray(m_grid_shader.position);
	m_grid_program->disableAttributeArray(m_grid_shader.color);

	glDepthMask(GL_TRUE);
}


void GLWidget::renderOtherObjects(QPainter& painter)
{
	int num_objs = m_level->numObjects();

	glm::vec2 tl = toLevelCoords(glm::vec2(0.0f, 0.0f));
	glm::vec2 br = toLevelCoords(glm::vec2(width(), height()));

	for (int i=0; i < num_objs; i++)
	{
		Level::Object* obj = m_level->getObject(i);
		Level::ObjectType type = obj->getType();

		glm::vec2 objmin = obj->getBoundingMin();
		glm::vec2 objmax = obj->getBoundingMax();

		bool render = m_display_filter & (1 << (type-1));

		if (objmax.x > tl.x &&
			objmax.y > tl.y &&
			objmin.x < br.x &&
			objmin.y < br.y &&
			render)
		{
			if (type == Level::OBJECT_TYPE_TRIGGER)
			{
				int num_points = obj->getNumPoints();
				if (num_points >= 2)
				{
					// polygon
					QPointF ppoints[9];
					for (int i=0; i < num_points; i++)
					{
						glm::vec2 p = toScreenCoords(obj->getPoint(i));
						ppoints[i].setX(p.x);
						ppoints[i].setY(p.y);
					}
					ppoints[num_points] = ppoints[0];

					QBrush brush = QBrush(QColor(0, 224, 224));

					painter.setPen(QPen(brush, 2, Qt::DashLine, Qt::FlatCap, Qt::RoundJoin));
					//painter.setPen(QColor(0,224,224));
					painter.setBrush(Qt::NoBrush);
					painter.drawPolygon(ppoints, num_points + 1);

					glm::vec2 tmin = toScreenCoords(objmin);
					glm::vec2 tmax = toScreenCoords(objmax);

					Level::Object::Param type = obj->getParam(0);

					painter.drawText(QRect(tmin.x, tmin.y, tmax.x-tmin.x, tmax.y-tmin.y),
									 Qt::AlignCenter|Qt::AlignVCenter, m_level->getTriggerName(type.i).c_str());

					Level::Object::Param angle = obj->getParam(1);
					float axisx = cos(angle.f * M_PI / 180.0);
					float axisy = sin(angle.f * M_PI / 180.0);

					glm::vec2 center = objmin + ((objmax - objmin) * 0.5f);
					glm::vec2 startp = toScreenCoords(center + (glm::vec2(axisx, axisy) * 2.0f));
					glm::vec2 endp = toScreenCoords(center + (glm::vec2(axisx, axisy) * 0.5f));

					painter.drawLine(startp.x, startp.y, endp.x, endp.y);
				}
			}
			else if (type == Level::OBJECT_TYPE_MOVER)
			{
				int num_points = obj->getNumPoints();
				if (num_points >= 2)
				{
					// polygon
					QPointF ppoints[9];
					for (int i=0; i < num_points; i++)
					{
						glm::vec2 p = toScreenCoords(obj->getPoint(i));
						ppoints[i].setX(p.x);
						ppoints[i].setY(p.y);
					}
					ppoints[num_points] = ppoints[0];

					QBrush brush = QBrush(QColor(224, 0, 224));

					painter.setPen(QPen(brush, 2, Qt::DashLine, Qt::FlatCap, Qt::RoundJoin));
					painter.setBrush(Qt::NoBrush);
					painter.drawPolygon(ppoints, num_points + 1);

					float angle = obj->getParam(0).f;

					float axisx = cos(angle * M_PI / 180.0);
					float axisy = sin(angle * M_PI / 180.0);

					float far_length;
					float near_length;
					if (type == Level::OBJECT_TYPE_MOVER)
					{
						near_length = 0.0f;
						far_length = obj->getParam(1).f;
					}
					else
					{
						near_length = 0.5f;
						far_length = 2.0f;
					}

					glm::vec2 center = objmin + ((objmax - objmin) * 0.5f);
					glm::vec2 startp = toScreenCoords(center+ (glm::vec2(axisx, axisy) * near_length));
					glm::vec2 endp = toScreenCoords(center + (glm::vec2(axisx, axisy) * far_length));

					painter.drawLine(startp.x, startp.y, endp.x, endp.y);
				}
			}
			else if (type == Level::OBJECT_TYPE_DESTRUCTIBLE)
			{
				int num_points = obj->getNumPoints();
				if (num_points >= 2)
				{
					// polygon
					QPointF ppoints[9];
					for (int i=0; i < num_points; i++)
					{
						glm::vec2 p = toScreenCoords(obj->getPoint(i));
						ppoints[i].setX(p.x);
						ppoints[i].setY(p.y);
					}
					ppoints[num_points] = ppoints[0];

					QBrush brush = QBrush(QColor(224, 150, 0));

					painter.setPen(QPen(brush, 2, Qt::DashLine, Qt::FlatCap, Qt::RoundJoin));
					painter.setBrush(Qt::NoBrush);
					painter.drawPolygon(ppoints, num_points + 1);
				}
			}
		}
	}
}

void GLWidget::renderTilemapBorders(QPainter& painter)
{
	float xs = (float)m_tilemap_xstart * m_tilemap_width;
	float xe = (float)m_tilemap_xend * m_tilemap_width;
	float ys = (float)m_tilemap_ystart * m_tilemap_height;
	float ye = (float)m_tilemap_yend * m_tilemap_height;

	int screen_width = width();
	int screen_height = height();

	glm::vec2 screen_tl = toLevelCoords(glm::vec2(0.0f, 0.0f));
	glm::vec2 screen_br = toLevelCoords(glm::vec2(screen_width, screen_height));

	if (!(m_filter & 0x40))
		return;

	if (xs > screen_br.x) return;
	if (xe < screen_tl.x) return;
	if (ys > screen_br.y) return;
	if (ye < screen_tl.y) return;

	glm::vec2 border_tl = toScreenCoords(glm::vec2(xs, ys));
	glm::vec2 border_br = toScreenCoords(glm::vec2(xe, ye));

	QColor border_dark = QColor(100, 100, 100, 255);
	QColor border_light = QColor(128, 128, 128, 255);

	int bsize = 10;

	// left side
	if (border_tl.x >= 0.0f && border_tl.x < screen_width)
	{
		int by = border_tl.y;
		if (by < 0) by = 0;
		int bh = border_br.y - by;
		if (bh > screen_height) bh = screen_height;
		
		painter.fillRect(border_tl.x - bsize, by - bsize, bsize, bh + (bsize * 2), border_dark);
		painter.fillRect(border_tl.x - bsize, by - bsize, bsize >> 1, bh + (bsize * 2), border_light);
	}
	// right side
	if (border_br.x >= 0.0f && border_br.x < screen_width)
	{
		int by = border_tl.y;
		if (by < 0) by = 0;
		int bh = border_br.y - by;
		if (bh > screen_height) bh = screen_height;
		
		painter.fillRect(border_br.x, by - bsize, bsize, bh + (bsize * 2), border_dark);
		painter.fillRect(border_br.x + (bsize >> 1), by - bsize, bsize >> 1, bh + (bsize * 2), border_light);
	}
	// top side
	if (border_tl.y >= 0.0f && border_tl.y < screen_height)
	{
		int bx = border_tl.x;
		if (bx < 0) bx = 0;
		int bw = border_br.x - bx;
		if (bw > screen_width) bw = screen_width;

		painter.fillRect(bx, border_tl.y - bsize, bw, bsize, border_dark);
		painter.fillRect(bx - bsize, border_tl.y - bsize, bw + (bsize * 2), bsize >> 1, border_light);
	}
	// bottom side
	if (border_br.y >= 0.0f && border_br.y < screen_height)
	{
		int bx = border_tl.x;
		if (bx < 0) bx = 0;
		int bw = border_br.x - bx;
		if (bw > screen_width) bw = screen_width;

		painter.fillRect(bx, border_br.y, bw, bsize, border_dark);
		painter.fillRect(bx - bsize, border_br.y + (bsize >> 1), bw + (bsize * 2), bsize >> 1, border_light);
	}
}

void GLWidget::renderTilemapExtras(QPainter& painter)
{
	QBrush brush = QBrush(QColor(224, 150, 0));

	painter.setPen(QColor(224, 224, 0));
	painter.setBrush(QBrush(QColor(160, 160, 0, 128)));

	int highlight_x = 1;
	int highlight_y = 0;

	int i = highlight_x;
	int j = highlight_y;

	int jj = j / 2;

	float xxs = (float)(m_tilemap_xstart + i) * m_tilemap_width;
	float xxe = (float)(m_tilemap_xstart + i + 1) * m_tilemap_width;
	float yys = (float)(m_tilemap_ystart + jj) * m_tilemap_height;
	float yye = (float)(m_tilemap_ystart + jj + 1) * m_tilemap_height;

	if (j & 1)
	{
		xxs += m_tilemap_width / 2;
		xxe += m_tilemap_width / 2;
		yys += m_tilemap_height / 2;
		yye += m_tilemap_height / 2;
	}

	QPoint pts[6];
	glm::vec2 tltl = toScreenCoords(glm::vec2(xxs, yys));
	glm::vec2 brbr = toScreenCoords(glm::vec2(xxe, yye));

	pts[0] = QPoint(tltl.x,
					tltl.y + (brbr.y - tltl.y) * (15.0 / 70.0));
	pts[1] = QPoint(tltl.x,
					tltl.y + (brbr.y - tltl.y) * (35.0 / 70.0));
	pts[2] = QPoint(tltl.x + (brbr.x - tltl.x) * 0.5,
					tltl.y + (brbr.y - tltl.y) * (50.0 / 70.0));
	pts[3] = QPoint(brbr.x,
					tltl.y + (brbr.y - tltl.y) * (35.0 / 70.0));
	pts[4] = QPoint(brbr.x,
					tltl.y + (brbr.y - tltl.y) * (15.0 / 70.0));
	pts[5] = QPoint(tltl.x + (brbr.x - tltl.x) * 0.5,
					tltl.y);

	painter.drawPolygon(pts, 6);
}



// render 
void GLWidget::paintGL()
{
	makeCurrent();

	QPainter painter;
	painter.begin(this);

	painter.beginNativePainting();

	// opengl scene rendering
	// --------------------------------------------------------------------------
//	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	qglClearColor(m_bgcolor);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	float aspect = (float)(width()) / (float)(height());
	float halfw = (float)((LEVEL_VIS_WIDTH / GLWidget::ZOOM_LEVELS[m_zoom_level]) / 2);
	float halfh = halfw / aspect;

	// setup matrices
	float level_vp_mat[4] = {1.0f / halfw, 0.0f,
							 0.0f, 1.0f / halfh };

	float level_rot_mat[4] = {1.0f, 0.0f,
							  0.0f, 1.0f};



	QMatrix2x2 vp_mat = QMatrix2x2(level_vp_mat);
	QMatrix2x2 rot_mat = QMatrix2x2(level_rot_mat);


	// grid
	if (m_enable_grid)
	{
		m_grid_program->bind();

		m_grid_program->setUniformValue(m_grid_shader.vp_matrix, vp_mat);
		m_grid_program->setUniformValue(m_grid_shader.rot_matrix, rot_mat);

		m_grid_program->setUniformValue(m_grid_shader.location, m_scroll.x, m_scroll.y);
		m_grid_program->setUniformValue(m_grid_shader.scale, 1.0f, 1.0f);

		drawGLGrid();
	}


	// other object types
	m_level_program->bind();

	m_level_program->setUniformValue(m_level_shader.vp_matrix, vp_mat);
	m_level_program->setUniformValue(m_level_shader.rot_matrix, rot_mat);
	m_level_program->setUniformValue(m_level_shader.location, m_scroll.x, m_scroll.y);
	m_level_program->setUniformValue(m_level_shader.scale, 1.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);


	// tilemap
	{
		float* geo = m_level->getTilemapVBO();
		int num_tris = m_level->numTilemapTris();

		// TODO filter

		bool render = true;
		if (render && num_tris > 0)
		{
			m_level_program->enableAttributeArray(m_level_shader.position);
			m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)geo, 3, sizeof(Tilemap::VBO));
			m_level_program->enableAttributeArray(m_level_shader.tex_coord);
			m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)geo+3, 2, sizeof(Tilemap::VBO));
			m_level_program->enableAttributeArray(m_level_shader.color);
			m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)geo + 20, 4, sizeof(Tilemap::VBO));

			glBindTexture(GL_TEXTURE_2D, m_base_tex);
			glDrawArrays(GL_TRIANGLES, 0, num_tris*3);

			m_level_program->disableAttributeArray(m_level_shader.position);
			m_level_program->disableAttributeArray(m_level_shader.tex_coord);
			m_level_program->disableAttributeArray(m_level_shader.color);
		}
	}


	for (int vbo = 0; vbo < Level::NUM_VBOS; vbo++)
	{
		float* static_geo = m_level->getVBO(vbo);
		int num_static_verts = m_level->numVBOVerts(vbo);

		bool render = false;
		switch (vbo)
		{
			case Level::VBO_DESTRUCTIBLE:	render = m_display_filter & (1 << (Level::OBJECT_TYPE_DESTRUCTIBLE - 1)); break;
			case Level::VBO_MOVER:			render = m_display_filter & (1 << (Level::OBJECT_TYPE_MOVER - 1)); break;
			case Level::VBO_ENEMY:			render = m_display_filter & (1 << (Level::OBJECT_TYPE_ENEMY - 1)); break;
		}

		if (render && num_static_verts > 0)
		{
			m_level_program->enableAttributeArray(m_level_shader.position);
			m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)static_geo, 3, sizeof(Level::VBO));
			m_level_program->enableAttributeArray(m_level_shader.tex_coord);
			m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)static_geo+3, 2, sizeof(Level::VBO));
			m_level_program->enableAttributeArray(m_level_shader.color);
			m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)static_geo + 20, 4, sizeof(Level::VBO));

			glBindTexture(GL_TEXTURE_2D, m_base_tex);
			glDrawArrays(GL_TRIANGLES, 0, num_static_verts);

			m_level_program->disableAttributeArray(m_level_shader.position);
			m_level_program->disableAttributeArray(m_level_shader.tex_coord);
			m_level_program->disableAttributeArray(m_level_shader.color);
		}
	}


	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);

	painter.endNativePainting();

	// 2d painter stuff
	// ---------------------------------------------------------------------------
	
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());


	renderTilemapBorders(painter);
	renderOtherObjects(painter);

	renderTilemapExtras(painter);


	// draw visbox
	if (m_enable_visbox)
	{
		glm::vec2 tl = toLevelCoords(glm::vec2(0, 0));
		glm::vec2 br = toLevelCoords(glm::vec2(width(), height()));

		glm::vec2 center = tl + ((br - tl) * 0.5f);

		glm::vec2 p1 = toScreenCoords(glm::vec2(center.x - (m_visbox_width * 0.5f), center.y - (m_visbox_height * 0.5f)));
		glm::vec2 p2 = toScreenCoords(glm::vec2(center.x + (m_visbox_width * 0.5f), center.y + (m_visbox_height * 0.5f)));

		painter.setPen(QColor(255, 255, 255, 192));
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(p1.x, p1.y, p2.x-p1.x, p2.y-p1.y);
	}


	switch (m_opmode)
	{
		case MODE_DRAW_POLY:	renderDrawPolyMode(painter); break;
		case MODE_DRAW_RECT:	renderDrawRectMode(painter); break;
		case MODE_SELECT:		renderEditMode(painter); break;
		case MODE_MOVE:			renderMoveMode(painter); break;
		case MODE_ROTATE:		renderMoveMode(painter); break;
		case MODE_SCALE:		renderMoveMode(painter); break;
	}


	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		char str[100];
		sprintf_s(str, "ERROR: %d", error);
		m_error_string = str;
	}

	if (m_error_string.length() > 0)
	{
		painter.setPen(QColor(0, 0, 0));
		painter.drawText(9, 32, m_error_string.c_str());
		painter.setPen(QColor(255, 255, 255));
		painter.drawText(8, 31, m_error_string.c_str());
	}

	glm::vec2 mlp = toLevelCoords(glm::vec2(mouse_p.x(), mouse_p.y()));
	
	QString modetext;
	switch (m_opmode)
	{
		case MODE_SELECT:
		{
			modetext = "Select";
			modetext += tr(": X: %1, Y: %2").arg(mlp.x).arg(mlp.y);
			break;
		}
		case MODE_MOVE:
		{
			modetext = "Move";
			if (m_dragging)
				modetext += tr(": X: %1, Y: %2").arg(mlp.x - m_drag_point.x).arg(mlp.y - m_drag_point.y);
			break;
		}
		case MODE_ROTATE:
		{
			modetext = "Rotate";
			if (m_dragging)
			{
				float angle = makeRotationAngle(mlp.x - m_drag_point.x);
				modetext += tr(": %1 degrees").arg(angle);
			}
			break;
		}
		case MODE_SCALE:
		{
			modetext = "Scale";
			if (m_dragging)
			{
				float scale = makeScalingScale(mlp.x - m_drag_point.x) * 100.0f;
				modetext += tr(": %1%").arg(scale);
			}
			break;
		}
		case MODE_DRAW_POLY:
		{
			modetext = tr("Draw Poly: X: %1, Y: %2").arg(mlp.x).arg(mlp.y);
			break;
		}
		case MODE_DRAW_RECT:
		{
			modetext = tr("Draw Rect: X: %1, Y: %2").arg(mlp.x).arg(mlp.y);
			break;
		}
		case MODE_TILEMAP:
		{
			modetext = tr("Tilemap: X: %1, Y: %2").arg(mlp.x).arg(mlp.y);
			break;
		}
	}	

	painter.setPen(QColor(255, 0, 0));
	painter.drawText(8, 16, modetext);

	painter.end();

	doneCurrent();
}

void GLWidget::resizeGL(int width, int height)
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

void GLWidget::paintEvent(QPaintEvent *e)
{
	paintGL();
}

void GLWidget::enableGrid(bool enable)
{
	m_enable_grid = enable;

	update();
}

void GLWidget::setSnapGrid(bool enable)
{
	m_snap_grid = enable;

	update();
}

void GLWidget::enableVisbox(bool enable)
{
	m_enable_visbox = enable;

	update();
}

void GLWidget::configVisbox(float width, float height)
{
	m_visbox_width = width;
	m_visbox_height = height;

	update();
}