#pragma once

#define _USE_MATH_DEFINES

#include <glm/glm.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gtx/intersect.hpp>

#include <vector>
#include <random>
#include <cmath>

#include "PolygonDef.h"

#include "qimage.h"
#include "qbuffer.h"


class AmbientOcclusion
{
public:
	AmbientOcclusion();
	~AmbientOcclusion();

	void calculate();
	QImage* getMap();
	bool load(QString name);

	enum Sides
	{
		SIDE_LEFT = 0x1,
		SIDE_RIGHT = 0x2,
		SIDE_TOPLEFT = 0x4,
		SIDE_TOPRIGHT = 0x8,
		SIDE_BOTLEFT = 0x10,
		SIDE_BOTRIGHT = 0x20,
	};

	enum WallSides
	{
		WALLSIDE_LEFT = 0x1,
		WALLSIDE_RIGHT = 0x2,
		WALLSIDE_FLOOR = 0x4,
	};

	static const int MAP_WIDTH = 32;
	static const int MAP_HEIGHT = 32;
	static const int FLOOR_TILES_X = 8;
	static const int FLOOR_TILES_Y = 8;

	static const float RAY_FALLOFF;

private:
	void makeRays(std::vector<glm::vec3>& rays, int numrays);
	void calculateFloor(int sides, int width, int height, int* buffer, std::vector<glm::vec3>& rays);
	bool sampleFloorHit(const glm::vec2& wall_point1, const glm::vec2& wall_point2, const glm::vec2& sample_point, const glm::vec3& ray);
	void calculateWall(int sides, int width, int height, int* buffer, std::vector<glm::vec3>& rays);

	QImage* m_map;
};