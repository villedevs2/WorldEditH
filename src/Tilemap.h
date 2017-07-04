#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

#include "PolygonDef.h"

class Tilemap
{
public:
	struct VBO
	{
		glm::vec3 pos;
		glm::vec2 uv;
		unsigned int color;
	};

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

	static const unsigned int TILE_MASK = 0xffff;
	static const unsigned int TILE_EMPTY = 0xffff;
	static const unsigned int Z_MASK = 0xff0000;
	static const unsigned int Z_SHIFT = 16;



	Tilemap();
	~Tilemap();

	bool enlarge(int xleft, int xright, int ytop, int ybottom);
	bool shrink(int xleft, int xright, int ytop, int ybottom);
	void resize(int width, int height);
	void reset();
	float* getVBO();
	int numTris();
	int getTile(int x, int y);
	int getZ(int x, int y);	
	void edit(int x, int y, int tile);
	void editZ(int x, int y, int z);
	void incZ(int x, int y, int z);
	unsigned int getRaw(int x, int y);
	void editRaw(int x, int y, unsigned int data);
	void tesselateAll();
	int insertTile(std::string name, PolygonDef* top, PolygonDef* side, unsigned int color, Tilemap::TileType type);
	bool removeTile(int id);
	void removeTiles();
	int getNumTiles();
	const Tilemap::Tile* getTile(int index);
	const Tilemap::Tile* getTileById(int id);
	int getTileIndexById(int id);
	int getWidth();
	int getHeight();
	float getTileWidth();
	float getTileHeight();

private:
	void tesselateTile(int x, int y, bool odd);

	int m_width;
	int m_height;

	float m_tile_width;
	float m_tile_height;

	unsigned int *m_map;

	VBO* m_vb; 

	int m_cumulative_tile_id;
	std::vector<Tile> m_tiles;
};
