#include "Tilemap.h"

Tilemap::Tilemap()
{
	m_vb = nullptr;
	m_map = nullptr;

	m_tile_width = 1.0f;
	m_tile_height = 1.4f;

	resize(50, 50);

	m_cumulative_tile_id = 1;
}

Tilemap::~Tilemap()
{
	if (m_map != nullptr)
		delete [] m_map;

	if (m_vb != nullptr)
		delete m_vb;
}

void Tilemap::reset()
{
	memset(m_map, TILE_EMPTY | 0, sizeof(int) * m_width * m_height);
	removeTiles();
	tesselateAll();
}

int Tilemap::getWidth()
{
	return m_width;
}

int Tilemap::getHeight()
{
	return m_height;
}

float Tilemap::getTileWidth()
{
	return m_tile_width;
}

float Tilemap::getTileHeight()
{
	return m_tile_height;
}

void Tilemap::resize(int width, int height)
{
	if (m_map != NULL)
		delete[] m_map;

	m_map = new unsigned int[width * height];

	memset(m_map, TILE_EMPTY | 0, sizeof(int) * width * height);

	m_width = width;
	m_height = height;

	if (m_vb != nullptr)
		delete m_vb;

	m_vb = new VBO(width * height * 4);
}

bool Tilemap::enlarge(int xleft, int xright, int ytop, int ybottom)
{
	int new_width = m_width + xleft + xright;
	int new_height = m_height + ytop + ybottom;

	if (xleft == 0 && xright == 0 && ytop == 0 && ybottom == 0)
		return false;

	unsigned int* new_map = new unsigned int[new_width * new_height];

	memset(new_map, TILE_EMPTY | 0, sizeof(int) * new_width * new_height);

	// copy old data
	for (int j=0; j < m_height; j++)
	{
		for (int i=0; i < m_width; i++)
		{
			int di = i + xleft;
			int dj = j + ytop;

			new_map[(dj * new_width) + di] = m_map[(j * m_width) + i];
		}
	}

	delete [] m_map;

	m_map = new_map;

	m_width = new_width;
	m_height = new_height;

	if (m_vb != nullptr)
		delete m_vb;

	m_vb = new VBO(new_width * new_height * 4);

	tesselateAll();

	return true;
}

bool Tilemap::shrink(int xleft, int xright, int ytop, int ybottom)
{
	int new_width = m_width + xleft + xright;
	int new_height = m_height + ytop + ybottom;

	if (xleft == 0 && xright == 0 && ytop == 0 && ybottom == 0)
		return false;

	unsigned int* new_map = new unsigned int[new_width * new_height];

	memset(new_map, TILE_EMPTY | 0, sizeof(int) * new_width * new_height);

	// copy old data
	for (int j = 0; j < new_height; j++)
	{
		for (int i = 0; i < new_width; i++)
		{
			int si = i - xleft;
			int sj = j - ytop;

			new_map[(j * new_width) + i] = m_map[(sj * m_width) + si];
		}
	}

	delete[] m_map;

	m_map = new_map;

	m_width = new_width;
	m_height = new_height;

	if (m_vb != nullptr)
		delete m_vb;

	m_vb = new VBO(new_width * new_height * 4);

	tesselateAll();

	return true;
	return true;
}

void Tilemap::tesselateAll()
{
	for (int j=0; j < m_height; j++)
	{
		for (int i=0; i < m_width; i++)
		{
			bool odd = j & 1;
			tesselateTile(i, j, odd);
		}
	}
}

void Tilemap::tesselateTile(int x, int y, bool odd)
{
	const int num_tris = 4;

	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	int tile = m_map[(y * m_width) + x] & TILE_MASK;

	int vb_index = ((y * m_width) + x) * num_tris;

	float tx1 = (float)(x) * m_tile_width;
	float tx2 = tx1 + m_tile_width;
	float ty1 = (float)(y) * (m_tile_height / 2);
	float ty2 = ty1 + (m_tile_height / 2);

	if (odd)
	{
		tx1 += m_tile_width / 2;
		tx2 += m_tile_width / 2;
	}

	float z = 100.0f;
	
	if (tile == TILE_EMPTY)
	{
		// make degen geo
		for (int i = 0; i < num_tris; i++)
		{
			m_vb->degenTri(vb_index + i);
		}
	}
	else
	{
		Tile& tiledata = m_tiles.at(tile);

		glm::vec2 vl = tiledata.top_points[1] - tiledata.top_points[0];
		glm::vec2 vr = tiledata.top_points[2] - tiledata.top_points[3];
		glm::vec2 vt = tiledata.top_points[3] - tiledata.top_points[0];
		glm::vec2 vb = tiledata.top_points[2] - tiledata.top_points[1];

		glm::vec2 uv1 = tiledata.top_points[0];
		glm::vec2 uv2 = tiledata.top_points[1];
		glm::vec2 uv3 = tiledata.top_points[2];
		glm::vec2 uv4 = tiledata.top_points[3];
		glm::vec2 uv5 = tiledata.top_points[4];
		glm::vec2 uv6 = tiledata.top_points[5];

		/*
		     p6
		p1         p5
		p2         p4
		     p3
		*/

		glm::vec3 p1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), z);
		glm::vec3 p2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), z);
		glm::vec3 p3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), z);
		glm::vec3 p4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), z);
		glm::vec3 p5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), z);
		glm::vec3 p6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, z);

		switch (tiledata.type)
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
				m_vb->makeTri(vb_index + 0, p1, p6, p5, uv1, uv6, uv5, tiledata.color);
				m_vb->makeTri(vb_index + 1, p1, p5, p4, uv1, uv5, uv4, tiledata.color);
				m_vb->makeTri(vb_index + 2, p1, p4, p3, uv1, uv4, uv3, tiledata.color);
				m_vb->makeTri(vb_index + 3, p1, p3, p2, uv1, uv3, uv2, tiledata.color);
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
				m_vb->makeTri(vb_index + 0, p1, p6, p3, uv1, uv4, uv3, tiledata.color);
				m_vb->makeTri(vb_index + 1, p1, p3, p2, uv1, uv3, uv2, tiledata.color);
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
				m_vb->makeTri(vb_index + 0, p6, p5, p4, uv2, uv1, uv4, tiledata.color);
				m_vb->makeTri(vb_index + 1, p6, p4, p3, uv2, uv4, uv3, tiledata.color);
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
				m_vb->makeTri(vb_index + 0, p1, p6, p5, uv1, uv3, uv2, tiledata.color);
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
				m_vb->makeTri(vb_index + 0, p2, p4, p3, uv1, uv3, uv2, tiledata.color);
				m_vb->degenTri(vb_index + 1);
				m_vb->degenTri(vb_index + 2);
				m_vb->degenTri(vb_index + 3);
				break;
			}
			case Tilemap::TILE_MID:
			{
				/*______
			      |    |
			      |____|
				*/
				m_vb->makeTri(vb_index + 0, p1, p5, p4, uv1, uv4, uv3, tiledata.color);
				m_vb->makeTri(vb_index + 1, p1, p4, p2, uv1, uv3, uv2, tiledata.color);
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
}

float* Tilemap::getVBO()
{
	return (float*)m_vb->getPointer();
}

int Tilemap::numTris()
{
	return m_vb->getCapacity();
}

int Tilemap::getTile(int x, int y)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	return m_map[(y * m_width) + x] & TILE_MASK;
}

int Tilemap::getZ(int x, int y)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	return (m_map[(y * m_width) + x] & Z_MASK) >> Z_SHIFT;
}

void Tilemap::edit(int x, int y, int tile)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	int index = (y * m_width) + x;

	m_map[index] &= ~TILE_MASK;
	m_map[index] |= tile & TILE_MASK;

	// if empty tile, reset Z as well
	if (tile == TILE_EMPTY)
	{
		m_map[index] &= ~Z_MASK;
	}

	bool odd = y & 1;
	tesselateTile(x, y, odd);
}

void Tilemap::editZ(int x, int y, int z)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	int index = (y * m_width) + x;

	m_map[index] &= ~Z_MASK;
	m_map[index] |= (z << Z_SHIFT) & Z_MASK;
}

unsigned int Tilemap::getRaw(int x, int y)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	return m_map[(y * m_width) + x];
}

void Tilemap::editRaw(int x, int y, unsigned int data)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	m_map[(y * m_width) + x] = data;
	bool odd = y & 1;
	tesselateTile(x, y, odd);
}

int Tilemap::insertTile(std::string name, PolygonDef* top, PolygonDef* side, unsigned int color, Tilemap::TileType type)
{
	Tile tile;
	for (int i=0; i < top->getNumPoints(); i++)
	{
		tile.top_points[i] = top->getPoint(i);
	}

	/*
	     p6
	p1         p5
	p2         p4
	     p3
	*/
	/*
	switch (type)
	{
		case TILE_FULL:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[1] = top->getPoint(1);
			tile.top_points[2] = top->getPoint(2);
			tile.top_points[3] = top->getPoint(3);
			tile.top_points[4] = top->getPoint(4);
			tile.top_points[5] = top->getPoint(5);
			break;
		}
		case TILE_LEFT:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[1] = top->getPoint(1);
			tile.top_points[2] = top->getPoint(2);
			tile.top_points[5] = top->getPoint(3);
			break;
		}
		case TILE_RIGHT:
		{
			tile.top_points[4] = top->getPoint(0);
			tile.top_points[5] = top->getPoint(1);
			tile.top_points[2] = top->getPoint(2);
			tile.top_points[3] = top->getPoint(3);
			break;
		}
		case TILE_TOP:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[4] = top->getPoint(1);
			tile.top_points[5] = top->getPoint(2);
			break;
		}
		case TILE_BOTTOM:
		{
			tile.top_points[1] = top->getPoint(0);
			tile.top_points[2] = top->getPoint(1);
			tile.top_points[3] = top->getPoint(2);
			break;
		}
		case TILE_MID:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[1] = top->getPoint(1);
			tile.top_points[3] = top->getPoint(2);
			tile.top_points[4] = top->getPoint(3);
			break;
		}
	}
	*/

	tile.side_bits = 0;

	switch (type)
	{
		case TILE_FULL:
		{
			tile.side_bits |= SIDE_LEFT;
			tile.side_bits |= SIDE_TOP_LEFT;
			tile.side_bits |= SIDE_TOP_RIGHT;
			tile.side_bits |= SIDE_RIGHT;
			tile.side_bits |= SIDE_BOT_RIGHT;
			tile.side_bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_LEFT:
		{
			tile.side_bits |= SIDE_LEFT;
			tile.side_bits |= SIDE_TOP_LEFT;
			tile.side_bits |= SIDE_BOT_LEFT;
			tile.side_bits |= SIDE_MID;
			break;
		}
		case TILE_RIGHT:
		{
			tile.side_bits |= SIDE_RIGHT;
			tile.side_bits |= SIDE_TOP_RIGHT;
			tile.side_bits |= SIDE_BOT_RIGHT;
			tile.side_bits |= SIDE_MID;
			break;
		}
		case TILE_TOP:
		{
			tile.side_bits |= SIDE_TOP_RIGHT;
			tile.side_bits |= SIDE_TOP_LEFT;
			break;
		}
		case TILE_BOTTOM:
		{
			tile.side_bits |= SIDE_BOT_RIGHT;
			tile.side_bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_MID:
		{
			tile.side_bits |= SIDE_LEFT;
			tile.side_bits |= SIDE_RIGHT;
			break;
		}
	}


	for (int i = 0; i < side->getNumPoints(); i++)
	{
		tile.side_points[i] = side->getPoint(i);
	}
	tile.name = name;
	tile.id = m_cumulative_tile_id;
	m_cumulative_tile_id++;

	tile.color = color;
	tile.type = type;

	m_tiles.push_back(tile);
	return tile.id;
}

bool Tilemap::removeTile(int id)
{
	int num_tiles = m_tiles.size();
	for (int i=0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			m_tiles.erase(m_tiles.begin() + i);
			return true;
		}
	}

	return false;
}

void Tilemap::removeTiles()
{
	m_tiles.clear();
}

int Tilemap::getNumTiles()
{
	return m_tiles.size();
}

const Tilemap::Tile* Tilemap::getTile(int index)
{
	assert(index >= 0 && index < m_tiles.size());

	return &m_tiles.at(index);
}

const Tilemap::Tile* Tilemap::getTileById(int id)
{
	int num_tiles = m_tiles.size();
	for (int i=0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			return tile;
		}
	}
	return nullptr;
}

int Tilemap::getTileIndexById(int id)
{
	int num_tiles = m_tiles.size();
	for (int i=0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			return i;
		}
	}
	return -1;
}