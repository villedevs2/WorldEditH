#include "PolygonDef.h"

PolygonDef::PolygonDef(int capacity)
{
	m_capacity = capacity;
	m_points.clear();
}

PolygonDef::~PolygonDef()
{
}

bool PolygonDef::convexTest(glm::vec2 point)
{	
	if (m_points.size() >= 2)
	{
		// check convexity
		for (int i=1; i < m_points.size(); i++)
		{
			glm::vec2 ap0 = m_points[i-1];
			glm::vec2 ap1 = m_points[i];

			glm::vec2 vec_a = ap1 - ap0;
			glm::vec2 norm_a = glm::vec2(-vec_a.y, vec_a.x);
			glm::vec2 vec_b = ap1 - point;

			float dot = glm::dot(norm_a, vec_b);
			if (dot < 0.0f)
				return false;
		}
	}

	return true;
}

bool PolygonDef::fullConvexTest()
{
	if (m_points.size() >= 2)
	{
		// check convexity
		for (int i=1; i < m_points.size(); i++)
		{
			glm::vec2 ap0, ap1, ap2;
			ap0 = m_points[i-1];
			ap1 = m_points[i];
			if (i == (m_points.size()-1))
				ap2 = m_points[0];
			else
				ap2 = m_points[i+1];
			
			glm::vec2 vec_a = ap1 - ap0;
			glm::vec2 norm_a = glm::vec2(-vec_a.y, vec_a.x);
			glm::vec2 vec_b = ap1 - ap2;

			float dot = glm::dot(norm_a, vec_b);
			if (dot < 0.0f)
				return false;
		}

		return true;
	}
	return false;
}

PolygonDef::Status PolygonDef::insertPoint(glm::vec2 point)
{
	if (m_points.size() >= m_capacity)
		return POLY_STATUS_CAPACITY;


	// check convexity
	if (!convexTest(point))
	{
		return POLY_STATUS_NONCONVEX;
	}

	m_points.push_back(point);

	return POLY_STATUS_OK;
}

void PolygonDef::deleteLatest()
{
	if (m_points.size() > 0)
		m_points.pop_back();
}

int PolygonDef::getNumPoints()
{
	return m_points.size();
}

int PolygonDef::getCapacity()
{
	return m_capacity;
}

void PolygonDef::reset()
{
	m_points.clear();
}

glm::vec2 PolygonDef::getPoint(int index)
{
	assert(index >= 0 && index < m_points.size());

	return m_points[index];
}

void PolygonDef::edit(int index, glm::vec2 point)
{
	assert(index >= 0 && index < m_points.size());

	m_points[index] = point;
}

bool PolygonDef::lineSide(glm::vec2& p0, glm::vec2& p1, glm::vec2& point)
{
	glm::vec2 edge = p1 - p0;
	glm::vec2 norm = glm::vec2(-edge.y, edge.x);
	glm::vec2 vecb = p1 - point;

	float dot = glm::dot(norm, vecb);
	if (dot < 0.0f)
		return false;

	return true;
}

bool PolygonDef::isPointInside(glm::vec2 point)
{
	for (int i=0; i < m_points.size(); i++)
	{
		glm::vec2 p0, p1;
		if (i == (m_points.size() - 1))
		{
			p0 = m_points[i];
			p1 = m_points[0];
		}
		else
		{
			p0 = m_points[i];
			p1 = m_points[i+1];
		}

		glm::vec2 edge = p1 - p0;
		glm::vec2 norm = glm::vec2(-edge.y, edge.x);
		glm::vec2 vecb = p1 - point;

		float dot = glm::dot(norm, vecb);
		if (dot < 0.0f)
			return false;
	}

	return true;
}

bool PolygonDef::isPointOnEdge(glm::vec2 point, int v1, int v2, float threshold)
{
	assert(v1 >= 0 && v1 < m_points.size());
	assert(v2 >= 0 && v2 < m_points.size());

	glm::vec2 np[4];

	glm::vec2 p0 = m_points[v1];
	glm::vec2 p1 = m_points[v2];

	glm::vec2 edge = p1 - p0;
	glm::vec2 norm = glm::normalize(glm::vec2(-edge.y, edge.x));

	np[0] = p0 + (norm * threshold);
	np[1] = p1 + (norm * threshold);
	np[2] = p1 + (-norm * threshold);
	np[3] = p0 + (-norm * threshold);

	if (!lineSide(np[0], np[1], point))
		return false;
	if (!lineSide(np[1], np[2], point))
		return false;
	if (!lineSide(np[2], np[3], point))
		return false;
	if (!lineSide(np[3], np[0], point))
		return false;

	return true;
}

void PolygonDef::calculateBounds(float*  minx, float* maxx, float* miny, float* maxy)
{
	*minx = m_points[0].x;
	*maxx = *minx;
	*miny = m_points[0].y;
	*maxy = *miny;

	for (int i=1; i < m_points.size(); i++)
	{
		glm::vec2 p = m_points[i];
		if (p.x < *minx)
			*minx = p.x;
		if (p.x > *maxx)
			*maxx = p.x;
		if (p.y < *miny)
			*miny = p.y;
		if (p.y > *maxy)
			*maxy = p.y;
	}
}

void PolygonDef::copy(PolygonDef* src)
{
	m_points.clear();

	for (int i = 0; i < src->getNumPoints(); i++)
	{
		m_points.push_back(src->getPoint(i));
	}
}