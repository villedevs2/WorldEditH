#include "Tileset.h"

Tileset::Tileset(Tileset::EditCallback* edit_callback)
{
	m_edit_callback = edit_callback;
	m_cumulative_tile_id = 1;
}

Tileset::~Tileset()
{

}


int Tileset::getSideBits(Tileset::TileType type)
{
	int bits = 0;

	switch (type)
	{
	case TILE_FULL:
	{
		bits |= SIDE_LEFT;
		bits |= SIDE_TOP_LEFT;
		bits |= SIDE_TOP_RIGHT;
		bits |= SIDE_RIGHT;
		bits |= SIDE_BOT_RIGHT;
		bits |= SIDE_BOT_LEFT;
		break;
	}
	case TILE_LEFT:
	{
		bits |= SIDE_LEFT;
		bits |= SIDE_TOP_LEFT;
		bits |= SIDE_BOT_LEFT;
		bits |= SIDE_MID;
		break;
	}
	case TILE_RIGHT:
	{
		bits |= SIDE_RIGHT;
		bits |= SIDE_TOP_RIGHT;
		bits |= SIDE_BOT_RIGHT;
		bits |= SIDE_MID;
		break;
	}
	case TILE_TOP:
	{
		bits |= SIDE_TOP_RIGHT;
		bits |= SIDE_TOP_LEFT;
		break;
	}
	case TILE_BOTTOM:
	{
		bits |= SIDE_BOT_RIGHT;
		bits |= SIDE_BOT_LEFT;
		break;
	}
	case TILE_MID:
	{
		bits |= SIDE_LEFT;
		bits |= SIDE_RIGHT;
		break;
	}
	case TILE_CORNER_TL:
	{
		bits |= SIDE_LEFT;
		bits |= SIDE_TOP_LEFT;
		break;
	}
	case TILE_CORNER_TR:
	{
		bits |= SIDE_RIGHT;
		bits |= SIDE_TOP_RIGHT;
		break;
	}
	case TILE_CORNER_BL:
	{
		bits |= SIDE_LEFT;
		bits |= SIDE_BOT_LEFT;
		break;
	}
	case TILE_CORNER_BR:
	{
		bits |= SIDE_RIGHT;
		bits |= SIDE_BOT_RIGHT;
		break;
	}
	}

	return bits;
}


int Tileset::insertTile(std::string name, PolygonDef* top, PolygonDef* side, unsigned int color,
	Tileset::TileType type,
	Tileset::TopType top_type,
	Tileset::ShadingType shading_type,
	float top_height, unsigned int* thumb, int thumb_w, int thumb_h)
{
	Tile tile;
	for (int i = 0; i < top->getNumPoints(); i++)
	{
		tile.top_points[i] = top->getPoint(i);
	}

	/*
		 p6
	p1         p5
	p2         p4
		 p3
	*/

	tile.side_bits = getSideBits(type);

	for (int i = 0; i < side->getNumPoints(); i++)
	{
		tile.side_points[i] = side->getPoint(i);
	}
	tile.name = name;
	tile.id = m_cumulative_tile_id;
	m_cumulative_tile_id++;

	tile.color = color;
	tile.type = type;
	tile.top_type = top_type;

	tile.top_height = top_height;

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

int Tileset::replaceTile(int index, std::string name, PolygonDef* top, PolygonDef* side, unsigned int color,
						Tileset::TileType type,
						Tileset::TopType top_type,
						Tileset::ShadingType shading_type,
						float top_height, unsigned int* thumb, int thumb_w, int thumb_h)
{
	Tile* tile = &m_tiles.at(index);
	for (int i = 0; i < top->getNumPoints(); i++)
	{
		tile->top_points[i] = top->getPoint(i);
	}

	tile->side_bits = getSideBits(type);

	for (int i = 0; i < side->getNumPoints(); i++)
	{
		tile->side_points[i] = side->getPoint(i);
	}
	tile->name = name;

	tile->color = color;
	tile->type = type;
	tile->top_type = top_type;

	tile->top_height = top_height;

	if (tile->thumbnail != nullptr)
		delete[] tile->thumbnail;

	tile->thumb_width = thumb_w;
	tile->thumb_height = thumb_h;
	tile->thumbnail = new unsigned int[thumb_w * thumb_h];

	for (int i = 0; i < thumb_w*thumb_h; i++)
	{
		tile->thumbnail[i] = thumb[i];
	}

	//tesselateAllByTile(index);

	m_edit_callback->tileReplaced(index);

	return tile->id;
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