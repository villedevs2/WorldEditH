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

	m_tile_selx = -1;
	m_tile_sely = -1;
	m_tile_basez = 0.0f;

	m_tile_brush = -1;

	m_create_poly_color = 0xffffffff;

	m_edited_tilemap = Level::TILEMAP_NORMAL;

	reset();

	this->setMouseTracking(true);
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

	m_edgedata.clear();

	m_tile_selx = -1;
	m_tile_sely = -1;

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
		case MODE_TILE_ZEDIT:
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

void GLWidget::setTileBrush(int tile)
{
	assert(tile >= -1 && tile < m_level->getTileset()->getNumTiles());

	m_tile_brush = tile;
}

void GLWidget::setTileBaseZ(float z)
{
	m_tile_basez = z;
}


void GLWidget::selectTilemap(Level::TilemapType type)
{
	m_edited_tilemap = type;
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



void GLWidget::tilemapDraw(Level::TilemapType map)
{
	if (m_tile_selx >= 0 &&
		m_tile_sely >= 0 &&
		m_tile_selx < Tilemap::AREA_WIDTH &&
		m_tile_sely < Tilemap::AREA_HEIGHT)
	{
		int brush = m_tile_brush;
		if (brush < 0)
			brush = Tilemap::TILE_EMPTY;

		Tilemap* tilemap = m_level->getTilemap(map);

		tilemap->edit(m_tile_selx, m_tile_sely, brush);
		tilemap->editZ(m_tile_selx, m_tile_sely, m_tile_basez);
//		emit onTileUpdate(map, m_tile_selx, m_tile_sely);
	}
}

void GLWidget::tilemapZDraw(Level::TilemapType map)
{
	if (m_tile_selx >= 0 &&
		m_tile_sely >= 0 &&
		m_tile_selx < Tilemap::AREA_WIDTH &&
		m_tile_sely < Tilemap::AREA_HEIGHT)
	{
		Tilemap* tilemap = m_level->getTilemap(map);

		tilemap->editZ(m_tile_selx, m_tile_sely, m_tile_basez);
//		emit onTileUpdate(map, m_tile_selx, m_tile_sely);
	}
}

void GLWidget::tilemapZEdit(Level::TilemapType map, int zmod)
{
	if (m_tile_selx >= 0 &&
		m_tile_sely >= 0 &&
		m_tile_selx < Tilemap::AREA_WIDTH &&
		m_tile_sely < Tilemap::AREA_HEIGHT)
	{
		Tilemap* tilemap = m_level->getTilemap(map);

		int z = tilemap->getZ(m_tile_selx, m_tile_sely);
		z += zmod;
		if (z < 0)
			z = 0;
		if (z > Tilemap::Z_MAX)
			z = Tilemap::Z_MAX;

		tilemap->editZ(m_tile_selx, m_tile_sely, z);
//		emit onTileUpdate(map, m_tile_selx, m_tile_sely);
	}
}

void GLWidget::updateTileDrawLocation(Level::TilemapType map, const glm::vec2& mouse_lp)
{
	Tilemap* tilemap = m_level->getTilemap(map);

	float tile_width = tilemap->getTileWidth();
	float tile_height = tilemap->getTileHeight();

	float tx1 = (float)(0) * tile_width;
	float ty1 = (float)(0) * (tile_height / 2);
	float tx2 = (float)(Tilemap::AREA_WIDTH) * tile_width;
	float ty2 = (float)(Tilemap::AREA_HEIGHT) * (tile_height / 2);

	int prev_x = m_tile_selx;
	int prev_y = m_tile_sely;

	if (mouse_lp.x >= tx1 && mouse_lp.x < tx2 &&
		mouse_lp.y >= ty1 && mouse_lp.y < ty2)
	{

		int block_x = (int)(mouse_lp.x / tile_width);
		int block_y = (int)(mouse_lp.y / (tile_height / 2));

		float selx = (block_x * tile_width);
		float sely = (block_y * (tile_height / 2));

		float lx = selx;
		float mx = selx + (tile_width / 2);
		float rx = selx + tile_width;
		float vy1 = sely;
		float vy2 = sely + (tile_height * (15.0 / 70.0));
		float vy3 = sely + (tile_height * (50.0 / 70.0));

		m_tile_sely = block_y;
		if (m_tile_sely & 1)
		{
			/*
			 e1\-   -/e2
			   +\   /+
		         \ /
		          |
			     +|-
			     e3
			*/

			glm::vec2 e1 = glm::vec2(mx, vy2) - glm::vec2(lx, vy1);
			glm::vec2 e2 = glm::vec2(rx, vy1) - glm::vec2(mx, vy2);
			glm::vec2 e3 = glm::vec2(mx, vy3) - glm::vec2(mx, vy2);

			glm::vec2 n1 = glm::vec2(-e1.y, e1.x);
			glm::vec2 n2 = glm::vec2(-e2.y, e2.x);
			glm::vec2 n3 = glm::vec2(-e3.y, e3.x);

			glm::vec2 b = mouse_lp - glm::vec2(mx, vy2);

			float dot1 = glm::dot(n1, b);
			float dot2 = glm::dot(n2, b);
			float dot3 = glm::dot(n3, b);

			if (dot1 < 0 && dot2 < 0)
			{
				// up
				m_tile_selx = block_x;
				m_tile_sely = block_y - 1;
			}
			else if (dot1 >= 0 && dot3 >= 0)
			{
				// left
				m_tile_selx = block_x - 1;
			}
			///else if (dot2 >= 0 && dot3 < 0)
			else
			{
				// right
				m_tile_selx = block_x;
			}
		}
		else
		{
			glm::vec2 e1 = glm::vec2(mx, vy1) - glm::vec2(lx, vy2);
			glm::vec2 e2 = glm::vec2(rx, vy2) - glm::vec2(mx, vy1);

			glm::vec2 n1 = glm::vec2(-e1.y, e1.x);
			glm::vec2 n2 = glm::vec2(-e2.y, e2.x);

			glm::vec2 b1 = mouse_lp - glm::vec2(lx, vy2);
			glm::vec2 b2 = mouse_lp - glm::vec2(rx, vy2);

			float dot1 = glm::dot(n1, b1);
			float dot2 = glm::dot(n2, b2);

			if (dot1 < 0)
			{
				m_tile_selx = block_x - 1;
				m_tile_sely--;
			}
			else if (dot2 < 0)
			{
				m_tile_selx = block_x;
				m_tile_sely--;
			}
			else
			{
				m_tile_selx = block_x;
			}
		}
	}
	else
	{
		m_tile_selx = -1;
		m_tile_sely = -1;
	}

//	if (prev_x != m_tile_selx || prev_y != m_tile_sely)
//		emit onTileSelect(map, m_tile_selx, m_tile_sely);
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
						
						if (obj->pointInside(mouse_p, 0.25f))
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

						if (obj->pointInside(mouse_p, 0.25f))
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
		case MODE_TILE_ZEDIT:
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
						
					if (obj->pointInside(mouse_p, 0.25f))
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
		case MODE_TILE_ZEDIT:
		{
			if (event->button() & Qt::LeftButton)
			{
				glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_x, mouse_y));

				updateTileDrawLocation(m_edited_tilemap, mouse_lp);

				if (m_opmode == MODE_TILEMAP)
					tilemapDraw(m_edited_tilemap);
				else if (m_opmode == MODE_TILE_ZEDIT)
					tilemapZDraw(m_edited_tilemap);

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
				update();
			}			
			break;
		}

		case MODE_TILEMAP:
		case MODE_TILE_ZEDIT:
		{
			glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_x, mouse_y));

			int prev_x = m_tile_selx;
			int prev_y = m_tile_sely;

			updateTileDrawLocation(m_edited_tilemap, mouse_lp);

			if (m_tile_selx != prev_x || m_tile_sely != prev_y)
				update();

			if (m_tilemap_painting)
			{
				//glm::vec2 mouse_lp = toLevelCoords(glm::vec2(mouse_x, mouse_y));
				if (m_opmode == MODE_TILEMAP)
					tilemapDraw(m_edited_tilemap);
				else if (m_opmode == MODE_TILE_ZEDIT)
					tilemapZDraw(m_edited_tilemap);

				update();
			}
			break;
		}
	}

	if (m_panning)
	{
		glm::vec2 mouse_p = toLevelCoords(glm::vec2(mouse_x, mouse_y));
		glm::vec2 delta = (m_pan_point - mouse_p) * 0.4f;

		m_scroll = m_scroll_saved - delta;
		update();
	}	
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

		case MODE_TILE_ZEDIT:
		{
			if (key == Qt::Key_Q)
			{
				tilemapZEdit(m_edited_tilemap, 1);
			}
			else if (key == Qt::Key_A)
			{
				tilemapZEdit(m_edited_tilemap, -1);
			}
			break;
		}
	}

	if (key == Qt::Key_Home)
	{
		if (event->modifiers() & Qt::ControlModifier)		// CTRL + HOME = center to a player start point
		{
			// find a player start object
			int num_objects = m_level->numObjects();
			for (int i = 0; i < num_objects; i++)
			{
				Level::Object* obj = m_level->getObject(i);
				if (obj->getType() == Level::OBJECT_TYPE_TRIGGER)
				{
					Level::Object::Param param = obj->getParam(0);
					std::string trig_name = m_level->getTriggerName(param.i);
					if (trig_name == "Player start point")
					{
						glm::vec2 bmin = obj->getBoundingMin();
						glm::vec2 bmax = obj->getBoundingMax();
						glm::vec2 center = bmin + ((bmax - bmin) * 0.5f);

						glm::vec2 tl = toLevelCoords(glm::vec2(0.0f, 0.0f));
						glm::vec2 br = toLevelCoords(glm::vec2(width(), height()));

						float screen_width = br.x - tl.x;
						float screen_height = br.y - tl.y;

						m_scroll = -glm::vec2(center.x - (screen_width / 2), center.y - (screen_height / 2));
					}
				}
			}
		}
		else
		{
			// HOME = center to origin
			m_scroll = glm::vec2(0.0f, 0.0f);
		}
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

void GLWidget::loadEnvTexture(QImage** textures)
{
	static GLenum cube_faces[6] =
	{
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	};

	makeCurrent();

	if (glIsTexture(m_env_tex))
		glDeleteTextures(1, &m_env_tex);
	glGenTextures(1, &m_env_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_env_tex);

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);

	for (int tex = 0; tex < 6; tex++)
	{
		int width = textures[tex]->width();
		int height = textures[tex]->height();

		char *pixels = new char[width * height * 3];

		int index = 0;
		for (int j = 0; j < height; j++)
		{
			QRgb *scan = (QRgb*)textures[tex]->scanLine(j);

			for (int i = 0; i < width; i++)
			{
				int r = qRed(scan[i]);
				int g = qGreen(scan[i]);
				int b = qBlue(scan[i]);
				/*
				switch (tex)
				{
				case 0: r = 255; g = 0; b = 0; break;
				case 1: r = 255; g = 255; b = 0; break;
				case 2: r = 0; g = 255; b = 0; break;
				case 3: r = 0; g = 0; b = 255; break;
				case 4: r = 0; g = 255; b = 255; break;
				case 5: r = 255; g = 0; b = 255; break;
				}
				*/

				pixels[index + 0] = r;
				pixels[index + 1] = g;
				pixels[index + 2] = b;

				index += 3;
			}
		}

		glTexImage2D(cube_faces[tex], 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		delete[] pixels;
	}

	doneCurrent();
}

void GLWidget::initializeGL()
{
	QString level_vs_file = loadShader("level_vs.glsl");
	QString level_fs_file = loadShader("level_fs.glsl");
	QString grid_vs_file = loadShader("grid_vs.glsl");
	QString grid_fs_file = loadShader("grid_fs.glsl");
	QString hs_vs_file = loadShader("homestar_vs.glsl");
	QString hs_fs_file = loadShader("homestar_fs.glsl");
	QString selflum_vs_file = loadShader("selflum_vs.glsl");
	QString selflum_fs_file = loadShader("selflum_fs.glsl");
	QString reflect_vs_file = loadShader("reflect_vs.glsl");
	QString reflect_fs_file = loadShader("reflect_fs.glsl");

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


	m_3d_program = new QGLShaderProgram(this);
	m_3d_program->addShaderFromSourceCode(QGLShader::Vertex, hs_vs_file);
	m_3d_program->addShaderFromSourceCode(QGLShader::Fragment, hs_fs_file);
	m_3d_program->link();

	error = m_3d_program->log();
	errors = error.toStdString();

	m_3d_shader.position = m_3d_program->attributeLocation("a_position");
	m_3d_shader.tex_coord = m_3d_program->attributeLocation("a_texcoord");
	m_3d_shader.color = m_3d_program->attributeLocation("a_color");
	m_3d_shader.vp_matrix = m_3d_program->uniformLocation("m_vp_matrix");


	m_selflum_program = new QGLShaderProgram(this);
	m_selflum_program->addShaderFromSourceCode(QGLShader::Vertex, selflum_vs_file);
	m_selflum_program->addShaderFromSourceCode(QGLShader::Fragment, selflum_fs_file);
	m_selflum_program->link();

	error = m_selflum_program->log();
	errors = error.toStdString();

	m_selflum_shader.position = m_selflum_program->attributeLocation("a_position");
	m_selflum_shader.tex_coord = m_selflum_program->attributeLocation("a_texcoord");
	m_selflum_shader.normal = m_selflum_program->attributeLocation("a_normal");
	m_selflum_shader.color = m_selflum_program->attributeLocation("a_color");
	m_selflum_shader.vp_matrix = m_selflum_program->uniformLocation("m_vp_matrix");


	m_reflect_program = new QGLShaderProgram(this);
	m_reflect_program->addShaderFromSourceCode(QGLShader::Vertex, reflect_vs_file);
	m_reflect_program->addShaderFromSourceCode(QGLShader::Fragment, reflect_fs_file);
	m_reflect_program->link();

	error = m_reflect_program->log();
	errors = error.toStdString();

	m_reflect_shader.position = m_reflect_program->attributeLocation("a_position");
	m_reflect_shader.tex_coord = m_reflect_program->attributeLocation("a_texcoord");
	m_reflect_shader.normal = m_reflect_program->attributeLocation("a_normal");
	m_reflect_shader.color = m_reflect_program->attributeLocation("a_color");
	m_reflect_shader.vp_matrix = m_reflect_program->uniformLocation("m_vp_matrix");
	m_reflect_shader.v_matrix = m_reflect_program->uniformLocation("m_v_matrix");
	m_reflect_shader.cam_pos = m_reflect_program->uniformLocation("v_camera_pos");
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

		bool render = true;

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

void GLWidget::renderTilemapPointer(Level::TilemapType map, QPainter& painter)
{
	Tilemap* tilemap = m_level->getTilemap(map);

	QBrush brush = QBrush(QColor(224, 150, 0));

	painter.setPen(QColor(224, 224, 0));
	painter.setBrush(QBrush(QColor(160, 160, 0, 128)));

	float tile_width = tilemap->getTileWidth();
	float tile_height = tilemap->getTileHeight();

	if (m_tile_selx >= 0 &&
		m_tile_sely >= 0 &&
		m_tile_selx < Tilemap::AREA_WIDTH &&
		m_tile_sely < Tilemap::AREA_HEIGHT)
	{
		int highlight_x = m_tile_selx;
		int highlight_y = m_tile_sely;

		int i = highlight_x;
		int j = highlight_y;
		
		float xxs = (float)(i) * tile_width;
		float xxe = xxs + tile_width;
		float yys = (float)(j) * (tile_height/2);
		float yye = yys + tile_height;

		if (j & 1)
		{
			xxs += tile_width / 2;
			xxe += tile_width / 2;
		}

		Tileset::TileType brush_type = Tileset::TILE_FULL;

		if (m_tile_brush >= 0)
		{
			const Tileset::Tile* tile = m_level->getTileset()->getTile(m_tile_brush);
			brush_type = tile->type;
		}

		QPoint pts[6];
		glm::vec2 tltl = toScreenCoords(glm::vec2(xxs, yys));
		glm::vec2 brbr = toScreenCoords(glm::vec2(xxe, yye));


		float txl = tltl.x;
		float txm = tltl.x + (brbr.x - tltl.x) * 0.5f;
		float txr = brbr.x;
		float ty1 = tltl.y;
		float ty2 = tltl.y + (brbr.y - tltl.y) * (15.0 / 70.0);
		float ty3 = tltl.y + (brbr.y - tltl.y) * (35.0 / 70.0);
		float ty4 = tltl.y + (brbr.y - tltl.y) * (50.0 / 70.0);
		
		/*
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
		*/

		switch (brush_type)
		{
			case Tileset::TILE_FULL:
			{
				pts[0] = QPoint(txl, ty2);
				pts[1] = QPoint(txl, ty3);
				pts[2] = QPoint(txm, ty4);
				pts[3] = QPoint(txr, ty3);
				pts[4] = QPoint(txr, ty2);
				pts[5] = QPoint(txm, ty1);
				painter.drawPolygon(pts, 6);
				break;
			}
			case Tileset::TILE_LEFT:
			{
				pts[0] = QPoint(txl, ty2);
				pts[1] = QPoint(txl, ty3);
				pts[2] = QPoint(txm, ty4);
				pts[3] = QPoint(txm, ty1);
				painter.drawPolygon(pts, 4);
				break;
			}
			case Tileset::TILE_RIGHT:
			{
				pts[0] = QPoint(txm, ty1);
				pts[1] = QPoint(txm, ty4);
				pts[2] = QPoint(txr, ty3);
				pts[3] = QPoint(txr, ty2);
				painter.drawPolygon(pts, 4);
				break;
			}
			case Tileset::TILE_TOP:
			{
				pts[0] = QPoint(txl, ty2);
				pts[1] = QPoint(txr, ty2);
				pts[2] = QPoint(txm, ty1);
				painter.drawPolygon(pts, 3);
				break;
			}
			case Tileset::TILE_BOTTOM:
			{
				pts[0] = QPoint(txl, ty3);
				pts[1] = QPoint(txm, ty4);
				pts[2] = QPoint(txr, ty3);
				painter.drawPolygon(pts, 3);
				break;
			}
			case Tileset::TILE_MID:
			{
				pts[0] = QPoint(txl, ty2);
				pts[1] = QPoint(txl, ty3);
				pts[2] = QPoint(txr, ty3);
				pts[3] = QPoint(txr, ty2);
				painter.drawPolygon(pts, 4);
				break;
			}
			case Tileset::TILE_CORNER_TL:
			{
				pts[0] = QPoint(txm, ty1);
				pts[1] = QPoint(txl, ty2);
				pts[2] = QPoint(txl, ty3);
				painter.drawPolygon(pts, 3);
				break;
			}
			case Tileset::TILE_CORNER_TR:
			{
				pts[0] = QPoint(txm, ty1);
				pts[1] = QPoint(txr, ty3);
				pts[2] = QPoint(txr, ty2);
				painter.drawPolygon(pts, 3);
				break;
			}
			case Tileset::TILE_CORNER_BL:
			{
				pts[0] = QPoint(txl, ty2);
				pts[1] = QPoint(txl, ty3);
				pts[2] = QPoint(txm, ty4);
				painter.drawPolygon(pts, 3);
				break;
			}
			case Tileset::TILE_CORNER_BR:
			{
				pts[0] = QPoint(txm, ty4);
				pts[1] = QPoint(txr, ty3);
				pts[2] = QPoint(txr, ty2);
				painter.drawPolygon(pts, 3);
				break;
			}
		}
	}
}



void GLWidget::renderEdgeData(Level::TilemapType map, QPainter& painter)
{
	Tilemap* tilemap = m_level->getTilemap(map);

	int tm_width = Tilemap::AREA_WIDTH;
	int tm_height = Tilemap::AREA_HEIGHT;

	float tile_width = tilemap->getTileWidth();
	float tile_height = tilemap->getTileHeight();

	painter.setPen(QColor(255, 0, 0));
	painter.setBrush(QBrush(QColor(0, 0, 0, 0)));

	QPoint pp[1024];

	for (int loop = 0; loop < m_edgedata.size(); loop++)
	{
		std::vector<int> points = m_edgedata.at(loop);
		int num_points = points.size();
		if (num_points > 1023)
			num_points = 1023;

		for (int p = 0; p < num_points; p++)
		{
			int index = points[p];
			int x = index % tm_width;
			int y = index / tm_width;

			glm::vec2 point;

			point.x = (x * tile_width) / 2.0f;
			point.y = (y/2) * tile_height;
			
			if ((y & 1) == 0)
			{								
				if ((x & 1) == 0)
					point.y += tile_height * (15.0 / 70.0);
			}
			else
			{
				point.y += tile_height * (35.0 / 70.0);
				if ((x & 1) != 0)
					point.y += tile_height * (15.0 / 70.0);
			}

			glm::vec2 sp = toScreenCoords(point);

			pp[p] = QPoint((int)(sp.x + 0.5f), (int)(sp.y + 0.5f));
		}

		painter.drawPolygon(pp, num_points);
	}
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
	//glEnable(GL_CULL_FACE);
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
	/*
	{
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
		if (xe > (Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH))
			xe = Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH;
		if (ye > (Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT))
			ye = Tilemap::AREA_HEIGHT / Tilemap::BUCKET_HEIGHT;

		for (int j = ys; j < ye; j++)
		{
			for (int i = xs; i < xe; i++)
			{
				const Tilemap::Bucket* bucket = m_level->getTileBucket(i, j);
				if (bucket != nullptr)
				{
					float* geo = (float*)bucket->tiles->getPointer();
					int vbsize = bucket->tiles->getVertexSize();

					m_level_program->enableAttributeArray(m_level_shader.position);
					m_level_program->setAttributeArray(m_level_shader.position, (GLfloat*)geo, 3, vbsize);
					m_level_program->enableAttributeArray(m_level_shader.tex_coord);
					m_level_program->setAttributeArray(m_level_shader.tex_coord, (GLfloat*)geo + 3, 2, vbsize);
					m_level_program->enableAttributeArray(m_level_shader.color);
					m_level_program->setAttributeArray(m_level_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)geo + 20, 4, vbsize);

					glDrawArrays(GL_TRIANGLES, 0, Tilemap::BUCKET_WIDTH*Tilemap::BUCKET_HEIGHT*4*3);

					m_level_program->disableAttributeArray(m_level_shader.position);
					m_level_program->disableAttributeArray(m_level_shader.tex_coord);
					m_level_program->disableAttributeArray(m_level_shader.color);
				}
			}
		}
	}
	*/

	for (int vbo = 0; vbo < Level::NUM_VBOS; vbo++)
	{
		float* static_geo = m_level->getVBO(vbo);
		int num_static_verts = m_level->numVBOVerts(vbo);

		bool render = true;

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

	// 3d tilemap

	// TODO: render all
	{
		Tilemap* tilemap = m_level->getTilemap(Level::TILEMAP_NORMAL);

		glDisable(GL_CULL_FACE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_ALPHA_TEST);

		float aspect = (float)(width()) / (float)(height());
		float halfw = (float)((LEVEL_VIS_WIDTH / GLWidget::ZOOM_LEVELS[m_zoom_level]) / 2);
		float halfh = halfw / aspect;

		const double fov = 60.0;
		const double near_plane = 0.01;
		const double far_plane = 100.0;

		float size = near_plane * (float)tan((fov * M_PI / 180.0) / 2);

		float camera_distance = ((float)(LEVEL_VIS_WIDTH / GLWidget::ZOOM_LEVELS[m_zoom_level]) / 2) / tan((fov * M_PI / 180.0) / 2);

		glm::vec2 cam_pos = -m_scroll + glm::vec2(halfw, halfh);


		glm::vec3 pos = glm::vec3(cam_pos.x, cam_pos.y, camera_distance);
		glm::vec3 eye = glm::vec3(cam_pos.x, cam_pos.y, 0.0f);

		glm::mat4 camera_proj_matrix = glm::frustum<float>(-size, size, size / aspect, -size / aspect, near_plane, far_plane);
		glm::mat4 camera_view_matrix = glm::lookAt(pos, eye, glm::vec3(0.0f, 1.0f, 0.0));
		glm::mat4 camera_vp_matrix = camera_proj_matrix * camera_view_matrix;


		QMatrix4x4 vp_mat = QMatrix4x4(glm::value_ptr(camera_vp_matrix));
		QMatrix4x4 v_mat = QMatrix4x4(glm::value_ptr(camera_view_matrix));

		m_reflect_program->bind();

		m_reflect_program->setUniformValue(m_reflect_shader.vp_matrix, vp_mat);
		m_reflect_program->setUniformValue(m_reflect_shader.v_matrix, v_mat);

		m_reflect_program->setUniformValue(m_reflect_shader.cam_pos, QVector3D(pos.x, pos.y, pos.z));

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);


		glBindTexture(GL_TEXTURE_2D, m_base_tex);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_env_tex);

		glm::vec2 tilemap_tl = toLevelCoords(glm::vec2(0, 0));
		glm::vec2 tilemap_br = toLevelCoords(glm::vec2(width(), height()));

		tilemap_tl.x *= tilemap->getTileWidth();
		tilemap_tl.y *= tilemap->getTileHeight();
		tilemap_br.x *= tilemap->getTileWidth();
		tilemap_br.y *= tilemap->getTileHeight();

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
				const Tilemap::Bucket* bucket = tilemap->getTileBucket(i, j);
				if (bucket != nullptr)
				{
					float* geo = (float*)bucket->tiles->getPointer();
					int vbsize = bucket->tiles->getVertexSize();

					m_reflect_program->enableAttributeArray(m_reflect_shader.position);
					m_reflect_program->setAttributeArray(m_reflect_shader.position, (GLfloat*)geo, 3, vbsize);
					m_reflect_program->enableAttributeArray(m_reflect_shader.tex_coord);
					m_reflect_program->setAttributeArray(m_reflect_shader.tex_coord, (GLfloat*)geo + 3, 2, vbsize);
					m_reflect_program->enableAttributeArray(m_reflect_shader.normal);
					m_reflect_program->setAttributeArray(m_reflect_shader.normal, (GLfloat*)geo + 5, 3, vbsize);
					m_reflect_program->enableAttributeArray(m_reflect_shader.color);
					m_reflect_program->setAttributeArray(m_reflect_shader.color, GL_UNSIGNED_BYTE, (GLbyte*)geo + 32, 4, vbsize);

					glDrawArrays(GL_TRIANGLES, 0, Tilemap::BUCKET_WIDTH*Tilemap::BUCKET_HEIGHT * Tilemap::MAX_VERTS * 3);

					m_reflect_program->disableAttributeArray(m_reflect_shader.position);
					m_reflect_program->disableAttributeArray(m_reflect_shader.tex_coord);
					m_reflect_program->disableAttributeArray(m_reflect_shader.normal);
					m_reflect_program->disableAttributeArray(m_reflect_shader.color);
				}
			}
		}
	}





	glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);

	painter.endNativePainting();

	// 2d painter stuff
	// ---------------------------------------------------------------------------
	
	QPoint mouse_p = this->mapFromGlobal(QCursor::pos());


	renderOtherObjects(painter);

	renderTilemapPointer(m_edited_tilemap, painter);
	renderEdgeData(m_edited_tilemap, painter);


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
			/*
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

			painter.setPen(QColor(255, 255, 0));
			painter.drawText(8, 32, tr("XS: %1, YS: %2, XE: %3, YE: %4").arg(xs).arg(ys).arg(xe).arg(ye));
			painter.drawText(8, 48, tr("TLX: %1, TLY: %2, BRX: %3, BRY: %4").arg(tl.x).arg(tl.y).arg(br.x).arg(br.y));

			painter.setPen(QColor(0, 255, 0));
			//painter.drawText(8, 32, tr("XS: %1, YS: %2, XE: %3, YE: %4").arg(xs).arg(ys).arg(xe).arg(ye));
			for (int j = ys; j < ye; j++)
			{
				QString str = "";
				for (int i = xs; i < xe; i++)
				{
					str += tr("%1 ").arg((j * (Tilemap::AREA_WIDTH / Tilemap::BUCKET_WIDTH)) + i);
				}
				painter.drawText(8, 64 + ((j-ys) * 8), str);
			}
			*/

			modetext = tr("Tilemap: X: %1, Y: %2").arg(m_tile_selx).arg(m_tile_sely);
			break;
		}
		case MODE_TILE_ZEDIT:
		{
			modetext = tr("Tilemap Z Edit: X: %1, Y: %1").arg(m_tile_selx).arg(m_tile_sely);
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

void GLWidget::setEdgeData(std::vector<std::vector<int>> data)
{
	m_edgedata = data;

	update();
}