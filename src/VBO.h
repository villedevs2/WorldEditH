#pragma once

#include <glm.hpp>

#include <string>
#include <vector>


class HSVertex
{
public:
	HSVertex::HSVertex();
	HSVertex::HSVertex(const glm::vec3& pos, const glm::vec2& uv, unsigned int color);
	HSVertex::HSVertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec3& normal, unsigned int color);
	HSVertex::~HSVertex();

	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	unsigned int color;
};


template <typename T>
class VBO
{
public:
	VBO(int capacity)
	{
		m_capacity = capacity;

		m_data = new T[capacity * 3];
	}
	~VBO()
	{
	}

	int makeTri(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
		const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, unsigned int color)
	{
		assert(index >= 0 && index < m_capacity);

		int i = index * 3;
		m_data[i + 0].pos = p1;		m_data[i + 0].uv = uv1;		m_data[i + 0].color = color;
		m_data[i + 1].pos = p2;		m_data[i + 1].uv = uv2;		m_data[i + 1].color = color;
		m_data[i + 2].pos = p3;		m_data[i + 2].uv = uv3;		m_data[i + 2].color = color;

		return 1;
	}
	int makeQuad(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4,
		const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, const glm::vec2& uv4, unsigned int color)
	{
		assert(index >= 0 && index < m_capacity);

		int i = index * 3;
		m_data[i + 0].pos = p1;		m_data[i + 0].uv = uv1;		m_data[i + 0].color = color;
		m_data[i + 1].pos = p3;		m_data[i + 1].uv = uv3;		m_data[i + 1].color = color;
		m_data[i + 2].pos = p2;		m_data[i + 2].uv = uv2;		m_data[i + 2].color = color;

		m_data[i + 3].pos = p1;		m_data[i + 3].uv = uv1;		m_data[i + 3].color = color;
		m_data[i + 4].pos = p4;		m_data[i + 4].uv = uv4;		m_data[i + 4].color = color;
		m_data[i + 5].pos = p3;		m_data[i + 5].uv = uv3;		m_data[i + 5].color = color;

		return 2;
	}

	int makeTri(int index, const T& v1, const T& v2, const T& v3)
	{
		assert(index >= 0 && index < m_capacity);

		int i = index * 3;
		m_data[i + 0] = v1;
		m_data[i + 1] = v3;
		m_data[i + 2] = v2;

		return 1;
	}

	int makeTriPolyNorm(int index, const T& v1, const T& v2, const T& v3)
	{
		assert(index >= 0 && index < m_capacity);

		glm::vec3 e1 = v2.pos - v1.pos;
		glm::vec3 e2 = v3.pos - v1.pos;
		glm::vec3 norm;
		glm::vec3 cr = glm::cross(e1, e2);
		if (!(cr.x == 0 && cr.y == 0 && cr.z == 0))
			norm = glm::normalize(cr);
		else
			norm = glm::vec3(0, 0, 0);

		int i = index * 3;
		m_data[i + 0] = v1;
		m_data[i + 0].normal = norm;
		m_data[i + 1] = v3;
		m_data[i + 1].normal = norm;
		m_data[i + 2] = v2;
		m_data[i + 2].normal = norm;

		return 1;
	}

	int makeQuad(int index, const T& v1, const T& v2, const T& v3, const T& v4)
	{
		assert(index >= 0 && index < m_capacity);

		int i = index * 3;
		m_data[i + 0] = v1;
		m_data[i + 1] = v3;
		m_data[i + 2] = v2;

		m_data[i + 3] = v1;
		m_data[i + 4] = v4;
		m_data[i + 5] = v3;

		return 2;
	}

	int makeQuadPolyNorm(int index, const T& v1, const T& v2, const T& v3, const T& v4)
	{
		assert(index >= 0 && index < m_capacity);

		glm::vec3 e1 = v2.pos - v1.pos;
		glm::vec3 e2 = v4.pos - v1.pos;
		glm::vec3 norm;
		glm::vec3 cr = glm::cross(e1, e2);
		if (!(cr.x == 0 && cr.y == 0 && cr.z == 0))
			norm = glm::normalize(cr);
		else
			norm = glm::vec3(0, 0, 0);

		int i = index * 3;
		m_data[i + 0] = v1;
		m_data[i + 0].normal = norm;
		m_data[i + 1] = v3;
		m_data[i + 1].normal = norm;
		m_data[i + 2] = v2;
		m_data[i + 2].normal = norm;

		m_data[i + 3] = v1;
		m_data[i + 3].normal = norm;
		m_data[i + 4] = v4;
		m_data[i + 4].normal = norm;
		m_data[i + 5] = v3;
		m_data[i + 5].normal = norm;

		return 2;
	}

	void degenTri(int index)
	{
		assert(index >= 0 && index < m_capacity);

		int i = index * 3;
		m_data[i + 0].pos = glm::vec3(0, 0, 0);		m_data[i + 0].uv = glm::vec2(0, 0);		m_data[i + 0].color = 0;
		m_data[i + 1].pos = glm::vec3(0, 0, 0);		m_data[i + 1].uv = glm::vec2(0, 0);		m_data[i + 1].color = 0;
		m_data[i + 2].pos = glm::vec3(0, 0, 0);		m_data[i + 2].uv = glm::vec2(0, 0);		m_data[i + 2].color = 0;
	}

	void degenTris(int index, int num)
	{
		assert(index >= 0 && (index + num) <= m_capacity);

		for (int j = index; j < index + num; j++)
		{
			int i = j * 3;
			m_data[i + 0].pos = glm::vec3(0, 0, 0);		m_data[i + 0].uv = glm::vec2(0, 0);		m_data[i + 0].color = 0;
			m_data[i + 1].pos = glm::vec3(0, 0, 0);		m_data[i + 1].uv = glm::vec2(0, 0);		m_data[i + 1].color = 0;
			m_data[i + 2].pos = glm::vec3(0, 0, 0);		m_data[i + 2].uv = glm::vec2(0, 0);		m_data[i + 2].color = 0;
		}
	}

	void* getPointer()
	{
		return (void*)m_data;
	}

	int getCapacity()
	{
		return m_capacity;
	}

private:
	int m_capacity;
	T* m_data;
};

#if 0
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
			this->amb_uv = glm::vec2();
			this->normal = glm::vec3();
			this->color = 0;
		}
		Vertex::Vertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec2& amb_uv, const glm::vec3& normal, unsigned int color)
		{
			this->pos = pos;
			this->uv = uv;
			this->amb_uv = amb_uv;
			this->normal = normal;
			this->color = color;
		}
		Vertex::~Vertex()
		{
		}

		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec2 amb_uv;
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
	int makeTriPolyNorm(int index, const VBO::Vertex& v1, const VBO::Vertex& v2, const VBO::Vertex& v3);
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
#endif