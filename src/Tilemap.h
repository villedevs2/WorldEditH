#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

class Tilemap
{
public:
	struct VBO
	{
		glm::vec3 pos;
		glm::vec2 uv;
		unsigned int color;
	};

	struct Config
	{
		int xstart;
		int xend;
		int ystart;
		int yend;
		float tile_width;
		float tile_height;
	};

	struct Tile
	{
		glm::vec2 points[4];
		std::string name;
		unsigned int color;
		int id;
		int type;
	};

	Tilemap();
	~Tilemap();

	bool resize(int x1, int y1, int x2, int y2, float tile_width, float tile_height);
	bool resize(const Tilemap::Config& config);
	void reset();
	float* getVBO();
	int numTris();
	int get(int x, int y);
	void edit(int x, int y, int tile);
	void tesselateAll();
	const Tilemap::Config& getConfig();
	int insertTile(std::string name, glm::vec2* points, unsigned int color);
	bool removeTile(int id);
	void removeTiles();
	int getNumTiles();
	const Tilemap::Tile& getTile(int index);
	const Tilemap::Tile* getTileById(int id);
	int getTileIndexById(int id);

private:
	void tesselateTile(int x, int y);

	int m_x;
	int m_y;
	int m_width;
	int m_height;

	float m_tile_width;
	float m_tile_height;

	int *m_map;

	VBO* m_vb; 

	Config m_config;

	int m_cumulative_tile_id;
	std::vector<Tile> m_tiles;
};
