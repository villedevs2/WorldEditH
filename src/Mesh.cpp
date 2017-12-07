#include "Mesh.h"

Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

bool Mesh::load(std::string& filename)
{
	BinaryFile input;

	try
	{
		uint32_t id;
		input.open(filename, BinaryFile::MODE_READONLY);

		// VMF ID
		id = input.read_dword();
		if (id != VMF_FORMAT_ID)
			throw "VMF format id not found";

		// VMF version
		uint32_t vmf_version = input.read_dword();
		if (vmf_version != VMF_HEADER_VERSION)
			throw "Wrong VMF header version";

		int num_total_faces = input.read_dword();
		int num_submeshes = input.read_dword();

		// submeshes
		for (int mesh = 0; mesh < num_submeshes; mesh++)
		{
			SubMesh sb;

			// submesh id
			id = input.read_dword();
			if (id != VMF_MESH_ID)
				throw "VMF submesh id not found";

			uint32_t version = input.read_dword();
			if (version != VMF_MESH_VERSION)
				throw "Wrong VMF submesh version";

			int submesh_struct_length = input.read_dword();

			// submesh name
			char submesh_name[64];
			for (int c = 0; c < 64; c++)
			{
				submesh_name[c] = input.read_byte();
			}

			// submesh matrix
			for (int r = 0; r < 4; r++)
			{
				for (int c = 0; c < 4; c++)
				{
					float f = input.read_float();
					sb.matrix[r][c] = f;
				}
			}

			// vertex coordinates
			// --------------------------------------------------------------------------

			// vertex id
			id = input.read_dword();
			if (id != VMF_VTX_ID)
				throw "Wrong VMF vertex id";

			// number of vertices
			int num_verts = input.read_dword();

			// vertex structures
			for (int i = 0; i < num_verts; i++)
			{
				float x = input.read_float();
				float y = input.read_float();
				float z = input.read_float();
				sb.position[i] = glm::vec3(x, y, z);
			}

			// texture vertex coordinates
			// --------------------------------------------------------------------------

			// tex vertex id
			id = input.read_dword();
			if (id != VMF_TVTX_ID)
				throw "Wrong VMF texture vertex id";

			// number of tex vertices
			int num_tverts = input.read_dword();

			// tex vertex structures
			for (int i = 0; i < num_tverts; i++)
			{
				float x = input.read_float();
				float y = input.read_float();
				sb.uvcoord[i] = glm::vec2(x, y);
			}

			// faces
			// --------------------------------------------------------------------------

			// face id
			id = input.read_dword();
			if (id != VMF_FACE_ID)
				throw "Wrong VMF face id";

			// number of faces
			int num_faces = input.read_dword();

			// face structures
			for (int i = 0; i < num_faces; i++)
			{
				Triangle tri;

				tri.p[0] = input.read_dword();
				tri.p[1] = input.read_dword();
				tri.p[2] = input.read_dword();

				tri.uv[0] = input.read_dword();
				tri.uv[1] = input.read_dword();
				tri.uv[2] = input.read_dword();

				tri.normal[0].x = input.read_float();
				tri.normal[0].y = input.read_float();
				tri.normal[0].z = input.read_float();
				tri.normal[1].x = input.read_float();
				tri.normal[1].y = input.read_float();
				tri.normal[1].z = input.read_float();
				tri.normal[2].x = input.read_float();
				tri.normal[2].y = input.read_float();
				tri.normal[2].z = input.read_float();

				sb.tris.push_back(tri);
			}

			m_submesh.push_back(sb);
		}

	}
	catch ()
	{

	}
}