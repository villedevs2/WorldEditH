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

	static const int MAX_VERTS = 128;

	struct Bucket
	{
		unsigned int* map;
		VBO<HSVertex>* tiles;
		uint64_t coverage;
		int x;
		int y;
	};

	enum Flags
	{
		FLAGS_FIXED_Z = 0x00000001,
	};

	struct TileAO
	{
		AmbientOcclusion::AOFloorTile* floor;
		AmbientOcclusion::AOWallTile* wall_left;
		AmbientOcclusion::AOWallTile* wall_right;
		AmbientOcclusion::AOWallTile* wall_topleft;
		AmbientOcclusion::AOWallTile* wall_topright;
		AmbientOcclusion::AOWallTile* wall_botleft;
		AmbientOcclusion::AOWallTile* wall_botright;
		AmbientOcclusion::AOWallTile* wall_sideleft;
		AmbientOcclusion::AOWallTile* wall_sideright;
		AmbientOcclusion::AOWallTile* wall_midtop;
		AmbientOcclusion::AOWallTile* wall_midbot;
		AmbientOcclusion::AOWallTile* wall_centtop;
		AmbientOcclusion::AOWallTile* wall_centbot;
		AmbientOcclusion::AOWallTile* wall_corntl;
		AmbientOcclusion::AOWallTile* wall_corntr;
		AmbientOcclusion::AOWallTile* wall_cornbl;
		AmbientOcclusion::AOWallTile* wall_cornbr;
	};

	struct TileDef
	{
		glm::vec2 floor_uvs[6];
		glm::vec2 floor_uvcen;
		glm::vec2 wallmid_uvs[4];
		glm::vec2 walltop_uvs[4];
		glm::vec2 wallbot_uvs[4];
		Tileset::TileType tiletype;
		Tileset::TopType toptype;
		Tileset::ShadingType shading;
		float tile_z;
		float top_height;
		unsigned int color;
		float tile_width;
		float tile_height;
		Tilemap::TileAO tile_ao;
		int left_height;
		int topleft_height;
		int topright_height;
		int right_height;
		int botright_height;
		int botleft_height;
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



	Tilemap(Tileset* tileset, Tilemap::EditCallback* edit_callback, float zbase, float zbase_height, unsigned int flags, AmbientOcclusion* ao);
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
	static int makeVBOTile(VBO<HSVertex>* vbo, int index, const TileDef& tiledef, int x, int y);

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

	struct AOSolution
	{
		int floor;
		int wall_left;
		int wall_right;
		int wall_topleft;
		int wall_topright;
		int wall_botleft;
		int wall_botright;
		int wall_sideleft;
		int wall_sideright;
		int wall_midtop;
		int wall_midbot;
		int wall_centtop;
		int wall_centbot;
		int wall_corntl;
		int wall_corntr;
		int wall_cornbl;
		int wall_cornbr;
	};

	void tesselateTile(Bucket* bucket, int bx, int by);
	void retesselateTileByCoords(int tx, int ty);
	void allocBucket(int bin);
	void deallocBucket(int bin);
	void tesselateAllByTile(int tile);
	unsigned int getTileColor(unsigned int basecolor, float lum);
	void getAdjacentTileCoords(AdjacentTileCoords* tiles, int tx, int ty);
	void getAdjacentTiles(AdjacentTiles* tiles, AdjacentTileCoords* coords);
	void makeAOSolution(int tx, int ty, AOSolution* ao);

	float m_tile_width;
	float m_tile_height;

	float m_zbase;
	float m_zbase_height;
	unsigned int m_flags;

	Bucket** m_buckets;

	Tileset* m_tileset;
	Tilemap::EditCallback* m_edit_callback;

	AmbientOcclusion* m_ao;
};
