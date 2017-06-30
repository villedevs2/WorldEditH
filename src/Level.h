#pragma once

#include <glm.hpp>

#include <vector>
#include <string>

#include "Tilemap.h"

class Level
{
public:
	enum ObjectType
	{
		OBJECT_TYPE_INVALID			= 0,
		OBJECT_TYPE_TRIGGER			= 1,
		OBJECT_TYPE_DESTRUCTIBLE	= 2,
		OBJECT_TYPE_MOVER			= 3,
		OBJECT_TYPE_ENEMY			= 4,
	};

	enum VBOIndex
	{
		VBO_DESTRUCTIBLE = 0,
		VBO_MOVER = 1,
		VBO_ENEMY = 2,
		NUM_VBOS,
	};

	class Object
	{
		friend class Level;

	public:
		union Param
		{
			int i;
			float f;
			char b[4];
		};

		static const int NUM_PARAMS = 8;

		Object(int id, glm::vec2* points, glm::vec2* uvs, int num_points, ObjectType type, std::string& name, unsigned int color);
		~Object();

		bool pointInside(glm::vec2 point, float threshold);
		const glm::vec2& getPoint(int i);
		const glm::vec2& getUV(int i);
		int getNumPoints();
		void insertPoint(glm::vec2 pos, glm::vec2 uv);
		std::string& getName();
		void setName(std::string& name);
		Level::ObjectType getType();
		void setType(Level::ObjectType type);
		int getId();
		void reset();
		void setParam(int index, Level::Object::Param data);
		Level::Object::Param getParam(int index);
		void copy(const Level::Object& source);
		static void setParent(Level* level);
		glm::vec2 getBoundingMin();
		glm::vec2 getBoundingMax();
		void setZ(int z);
		int getZ();
		unsigned int getColor();
		void setColor(unsigned int color);

	private:
		glm::vec2 m_points[8];
		glm::vec2 m_uvs[8];
		int m_num_points;
		unsigned int m_color;
		ObjectType m_type;
		std::string m_name;
		float m_minx;
		float m_maxx;
		float m_miny;
		float m_maxy;
		int m_vbo_index;
		int m_id;
		int m_z;
		Param m_params[8];

		static Level* m_parent;

		void calculateBoundingBox();
	};

	struct VBO
	{
		glm::vec3 position;
		glm::vec2 uv;
		unsigned int color;
	};

	Level();
	~Level();

	int insertObject(glm::vec2* points, glm::vec2* uvs, int num_points, ObjectType type, std::string name, unsigned int color);
	void editObjectGeo(int object, glm::vec2* points, glm::vec2* uvs);
	void editObjectGeo(int object, glm::vec2* points);
	void editObjectUVs(int object, glm::vec2* uvs);
	void removeObject(int object);
	void removeObjectById(int id);
	void removeObjects();
	int numObjects();
	Level::Object* getObject(int object);
	Level::Object* getObjectById(int id);
	int getIndexById(int id);
	void reset();
	float* getVBO(int index);
	int numVBOVerts(int index);
	bool isModified();
	void resetModify();
	bool isVBOUpdated();
	void resetVBOUpdate();
	void setTriggerList(std::vector<std::string>& list);
	const std::string& getTriggerName(int index);
	int getNumTriggers();


	void setTilemapSize(int width, int height);
	void enlargeTilemap(int xleft, int xright, int ytop, int ybottom);
	void shrinkTilemap(int xleft, int xright, int ytop, int ybottom);
	int readTilemap(int x, int y);
	void editTilemap(int x, int y, int tile);
	int getTilemapWidth();
	int getTilemapHeight();
	float* getTilemapVBO();
	int numTilemapTris();
	const Tilemap::Config& getTilemapConfig();
	int insertTile(std::string name, glm::vec2* points, unsigned int color, Tilemap::TileType type);
	void removeTile(int id);
	void removeTiles();
	int getNumTiles();
	const Tilemap::Tile& getTile(int index);
	const Tilemap::Tile* getTileById(int id);
	int getTileIndexById(int id);

private:
	int tesselateObject(int object);
	void tesselateAllObjects();
	void setModified();

	std::vector<Object*> m_objects;
	VBO* m_vbo[NUM_VBOS];
	int m_num_verts[NUM_VBOS];
	int m_cumulative_object_id;

	bool m_modified;
	bool m_vbo_updated;

	std::vector<std::string> m_trigger_list;

	Tilemap* m_tilemap;
};