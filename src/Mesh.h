#pragma once

#include <glm.hpp>

#include <string>
#include <vector>

#include "BinaryFile.h"

class Mesh
{
public:
	Mesh();
	~Mesh();
	bool load(std::string& filename);

private:
	static const int VMF_HEADER_VERSION = 0x0005;
	static const int VMF_MESH_VERSION = 0x0005;

	static const unsigned int VMF_FORMAT_ID = 0x564d4658;
	static const unsigned int VMF_MESH_ID = 0x53424d58;
	static const unsigned int VMF_TEX_ID = 0x54455858;
	static const unsigned int VMF_VTX_ID = 0x56545858;
	static const unsigned int VMF_TVTX_ID = 0x54565458;
	static const unsigned int VMF_NRM_ID = 0x4e524d58;
	static const unsigned int VMF_FACE_ID = 0x46435858;
	static const unsigned int VMF_KEY_ID = 0x4b455958;
	static const unsigned int VMF_ANIM_ID = 0x414e4d58;

	struct Triangle
	{
		int p[3];
		int uv[3];
		glm::vec3 normal[3];
	};

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 uvcoord;
		glm::vec3 normal;
	};

	struct SubMesh
	{
		std::vector<glm::vec3> position;
		std::vector<glm::vec2> uvcoord;
		std::vector<Triangle> tris;
		std::string texname;
		std::string name;
		glm::mat4 matrix;
	};

	vector<SubMesh> m_submesh;
};