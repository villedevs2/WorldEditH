#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

#include "PolygonDef.h"
#include "VBO.h"
#include "Tileset.h"

class Tilemap
{
public:
	class EditCallback
	{
	public:
		virtual void tilemapModified() = 0;
	};

	static const int MAX_VERTS = 18;

	/*
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
		std::string name;
		unsigned int color;
		int id;
		Tilemap::TileType type;
		int side_bits;
		Tilemap::TopType top_type;
		float top_height;
		Tilemap::ShadingType shading_type;
		unsigned int* thumbnail;
		int thumb_width;
		int thumb_height;

		int numTopPoints()
		{
			int numtop = 0;
			switch (type)
			{
				case Tilemap::TILE_FULL: numtop = 6; break;
				case Tilemap::TILE_LEFT: numtop = 4; break;
				case Tilemap::TILE_RIGHT: numtop = 4; break;
				case Tilemap::TILE_TOP: numtop = 3; break;
				case Tilemap::TILE_BOTTOM: numtop = 3; break;
				case Tilemap::TILE_MID: numtop = 4; break;
				case Tilemap::TILE_CORNER_TL: numtop = 3; break;
				case Tilemap::TILE_CORNER_TR: numtop = 3; break;
				case Tilemap::TILE_CORNER_BL: numtop = 3; break;
				case Tilemap::TILE_CORNER_BR: numtop = 3; break;
			}
			return numtop;
		}
	};
	*/

	struct Bucket
	{
		unsigned int* map;
		VBO* tiles;
		uint64_t coverage;
		int x;
		int y;
	};

	static const unsigned int TILE_MASK = 0xffff;
	static const unsigned int TILE_EMPTY = 0xffff;
	static const unsigned int Z_MASK = 0xff0000;
	static const unsigned int Z_SHIFT = 16;

	static const int Z_MAX = 100;

	static const int AREA_WIDTH = 8192;
	static const int AREA_HEIGHT = 8192;
	static const int AREA_CENTER_X = AREA_WIDTH / 2;
	static const int AREA_CENTER_Y = AREA_HEIGHT / 2;
	static const int BUCKET_WIDTH = 8;
	static const int BUCKET_HEIGHT = 8;

	static const int THUMB_WIDTH = 70;
	static const int THUMB_HEIGHT = 105;



	Tilemap(Tileset* tileset, Tilemap::EditCallback* edit_callback);
	~Tilemap();

	void tileChanged(int index);

	void reset();
	int get(int x, int y);
	int getZ(int x, int y);	
	void edit(int x, int y, int tile);
	void editZ(int x, int y, int z);
	float getTileWidth();
	float getTileHeight();
	Tilemap::Bucket* getTileBucket(int bx, int by);
	Tilemap::Bucket* getTileBucket(int index);

private:
	void tesselateTile(Bucket* bucket, int bx, int by);
	void allocBucket(int bin);
	void deallocBucket(int bin);
	void tesselateAllByTile(int tile);
	unsigned int getTileColor(unsigned int basecolor, float lum);

	float m_tile_width;
	float m_tile_height;

	Bucket** m_buckets;

	Tileset* m_tileset;
	Tilemap::EditCallback* m_edit_callback;
};
