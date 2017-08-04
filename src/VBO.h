#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

class VBO
{
public:
	/*
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		unsigned int color;
	};
	*/

	class Vertex
	{
	public:
		Vertex::Vertex()
		{
			this->pos = glm::vec3();
			this->uv = glm::vec2();
			this->normal = glm::vec3();
			this->color = 0;
		}
		Vertex::Vertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec3& normal, unsigned int color)
		{
			this->pos = pos;
			this->uv = uv;
			this->normal = normal;
			this->color = color;
		}
		Vertex::~Vertex()
		{
		}

		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		unsigned int color;
	};

	VBO(int capacity);
	~VBO();

	int makeTri(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
				const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, unsigned int color);
	int makeQuad(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4,
				const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, const glm::vec2& uv4, unsigned int color);

	/*
	int makeTri(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
				const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3,
				const glm::vec3& uvb1, const glm::vec3& uvb2, const glm::vec3& uvb4, unsigned int color);
	int makeQuad(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4,
				const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, const glm::vec2& uv4,
				const glm::vec3& uvb1, const glm::vec3& uvb2, const glm::vec3& uvb3, const glm::vec3& uvb4, unsigned int color);
				*/
	int makeTri(int index, const VBO::Vertex& v1, const VBO::Vertex& v2, const VBO::Vertex& v3);
	int makeQuad(int index, const VBO::Vertex& v1, const VBO::Vertex& v2, const VBO::Vertex& v3, const VBO::Vertex& v4);
	int makeQuadPolyNorm(int index, const VBO::Vertex& v1, const VBO::Vertex& v2, const VBO::Vertex& v3, const VBO::Vertex& v4);

	void degenTri(int index);
	void degenTris(int index, int num);
	static int getVertexSize();
	void* getPointer();
	int getCapacity();

private:
	int m_capacity;

	Vertex* m_data;
};