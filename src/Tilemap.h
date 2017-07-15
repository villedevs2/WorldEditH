#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

#include "PolygonDef.h"
#include "VBO.h"

class Tilemap
{
public:
	enum TileType
	{
		TILE_FULL = 0,
		TILE_LEFT = 1,
		TILE_RIGHT = 2,
		TILE_TOP = 3,
		TILE_BOTTOM = 4,
		TILE_MID = 5,
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
		glm::vec2 top_points[6];
		glm::vec2 side_points[4];
		std::string name;
		unsigned int color;
		int id;
		Tilemap::TileType type;
		int side_bits;
	};

	struct Bucket
	{
		unsigned int* map;
		int vb_slot;
		VBO* tiles;
		VBO* preview;
		uint64_t coverage;
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



	Tilemap();
	~Tilemap();

	void reset();
	float* getVBO(int bx, int by);
	int numTris(int bx, int by);
	int get(int x, int y);
	int getZ(int x, int y);	
	void edit(int x, int y, int tile);
	void editZ(int x, int y, int z);
	unsigned int getRaw(int x, int y);
	void editRaw(int x, int y, unsigned int data);
	int insertTile(std::string name, PolygonDef* top, PolygonDef* side, unsigned int color, Tilemap::TileType type);
	bool removeTile(int id);
	void removeTiles();
	int getNumTiles();
	const Tilemap::Tile* getTile(int index);
	const Tilemap::Tile* getTileById(int id);
	int getTileIndexById(int id);
	float getTileWidth();
	float getTileHeight();

private:
	void tesselateTile(int x, int y);
	void allocBucket(int bin);
	void deallocBucket(int bin);

	float m_tile_width;
	float m_tile_height;

	Bucket** m_buckets;

	int m_cumulative_tile_id;
	std::vector<Tile> m_tiles;
};
