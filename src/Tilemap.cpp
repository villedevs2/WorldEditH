#include "Tilemap.h"

Tilemap::Tilemap()
{
	m_tile_width = 1.0f;
	m_tile_height = 1.4f;

	m_cumulative_tile_id = 1;

	m_buckets = new Bucket*[(AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT)];
}

Tilemap::~Tilemap()
{
	if (m_buckets != nullptr)
		delete[] m_buckets;
}

void Tilemap::reset()
{
	removeTiles();

	int size = (AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT);
	for (int i = 0; i < size; i++)
	{
		m_buckets[i] = nullptr;
	}
}

float Tilemap::getTileWidth()
{
	return m_tile_width;
}

float Tilemap::getTileHeight()
{
	return m_tile_height;
}

void Tilemap::tesselateTile(int x, int y)
{
	const int num_tris = 4;

	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);

	Bucket* bucket = m_buckets[bin];
	if (bucket != nullptr)
	{
		VBO* vbo = bucket->tiles;

		int iy = y % BUCKET_HEIGHT;
		int ix = x % BUCKET_WIDTH;

		int tile = bucket->map[(iy * BUCKET_WIDTH) + ix] & TILE_MASK;

		int vb_index = ((iy * BUCKET_WIDTH) + ix) * num_tris;

		float tx1 = (float)(x)* m_tile_width;
		float tx2 = tx1 + m_tile_width;
		float ty1 = (float)(y) * (m_tile_height / 2);
		float ty2 = ty1 + (m_tile_height / 2);

		if (y & 1)
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
				vbo->degenTri(vb_index + i);
			}
		}
		else
		{
			Tile& tiledata = m_tiles.at(tile);

			/*
			glm::vec2 vl = tiledata.top_points[1] - tiledata.top_points[0];
			glm::vec2 vr = tiledata.top_points[2] - tiledata.top_points[3];
			glm::vec2 vt = tiledata.top_points[3] - tiledata.top_points[0];
			glm::vec2 vb = tiledata.top_points[2] - tiledata.top_points[1];
*/
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
					vbo->makeTri(vb_index + 0, p1, p6, p5, uv1, uv6, uv5, tiledata.color);
					vbo->makeTri(vb_index + 1, p1, p5, p4, uv1, uv5, uv4, tiledata.color);
					vbo->makeTri(vb_index + 2, p1, p4, p3, uv1, uv4, uv3, tiledata.color);
					vbo->makeTri(vb_index + 3, p1, p3, p2, uv1, uv3, uv2, tiledata.color);
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
					vbo->makeTri(vb_index + 0, p1, p6, p3, uv1, uv4, uv3, tiledata.color);
					vbo->makeTri(vb_index + 1, p1, p3, p2, uv1, uv3, uv2, tiledata.color);
					vbo->degenTri(vb_index + 2);
					vbo->degenTri(vb_index + 3);
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
					vbo->makeTri(vb_index + 0, p6, p5, p4, uv2, uv1, uv4, tiledata.color);
					vbo->makeTri(vb_index + 1, p6, p4, p3, uv2, uv4, uv3, tiledata.color);
					vbo->degenTri(vb_index + 2);
					vbo->degenTri(vb_index + 3);
					break;
				}
				case Tilemap::TILE_TOP:
				{
					/*
						/\
					   /__\
					*/
					vbo->makeTri(vb_index + 0, p1, p6, p5, uv1, uv3, uv2, tiledata.color);
					vbo->degenTri(vb_index + 1);
					vbo->degenTri(vb_index + 2);
					vbo->degenTri(vb_index + 3);
					break;
				}
				case Tilemap::TILE_BOTTOM:
				{
					/* ____
					   \  /
						\/
					*/
					vbo->makeTri(vb_index + 0, p2, p4, p3, uv1, uv3, uv2, tiledata.color);
					vbo->degenTri(vb_index + 1);
					vbo->degenTri(vb_index + 2);
					vbo->degenTri(vb_index + 3);
					break;
				}
				case Tilemap::TILE_MID:
				{
					/*______
					  |    |
					  |____|
					*/
					vbo->makeTri(vb_index + 0, p1, p5, p4, uv1, uv4, uv3, tiledata.color);
					vbo->makeTri(vb_index + 1, p1, p4, p2, uv1, uv3, uv2, tiledata.color);
					vbo->degenTri(vb_index + 2);
					vbo->degenTri(vb_index + 3);
					break;
				}

				default:
				{
					// make degen geo
					for (int i = 0; i < num_tris; i++)
					{
						vbo->degenTri(vb_index + i);
					}
					break;
				}
			}
		}
	}
}

float* Tilemap::getVBO(int bx, int by)
{
	int bin = (by * (AREA_WIDTH / BUCKET_WIDTH)) + bx;

	if (m_buckets[bin] != nullptr)
	{
		return (float*)m_buckets[bin]->tiles->getPointer();
	}
	else
	{
		return nullptr;
	}
}

int Tilemap::numTris(int bx, int by)
{
	int bin = (by * (AREA_WIDTH / BUCKET_WIDTH)) + bx;

	if (m_buckets[bin] != nullptr)
	{
		return m_buckets[bin]->tiles->getCapacity();
	}
	else
	{
		return 0;
	}
}

int Tilemap::get(int x, int y)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return m_buckets[bin]->map[(iy * BUCKET_WIDTH) + ix] & TILE_MASK;
	}
	else
	{
		return -1;
	}
}

int Tilemap::getZ(int x, int y)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return (m_buckets[bin]->map[(iy * BUCKET_WIDTH) + ix] & Z_MASK) >> Z_SHIFT;
	}
	else
	{
		return -1;
	}
}

void Tilemap::edit(int x, int y, int tile)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);

	if (m_buckets[bin] == nullptr)
	{
		// TODO allocate
		allocBucket(bin);
	}

	int ix = x % BUCKET_WIDTH;
	int iy = y % BUCKET_HEIGHT;

	int index = (iy * BUCKET_WIDTH) + ix;

	m_buckets[bin]->map[index] &= ~TILE_MASK;
	m_buckets[bin]->map[index] |= tile & TILE_MASK;

	// if empty tile, reset Z as well
	if (tile == TILE_EMPTY)
	{
		m_buckets[bin]->map[index] &= ~Z_MASK;
	}

	// mark tile
	if (tile != TILE_EMPTY)
		m_buckets[bin]->coverage |= uint64_t(1) << index;
	else
		m_buckets[bin]->coverage &= ~(uint64_t(1) << index);

	tesselateTile(x, y);

	// deallocate if coverage == 0
	if (m_buckets[bin]->coverage == 0)
		deallocBucket(bin);
}

void Tilemap::editZ(int x, int y, int z)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);

	if (m_buckets[bin] != nullptr)		// do nothing if not mapped
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;

		int index = (iy * BUCKET_WIDTH) + ix;

		m_buckets[bin]->map[index] &= ~Z_MASK;
		m_buckets[bin]->map[index] |= (z << Z_SHIFT) & Z_MASK;
	}
}

unsigned int Tilemap::getRaw(int x, int y)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return m_buckets[bin]->map[(iy * BUCKET_WIDTH) + ix];
	}
	else
	{
		return TILE_EMPTY;
	}
}

void Tilemap::editRaw(int x, int y, unsigned int data)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		m_buckets[bin]->map[(iy * BUCKET_WIDTH) + ix] = data;
		tesselateTile(x, y);
	}
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