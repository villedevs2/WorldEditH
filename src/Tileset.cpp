#include "Tileset.h"

Tileset::Tileset(Tileset::EditCallback* edit_callback)
{
	m_edit_callback = edit_callback;
	m_cumulative_tile_id = 1;
}

Tileset::~Tileset()
{

}


int Tileset::insertTile(std::string name, PolygonDef* uv, unsigned int color, std::string type, unsigned int* thumb, int thumb_w, int thumb_h)
{
	Tile tile;
	for (int i = 0; i < uv->getNumPoints(); i++)
	{
		tile.points[i] = uv->getPoint(i);
	}

	/*
		 p6
	p1         p5
	p2         p4
		 p3
	*/

	tile.name = name;
	tile.id = m_cumulative_tile_id;
	m_cumulative_tile_id++;

	tile.color = color;
	tile.type = TILE_FLOOR;

	if (type == "floor")
	{
		tile.type = TILE_FLOOR;
	}
	else if (type == "wall")
	{
		tile.type = TILE_WALL;
	}
	else if (type == "tower")
	{
		tile.type = TILE_TOWER;
	}
	else if (type == "env")
	{
		tile.type = TILE_ENV;
	}

	tile.thumb_width = thumb_w;
	tile.thumb_height = thumb_h;
	tile.thumbnail = new unsigned int[thumb_w * thumb_h];

	for (int i = 0; i < thumb_w*thumb_h; i++)
	{
		tile.thumbnail[i] = thumb[i];
	}

	m_tiles.push_back(tile);

	m_edit_callback->tileAdded(m_tiles.size() - 1);

	return tile.id;
}

bool Tileset::removeTile(int id)
{
	int num_tiles = m_tiles.size();
	for (int i = 0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			m_tiles.erase(m_tiles.begin() + i);

			m_edit_callback->tileRemoved(i);
			return true;
		}
	}

	return false;
}

void Tileset::removeTiles()
{
	m_tiles.clear();
}

int Tileset::getNumTiles()
{
	return m_tiles.size();
}

Tileset::Tile* Tileset::getTile(int index)
{
	assert(index >= 0 && index < m_tiles.size());

	return &m_tiles.at(index);
}

Tileset::Tile* Tileset::getTileById(int id)
{
	int num_tiles = m_tiles.size();
	for (int i = 0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			return tile;
		}
	}
	return nullptr;
}

int Tileset::getTileIndexById(int id)
{
	int num_tiles = m_tiles.size();
	for (int i = 0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			return i;
		}
	}
	return -1;
}