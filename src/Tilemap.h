#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

#include "PolygonDef.h"
#include "VBO.h"
#include "Tileset.h"
#include "AmbientOcclusion.h"

class Tilemap
{
public:
	class EditCallback
	{
	public:
		virtual void tilemapModified() = 0;
	};

	static const int MAX_VERTS = 18;

	struct Bucket
	{
		unsigned int* map;
		VBO* tiles;
		uint64_t coverage;
		int x;
		int y;
	};

	enum Flags
	{
		FLAGS_FIXED_Z = 0x00000001,
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



	Tilemap(Tileset* tileset, Tilemap::EditCallback* edit_callback, float zbase, float zbase_height, unsigned int flags);
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
	struct TileCoord
	{
		int x;
		int y;
	};
	struct AdjacentTileCoords
	{
		TileCoord left;
		TileCoord right;
		TileCoord topleft;
		TileCoord topright;
		TileCoord botleft;
		TileCoord botright;
	};
	struct AdjacentTiles
	{
		int left;
		int right;
		int topleft;
		int topright;
		int botleft;
		int botright;
	};


	void tesselateTile(Bucket* bucket, int bx, int by);
	void retesselateTileByCoords(int tx, int ty);
	void allocBucket(int bin);
	void deallocBucket(int bin);
	void tesselateAllByTile(int tile);
	unsigned int getTileColor(unsigned int basecolor, float lum);
	void getAdjacentTileCoords(AdjacentTileCoords* tiles, int tx, int ty);
	void getAdjacentTiles(AdjacentTiles* tiles, AdjacentTileCoords* coords);

	float m_tile_width;
	float m_tile_height;

	float m_zbase;
	float m_zbase_height;
	unsigned int m_flags;

	Bucket** m_buckets;

	Tileset* m_tileset;
	Tilemap::EditCallback* m_edit_callback;
};
