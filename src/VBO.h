#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

class VBO
{
public:
	VBO(int capacity);
	~VBO();

	void makeTri(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
							const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, unsigned int color);
	void makeQuad(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4,
							 const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, const glm::vec2& uv4, unsigned int color);
	void degenTri(int index);
	static int getVertexSize();
	void* getPointer();
	int getCapacity();

private:
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
		unsigned int color;
	};

	int m_capacity;

	Vertex* m_data;
};