#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

#include "PolygonDef.h"
#include "VBO.h"

class Tileset
{
public:
	class EditCallback
	{
	public:
		virtual void tileAdded(int index) = 0;
		virtual void tileReplaced(int index) = 0;
		virtual void tileRemoved(int index) = 0;
	};

	enum TileType
	{
		TILE_FLOOR,
		TILE_WALL,
		TILE_TOWER,
		TILE_ENV
	};

	enum TileSide
	{
		SIDE_LEFT = 0x1,
		SIDE_TOP_LEFT = 0x2,
		SIDE_TOP_RIGHT = 0x4,
		SIDE_RIGHT = 0x8,
		SIDE_BOT_RIGHT = 0x10,
		SIDE_BOT_LEFT = 0x20,
		SIDE_MID = 0x40,
	};

	struct Tile
	{
		glm::vec2 points[6];
		std::string name;
		unsigned int color;
		int id;
		Tileset::TileType type;
		int side_bits;
		unsigned int* thumbnail;
		int thumb_width;
		int thumb_height;
	};

	Tileset(Tileset::EditCallback* edit_callback);
	~Tileset();

	int insertTile(std::string name, PolygonDef* uv, unsigned int color, std::string type, unsigned int* thumb, int thumb_w, int thumb_h);
	bool removeTile(int id);
	void removeTiles();
	int getNumTiles();
	Tileset::Tile* getTile(int index);
	Tileset::Tile* getTileById(int id);
	int getTileIndexById(int id);

private:
	int getSideBits(Tileset::TileType type);

	int m_cumulative_tile_id;
	std::vector<Tile> m_tiles;
	Tileset::EditCallback* m_edit_callback;
};