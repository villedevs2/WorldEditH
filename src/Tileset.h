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
		TILE_FULL = 0,
		TILE_LEFT = 1,
		TILE_RIGHT = 2,
		TILE_TOP = 3,
		TILE_BOTTOM = 4,
		TILE_MID = 5,
		TILE_CORNER_TL = 6,
		TILE_CORNER_TR = 7,
		TILE_CORNER_BL = 8,
		TILE_CORNER_BR = 9,
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

	enum TopType
	{
		TOP_FLAT = 0,
		TOP_POINTY = 1
	};

	enum ShadingType
	{
		SHADING_STANDARD = 0,
		SHADING_ENVIRONMENT = 1,
		SHADING_SELFLUMINOUS = 2,
	};

	struct Tile
	{
		glm::vec2 top_points[6];
		glm::vec2 side_points[4];
		glm::vec2 sidetop_points[4];
		glm::vec2 sidebot_points[4];
		std::string name;
		unsigned int color;
		int id;
		Tileset::TileType type;
		int side_bits;
		Tileset::TopType top_type;
		float top_height;
		Tileset::ShadingType shading_type;
		unsigned int* thumbnail;
		int thumb_width;
		int thumb_height;

		int numTopPoints()
		{
			int numtop = 0;
			switch (type)
			{
			case Tileset::TILE_FULL: numtop = 6; break;
			case Tileset::TILE_LEFT: numtop = 4; break;
			case Tileset::TILE_RIGHT: numtop = 4; break;
			case Tileset::TILE_TOP: numtop = 3; break;
			case Tileset::TILE_BOTTOM: numtop = 3; break;
			case Tileset::TILE_MID: numtop = 4; break;
			case Tileset::TILE_CORNER_TL: numtop = 3; break;
			case Tileset::TILE_CORNER_TR: numtop = 3; break;
			case Tileset::TILE_CORNER_BL: numtop = 3; break;
			case Tileset::TILE_CORNER_BR: numtop = 3; break;
			}
			return numtop;
		}
	};

	Tileset(Tileset::EditCallback* edit_callback);
	~Tileset();

	int insertTile(std::string name, PolygonDef* top, PolygonDef* side, PolygonDef* sidetop, PolygonDef* sidebot, unsigned int color,
					Tileset::TileType type,
					Tileset::TopType top_type,
					Tileset::ShadingType shading_type,
					float top_height, unsigned int* thumb, int thumb_w, int thumb_h);
	int replaceTile(int index, std::string name, PolygonDef* top, PolygonDef* side, PolygonDef* sidetop, PolygonDef* sidebot, unsigned int color,
					Tileset::TileType type,
					Tileset::TopType top_type,
					Tileset::ShadingType shading_type,
					float top_height, unsigned int* thumb, int thumb_w, int thumb_h);
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