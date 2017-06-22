#include "Tilemap.h"

Tilemap::Tilemap()
{
	m_vb = NULL;

	m_x = 0;
	m_y = 0;
	m_width = 50;
	m_height = 50;

	m_config.xstart = 0;
	m_config.xend = 50;
	m_config.ystart = 0;
	m_config.yend = 50;
	m_config.tile_width = 1.0f;
	m_config.tile_height = 1.2f;

	m_map = new int[50 * 50];
	memset(m_map, -1, sizeof(int) * 50 * 50);

	resize(m_config);

	m_cumulative_tile_id = 1;
}

Tilemap::~Tilemap()
{
	if (m_map != NULL)
		delete [] m_map;

	if (m_vb != NULL)
		delete [] m_vb;
}

void Tilemap::reset()
{
	memset(m_map, -1, sizeof(int) * m_width * m_height);
	removeTiles();
	tesselateAll();
}

bool Tilemap::resize(const Tilemap::Config& config)
{
	return resize(config.xstart, config.ystart, config.xend, config.yend, config.tile_width, config.tile_height);
}

bool Tilemap::resize(int x1, int y1, int x2, int y2, float tile_width, float tile_height)
{
	int new_width = x2 - x1;
	int new_height = y2 - y1;
	int new_x = x1;
	int new_y = y1;


	if (m_x != x1 || m_y != y1 || m_width != new_width || m_height != new_height)
	{
		int* new_map = new int[new_width * new_height];

		memset(new_map, -1, sizeof(int) * new_width * new_height);

		// copy old data
		int sx_offset, sy_offset;
		int sx, sy, dx, dy;
		int copy_width, copy_height;

		if (x1 <= m_x)
		{
			sx = 0;
			dx = m_x - x1;
		}
		else
		{
			sx = -(m_x - x1);
			dx = 0;
		}

		if (y1 <= m_y)
		{
			sy = 0;
			dy = m_y - y1;
		}
		else
		{
			sy = -(m_y - y1);
			dy = 0;
		}

		if (new_width > m_width)
			copy_width = m_width - sx;		
		else
			copy_width = new_width - sx;
		
		if (new_height > m_height)
			copy_height = m_height - sy;
		else
			copy_height = new_height - sy;

		for (int j=0; j < copy_height; j++)
		{
			for (int i=0; i < copy_width; i++)
			{
				int si = i + sx;
				int sj = j + sy;
				int di = i + dx;
				int dj = j + dy;

				new_map[(dj * new_width) + di] = m_map[(sj * m_width) + si];
			}
		}

		delete [] m_map;

		m_map = new_map;
	}

	m_x = new_x;
	m_y = new_y;
	m_width = new_width;
	m_height = new_height;

	m_tile_width = tile_width;
	m_tile_height = tile_height;

	if (m_vb != NULL)
	{
		delete [] m_vb;
		m_vb = NULL;
	}

	m_vb = new VBO[new_width * new_height * 6];

	tesselateAll();

	m_config.xstart = x1;
	m_config.xend = x2;
	m_config.ystart = y1;
	m_config.yend = y2;
	m_config.tile_width = tile_width;
	m_config.tile_height = tile_height;

	return true;
}

void Tilemap::tesselateAll()
{
	for (int j=0; j < m_height; j++)
	{
		for (int i=0; i < m_width; i++)
		{
			tesselateTile(i, j);
		}
	}
}

void Tilemap::tesselateTile(int x, int y)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	int tile = m_map[(y * m_width) + x];

	int vb_index = ((y * m_width) + x) * 6;

	float tx1 = (float)(x + m_x) * m_tile_width;
	float tx2 = tx1 + m_tile_width;
	float ty1 = (float)(y + m_y) * m_tile_height;
	float ty2 = ty1 + m_tile_height;

	float z = 100.0f;
	
	if (tile == -1)
	{
		// make degen geo
		m_vb[vb_index + 0].pos = glm::vec3(0, 0, 0);		m_vb[vb_index + 0].uv = glm::vec2(0, 0);		m_vb[vb_index + 0].color = 0;
		m_vb[vb_index + 1].pos = glm::vec3(0, 0, 0);		m_vb[vb_index + 1].uv = glm::vec2(0, 0);		m_vb[vb_index + 1].color = 0;
		m_vb[vb_index + 2].pos = glm::vec3(0, 0, 0);		m_vb[vb_index + 2].uv = glm::vec2(0, 0);		m_vb[vb_index + 2].color = 0;
		m_vb[vb_index + 3].pos = glm::vec3(0, 0, 0);		m_vb[vb_index + 3].uv = glm::vec2(0, 0);		m_vb[vb_index + 3].color = 0;
		m_vb[vb_index + 4].pos = glm::vec3(0, 0, 0);		m_vb[vb_index + 4].uv = glm::vec2(0, 0);		m_vb[vb_index + 4].color = 0;
		m_vb[vb_index + 5].pos = glm::vec3(0, 0, 0);		m_vb[vb_index + 5].uv = glm::vec2(0, 0);		m_vb[vb_index + 5].color = 0;
	}
	else
	{
		Tile& tiledata = m_tiles.at(tile);

		m_vb[vb_index + 0].pos = glm::vec3(tx1, ty1, z);	m_vb[vb_index + 0].uv = tiledata.points[0];		m_vb[vb_index + 0].color = tiledata.color;
		m_vb[vb_index + 1].pos = glm::vec3(tx1, ty2, z);	m_vb[vb_index + 1].uv = tiledata.points[1];		m_vb[vb_index + 1].color = tiledata.color;
		m_vb[vb_index + 2].pos = glm::vec3(tx2, ty2, z);	m_vb[vb_index + 2].uv = tiledata.points[2];		m_vb[vb_index + 2].color = tiledata.color;
		m_vb[vb_index + 3].pos = glm::vec3(tx1, ty1, z);	m_vb[vb_index + 3].uv = tiledata.points[0];		m_vb[vb_index + 3].color = tiledata.color;
		m_vb[vb_index + 4].pos = glm::vec3(tx2, ty2, z);	m_vb[vb_index + 4].uv = tiledata.points[2];		m_vb[vb_index + 4].color = tiledata.color;
		m_vb[vb_index + 5].pos = glm::vec3(tx2, ty1, z);	m_vb[vb_index + 5].uv = tiledata.points[3];		m_vb[vb_index + 5].color = tiledata.color;
	}
}

float* Tilemap::getVBO()
{
	return (float*)m_vb;
}

int Tilemap::numTris()
{
	return m_width * m_height * 2;
}

int Tilemap::get(int x, int y)
{
	int tx = x - m_x;
	int ty = y - m_y;

	assert(tx >= 0 && tx < m_width);
	assert(ty >= 0 && ty < m_height);

	return m_map[(ty * m_width) + tx];
}

void Tilemap::edit(int x, int y, int tile)
{
	int tx = x - m_x;
	int ty = y - m_y;

	assert(tx >= 0 && tx < m_width);
	assert(ty >= 0 && ty < m_height);

	m_map[(ty * m_width) + tx] = tile;

	tesselateTile(tx, ty);
}

const Tilemap::Config& Tilemap::getConfig()
{
	return m_config;
}


int Tilemap::insertTile(std::string name, glm::vec2* points, unsigned int color)
{
	Tile tile;
	for (int i=0; i < 4; i++)
	{
		tile.points[i] = points[i];
	}
	tile.name = name;
	tile.id = m_cumulative_tile_id;
	m_cumulative_tile_id++;

	tile.color = color;

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

const Tilemap::Tile& Tilemap::getTile(int index)
{
	assert(index >= 0 && index < m_tiles.size());

	return m_tiles.at(index);
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