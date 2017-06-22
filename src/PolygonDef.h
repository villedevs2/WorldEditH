#pragma once

#include <glm.hpp>
#include <vector>

class PolygonDef
{
public:
	enum Status
	{
		POLY_STATUS_OK = 0,
		POLY_STATUS_CAPACITY,
		POLY_STATUS_NONCONVEX,
	};

	PolygonDef(int capacity);
	~PolygonDef();

	PolygonDef::Status insertPoint(glm::vec2 point);
	void deleteLatest();
	int getNumPoints();
	int getCapacity();
	void reset();
	glm::vec2 getPoint(int index);
	bool convexTest(glm::vec2 point);
	bool fullConvexTest();
	void edit(int index, glm::vec2 point);
	bool lineSide(glm::vec2& p0, glm::vec2& p1, glm::vec2& point);
	bool isPointInside(glm::vec2 point);
	void calculateBounds(float* minx, float* maxx, float* miny, float* maxy);
	bool isPointOnEdge(glm::vec2 point, int v1, int v2, float threshold);

private:
	std::vector<glm::vec2> m_points;
	int m_capacity;
};