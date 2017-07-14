#include "VBO.h"

VBO::VBO(int capacity)
{
	m_capacity = capacity;

	m_data = new Vertex[capacity*3];
}

VBO::~VBO()
{
	delete[] m_data;
}

int VBO::getVertexSize()
{
	return sizeof(Vertex);
}

void* VBO::getPointer()
{
	return (void*)m_data;
}

int VBO::getCapacity()
{
	return m_capacity;
}

void VBO::makeTri(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3,
							 const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3, unsigned int color)
{
	assert(index >= 0 && index < m_capacity);
	
	int i = index * 3;
	m_data[i + 0].pos = p1;		m_data[i + 0].uv = uv1;		m_data[i + 0].color = color;
	m_data[i + 1].pos = p2;		m_data[i + 1].uv = uv2;		m_data[i + 1].color = color;
	m_data[i + 2].pos = p3;		m_data[i + 2].uv = uv3;		m_data[i + 2].color = color;
}

void VBO::makeQuad(int index, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& p4,
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
}

void VBO::degenTri(int index)
{
	assert(index >= 0 && index < m_capacity);

	int i = index * 3;
	m_data[i + 0].pos = glm::vec3(0, 0, 0);		m_data[i + 0].uv = glm::vec2(0, 0);		m_data[i + 0].color = 0;
	m_data[i + 1].pos = glm::vec3(0, 0, 0);		m_data[i + 1].uv = glm::vec2(0, 0);		m_data[i + 1].color = 0;
	m_data[i + 2].pos = glm::vec3(0, 0, 0);		m_data[i + 2].uv = glm::vec2(0, 0);		m_data[i + 2].color = 0;
}

void VBO::degenTris(int index, int num)
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