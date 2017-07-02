#include "Tilemap.h"

Tilemap::Tilemap()
{
	m_vb = NULL;
	m_map = NULL;

	m_config.xstart = 0;
	m_config.xend = 50;
	m_config.ystart = 0;
	m_config.yend = 50;
	m_config.tile_width = 1.0f;
	m_config.tile_height = 1.2f;

	m_tile_width = 1.0f;
	m_tile_height = 1.4f;

	resize(50, 50);

	m_cumulative_tile_id = 1;
}

Tilemap::~Tilemap()
{
	if (m_map != NULL)
		delete [] m_map;

	if (m_vb != NULL)
		delete [] m_vb;
}

void Tilemap::reset()
{
	memset(m_map, -1, sizeof(int) * m_width * m_height);
	removeTiles();
	tesselateAll();
}

int Tilemap::getWidth()
{
	return m_width;
}

int Tilemap::getHeight()
{
	return m_height;
}

float Tilemap::getTileWidth()
{
	return m_tile_width;
}

float Tilemap::getTileHeight()
{
	return m_tile_height;
}

void Tilemap::resize(int width, int height)
{
	if (m_map != NULL)
		delete[] m_map;

	m_map = new int[width * height];

	memset(m_map, -1, sizeof(int) * width * height);

	m_width = width;
	m_height = height;

	if (m_vb != NULL)
	{
		delete[] m_vb;
		m_vb = NULL;
	}

	m_vb = new VBO[width * height * 12];
}

bool Tilemap::enlarge(int xleft, int xright, int ytop, int ybottom)
{
	int new_width = m_width + xleft + xright;
	int new_height = m_height + ytop + ybottom;

	if (xleft == 0 && xright == 0 && ytop == 0 && ybottom == 0)
		return false;

	int* new_map = new int[new_width * new_height];

	memset(new_map, -1, sizeof(int) * new_width * new_height);

	// copy old data
	for (int j=0; j < m_height; j++)
	{
		for (int i=0; i < m_width; i++)
		{
			int di = i + xleft;
			int dj = j + ytop;

			new_map[(dj * new_width) + di] = m_map[(j * m_width) + i];
		}
	}

	delete [] m_map;

	m_map = new_map;

	m_width = new_width;
	m_height = new_height;

	if (m_vb != NULL)
	{
		delete [] m_vb;
		m_vb = NULL;
	}

	m_vb = new VBO[new_width * new_height * 12];

	tesselateAll();

	return true;
}

bool Tilemap::shrink(int xleft, int xright, int ytop, int ybottom)
{
	int new_width = m_width + xleft + xright;
	int new_height = m_height + ytop + ybottom;

	if (xleft == 0 && xright == 0 && ytop == 0 && ybottom == 0)
		return false;

	int* new_map = new int[new_width * new_height];

	memset(new_map, -1, sizeof(int) * new_width * new_height);

	// copy old data
	for (int j = 0; j < new_height; j++)
	{
		for (int i = 0; i < new_width; i++)
		{
			int si = i - xleft;
			int sj = j - ytop;

			new_map[(j * new_width) + i] = m_map[(sj * m_width) + si];
		}
	}

	delete[] m_map;

	m_map = new_map;

	m_width = new_width;
	m_height = new_height;

	if (m_vb != NULL)
	{
		delete[] m_vb;
		m_vb = NULL;
	}

	m_vb = new VBO[new_width * new_height * 12];

	tesselateAll();

	return true;
	return true;
}

void Tilemap::tesselateAll()
{
	for (int j=0; j < m_height; j++)
	{
		for (int i=0; i < m_width; i++)
		{
			bool odd = j & 1;
			tesselateTile(i, j, odd);
		}
	}
}

void Tilemap::tesselateTile(int x, int y, bool odd)
{
	const int num_verts = 3 * 4;

	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	int tile = m_map[(y * m_width) + x];

	int vb_index = ((y * m_width) + x) * num_verts;

	float tx1 = (float)(x) * m_tile_width;
	float tx2 = tx1 + m_tile_width;
	float ty1 = (float)(y) * (m_tile_height / 2);
	float ty2 = ty1 + (m_tile_height / 2);

	if (odd)
	{
		tx1 += m_tile_width / 2;
		tx2 += m_tile_width / 2;
	}

	float z = 100.0f;
	
	if (tile == -1)
	{
		// make degen geo
		for (int i = 0; i < num_verts; i++)
		{
			m_vb[vb_index + i].pos = glm::vec3(0, 0, 0);
			m_vb[vb_index + i].uv = glm::vec2(0, 0);
			m_vb[vb_index + i].color = 0;
		}
	}
	else
	{
		Tile& tiledata = m_tiles.at(tile);

		glm::vec2 vl = tiledata.top_points[1] - tiledata.top_points[0];
		glm::vec2 vr = tiledata.top_points[2] - tiledata.top_points[3];
		glm::vec2 vt = tiledata.top_points[3] - tiledata.top_points[0];
		glm::vec2 vb = tiledata.top_points[2] - tiledata.top_points[1];

		/*
		glm::vec2 tex_my1 = tiledata.points[0] + (vt * 0.5f);
		glm::vec2 tex_my2 = tiledata.points[1] + (vb * 0.5f);
		glm::vec2 tex_ly1 = tiledata.points[0] + (vl * (float)(15.0 / 50.0));
		glm::vec2 tex_ly2 = tiledata.points[0] + (vl * (float)(35.0 / 50.0));
		glm::vec2 tex_ry1 = tiledata.points[3] + (vr * (float)(15.0 / 50.0));
		glm::vec2 tex_ry2 = tiledata.points[3] + (vr * (float)(35.0 / 50.0));
		*/

		
		/*
		glm::vec2 uv1 = tiledata.top_points[0] + (vl * (float)(15.0 / 50.0));
		glm::vec2 uv2 = tiledata.top_points[0] + (vl * (float)(35.0 / 50.0));
		glm::vec2 uv3 = tiledata.top_points[1] + (vb * 0.5f);
		glm::vec2 uv4 = tiledata.top_points[3] + (vr * (float)(35.0 / 50.0));
		glm::vec2 uv5 = tiledata.top_points[3] + (vr * (float)(15.0 / 50.0));
		glm::vec2 uv6 = tiledata.top_points[0] + (vt * 0.5f);
		*/

		glm::vec2 uv1 = tiledata.top_points[0];
		glm::vec2 uv2 = tiledata.top_points[1];
		glm::vec2 uv3 = tiledata.top_points[2];
		glm::vec2 uv4 = tiledata.top_points[3];
		glm::vec2 uv5 = tiledata.top_points[4];
		glm::vec2 uv6 = tiledata.top_points[5];

		/*
		     p6
		p1         p5
		p2         p4
		     p3
		*/

		glm::vec3 p1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), z);
		glm::vec3 p2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), z);
		glm::vec3 p3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), z);
		glm::vec3 p4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), z);
		glm::vec3 p5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), z);
		glm::vec3 p6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, z);

		switch (tiledata.type)
		{
			case Tilemap::TILE_FULL:
			{
				/*     
					   /\
					  /  \
					 |    |
					 |    |
					  \  /
					   \/
				*/

				m_vb[vb_index + 0].pos = p1;	m_vb[vb_index + 0].uv = uv1;		m_vb[vb_index + 0].color = tiledata.color;
				m_vb[vb_index + 1].pos = p6;	m_vb[vb_index + 1].uv = uv6;		m_vb[vb_index + 1].color = tiledata.color;
				m_vb[vb_index + 2].pos = p5;	m_vb[vb_index + 2].uv = uv5;		m_vb[vb_index + 2].color = tiledata.color;

				m_vb[vb_index + 3].pos = p1;	m_vb[vb_index + 3].uv = uv1;		m_vb[vb_index + 3].color = tiledata.color;
				m_vb[vb_index + 4].pos = p5;	m_vb[vb_index + 4].uv = uv5;		m_vb[vb_index + 4].color = tiledata.color;
				m_vb[vb_index + 5].pos = p4;	m_vb[vb_index + 5].uv = uv4;		m_vb[vb_index + 5].color = tiledata.color;

				m_vb[vb_index + 6].pos = p1;	m_vb[vb_index + 6].uv = uv1;		m_vb[vb_index + 6].color = tiledata.color;
				m_vb[vb_index + 7].pos = p4;	m_vb[vb_index + 7].uv = uv4;		m_vb[vb_index + 7].color = tiledata.color;
				m_vb[vb_index + 8].pos = p3;	m_vb[vb_index + 8].uv = uv3;		m_vb[vb_index + 8].color = tiledata.color;

				m_vb[vb_index + 9].pos = p1;	m_vb[vb_index + 9].uv = uv1;		m_vb[vb_index + 9].color = tiledata.color;
				m_vb[vb_index + 10].pos = p3;	m_vb[vb_index + 10].uv = uv3;		m_vb[vb_index + 10].color = tiledata.color;
				m_vb[vb_index + 11].pos = p2;	m_vb[vb_index + 11].uv = uv2;		m_vb[vb_index + 11].color = tiledata.color;
				break;
			}
			case Tilemap::TILE_LEFT:
			{
				/*
				     /|
				    / |
			       |  |
			       |  |
			        \ |
			         \|
				*/

				m_vb[vb_index + 0].pos = p1;	m_vb[vb_index + 0].uv = uv1;		m_vb[vb_index + 0].color = tiledata.color;
				m_vb[vb_index + 1].pos = p6;	m_vb[vb_index + 1].uv = uv4;		m_vb[vb_index + 1].color = tiledata.color;
				m_vb[vb_index + 2].pos = p3;	m_vb[vb_index + 2].uv = uv3;		m_vb[vb_index + 2].color = tiledata.color;

				m_vb[vb_index + 3].pos = p1;	m_vb[vb_index + 3].uv = uv1;		m_vb[vb_index + 3].color = tiledata.color;
				m_vb[vb_index + 4].pos = p3;	m_vb[vb_index + 4].uv = uv3;		m_vb[vb_index + 4].color = tiledata.color;
				m_vb[vb_index + 5].pos = p2;	m_vb[vb_index + 5].uv = uv2;		m_vb[vb_index + 5].color = tiledata.color;

				// degen
				m_vb[vb_index + 6].pos = p2;	m_vb[vb_index + 6].uv = uv2;		m_vb[vb_index + 6].color = tiledata.color;
				m_vb[vb_index + 7].pos = p2;	m_vb[vb_index + 7].uv = uv2;		m_vb[vb_index + 7].color = tiledata.color;
				m_vb[vb_index + 8].pos = p2;	m_vb[vb_index + 8].uv = uv2;		m_vb[vb_index + 8].color = tiledata.color;
				m_vb[vb_index + 9].pos = p2;	m_vb[vb_index + 9].uv = uv2;		m_vb[vb_index + 9].color = tiledata.color;
				m_vb[vb_index + 10].pos = p2;	m_vb[vb_index + 10].uv = uv2;		m_vb[vb_index + 10].color = tiledata.color;
				m_vb[vb_index + 11].pos = p2;	m_vb[vb_index + 11].uv = uv2;		m_vb[vb_index + 11].color = tiledata.color;

				break;
			}
			case Tilemap::TILE_RIGHT:
			{
				/*
				    |\
				    | \
			        |  |
			        |  |
			        | /
			        |/
				*/

				m_vb[vb_index + 0].pos = p6;	m_vb[vb_index + 0].uv = uv2;		m_vb[vb_index + 0].color = tiledata.color;
				m_vb[vb_index + 1].pos = p5;	m_vb[vb_index + 1].uv = uv1;		m_vb[vb_index + 1].color = tiledata.color;
				m_vb[vb_index + 2].pos = p4;	m_vb[vb_index + 2].uv = uv4;		m_vb[vb_index + 2].color = tiledata.color;

				m_vb[vb_index + 3].pos = p6;	m_vb[vb_index + 3].uv = uv2;		m_vb[vb_index + 3].color = tiledata.color;
				m_vb[vb_index + 4].pos = p4;	m_vb[vb_index + 4].uv = uv4;		m_vb[vb_index + 4].color = tiledata.color;
				m_vb[vb_index + 5].pos = p3;	m_vb[vb_index + 5].uv = uv3;		m_vb[vb_index + 5].color = tiledata.color;

				// degen
				m_vb[vb_index + 6].pos = p3;	m_vb[vb_index + 6].uv = uv3;		m_vb[vb_index + 6].color = tiledata.color;
				m_vb[vb_index + 7].pos = p3;	m_vb[vb_index + 7].uv = uv3;		m_vb[vb_index + 7].color = tiledata.color;
				m_vb[vb_index + 8].pos = p3;	m_vb[vb_index + 8].uv = uv3;		m_vb[vb_index + 8].color = tiledata.color;
				m_vb[vb_index + 9].pos = p3;	m_vb[vb_index + 9].uv = uv3;		m_vb[vb_index + 9].color = tiledata.color;
				m_vb[vb_index + 10].pos = p3;	m_vb[vb_index + 10].uv = uv3;		m_vb[vb_index + 10].color = tiledata.color;
				m_vb[vb_index + 11].pos = p3;	m_vb[vb_index + 11].uv = uv3;		m_vb[vb_index + 11].color = tiledata.color;
				break;
			}
			case Tilemap::TILE_TOP:
			{
				/* 
			        /\
			       /__\
				*/

				m_vb[vb_index + 0].pos = p1;	m_vb[vb_index + 0].uv = uv1;		m_vb[vb_index + 0].color = tiledata.color;
				m_vb[vb_index + 1].pos = p6;	m_vb[vb_index + 1].uv = uv3;		m_vb[vb_index + 1].color = tiledata.color;
				m_vb[vb_index + 2].pos = p5;	m_vb[vb_index + 2].uv = uv2;		m_vb[vb_index + 2].color = tiledata.color;

				// degen
				m_vb[vb_index + 3].pos = p5;	m_vb[vb_index + 3].uv = uv5;		m_vb[vb_index + 3].color = tiledata.color;
				m_vb[vb_index + 4].pos = p5;	m_vb[vb_index + 4].uv = uv5;		m_vb[vb_index + 4].color = tiledata.color;
				m_vb[vb_index + 5].pos = p5;	m_vb[vb_index + 5].uv = uv5;		m_vb[vb_index + 5].color = tiledata.color;
				m_vb[vb_index + 6].pos = p5;	m_vb[vb_index + 6].uv = uv5;		m_vb[vb_index + 6].color = tiledata.color;
				m_vb[vb_index + 7].pos = p5;	m_vb[vb_index + 7].uv = uv5;		m_vb[vb_index + 7].color = tiledata.color;
				m_vb[vb_index + 8].pos = p5;	m_vb[vb_index + 8].uv = uv5;		m_vb[vb_index + 8].color = tiledata.color;
				m_vb[vb_index + 9].pos = p5;	m_vb[vb_index + 9].uv = uv5;		m_vb[vb_index + 9].color = tiledata.color;
				m_vb[vb_index + 10].pos = p5;	m_vb[vb_index + 10].uv = uv5;		m_vb[vb_index + 10].color = tiledata.color;
				m_vb[vb_index + 11].pos = p5;	m_vb[vb_index + 11].uv = uv5;		m_vb[vb_index + 11].color = tiledata.color;
				break;
			}
			case Tilemap::TILE_BOTTOM:
			{
				/* ____
				   \  /
			        \/
				*/

				m_vb[vb_index + 0].pos = p2;	m_vb[vb_index + 0].uv = uv1;		m_vb[vb_index + 0].color = tiledata.color;
				m_vb[vb_index + 1].pos = p4;	m_vb[vb_index + 1].uv = uv3;		m_vb[vb_index + 1].color = tiledata.color;
				m_vb[vb_index + 2].pos = p3;	m_vb[vb_index + 2].uv = uv2;		m_vb[vb_index + 2].color = tiledata.color;

				// degen
				m_vb[vb_index + 3].pos = p3;	m_vb[vb_index + 3].uv = uv3;		m_vb[vb_index + 3].color = tiledata.color;
				m_vb[vb_index + 4].pos = p3;	m_vb[vb_index + 4].uv = uv3;		m_vb[vb_index + 4].color = tiledata.color;
				m_vb[vb_index + 5].pos = p3;	m_vb[vb_index + 5].uv = uv3;		m_vb[vb_index + 5].color = tiledata.color;
				m_vb[vb_index + 6].pos = p3;	m_vb[vb_index + 6].uv = uv3;		m_vb[vb_index + 6].color = tiledata.color;
				m_vb[vb_index + 7].pos = p3;	m_vb[vb_index + 7].uv = uv3;		m_vb[vb_index + 7].color = tiledata.color;
				m_vb[vb_index + 8].pos = p3;	m_vb[vb_index + 8].uv = uv3;		m_vb[vb_index + 8].color = tiledata.color;
				m_vb[vb_index + 9].pos = p3;	m_vb[vb_index + 9].uv = uv3;		m_vb[vb_index + 9].color = tiledata.color;
				m_vb[vb_index + 10].pos = p3;	m_vb[vb_index + 10].uv = uv3;		m_vb[vb_index + 10].color = tiledata.color;
				m_vb[vb_index + 11].pos = p3;	m_vb[vb_index + 11].uv = uv3;		m_vb[vb_index + 11].color = tiledata.color;
				break;
			}
			case Tilemap::TILE_MID:
			{
				/*______
			      |    |
			      |____|
				*/

				m_vb[vb_index + 0].pos = p1;	m_vb[vb_index + 0].uv = uv1;		m_vb[vb_index + 0].color = tiledata.color;
				m_vb[vb_index + 1].pos = p5;	m_vb[vb_index + 1].uv = uv4;		m_vb[vb_index + 1].color = tiledata.color;
				m_vb[vb_index + 2].pos = p4;	m_vb[vb_index + 2].uv = uv3;		m_vb[vb_index + 2].color = tiledata.color;
				
				m_vb[vb_index + 3].pos = p1;	m_vb[vb_index + 3].uv = uv1;		m_vb[vb_index + 3].color = tiledata.color;
				m_vb[vb_index + 4].pos = p4;	m_vb[vb_index + 4].uv = uv3;		m_vb[vb_index + 4].color = tiledata.color;
				m_vb[vb_index + 5].pos = p2;	m_vb[vb_index + 5].uv = uv2;		m_vb[vb_index + 5].color = tiledata.color;
				
				// degen
				m_vb[vb_index + 6].pos = p2;	m_vb[vb_index + 6].uv = uv2;		m_vb[vb_index + 6].color = tiledata.color;
				m_vb[vb_index + 7].pos = p2;	m_vb[vb_index + 7].uv = uv2;		m_vb[vb_index + 7].color = tiledata.color;
				m_vb[vb_index + 8].pos = p2;	m_vb[vb_index + 8].uv = uv2;		m_vb[vb_index + 8].color = tiledata.color;
				m_vb[vb_index + 9].pos = p2;	m_vb[vb_index + 9].uv = uv2;		m_vb[vb_index + 9].color = tiledata.color;
				m_vb[vb_index + 10].pos = p2;	m_vb[vb_index + 10].uv = uv2;		m_vb[vb_index + 10].color = tiledata.color;
				m_vb[vb_index + 11].pos = p2;	m_vb[vb_index + 11].uv = uv2;		m_vb[vb_index + 11].color = tiledata.color;
				break;
			}

			default:
			{
				// make degen geo
				for (int i = 0; i < num_verts; i++)
				{
					m_vb[vb_index + i].pos = glm::vec3(0, 0, 0);
					m_vb[vb_index + i].uv = glm::vec2(0, 0);
					m_vb[vb_index + i].color = 0;
				}
				break;
			}
		}
	}
}

float* Tilemap::getVBO()
{
	return (float*)m_vb;
}

int Tilemap::numTris()
{
	return m_width * m_height * 4;
}

int Tilemap::get(int x, int y)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	return m_map[(y * m_width) + x];
}

void Tilemap::edit(int x, int y, int tile)
{
	assert(x >= 0 && x < m_width);
	assert(y >= 0 && y < m_height);

	m_map[(y * m_width) + x] = tile;

	bool odd = y & 1;
	tesselateTile(x, y, odd);
}

const Tilemap::Config& Tilemap::getConfig()
{
	return m_config;
}


int Tilemap::insertTile(std::string name, PolygonDef* top, PolygonDef* side, unsigned int color, Tilemap::TileType type)
{
	Tile tile;
	for (int i=0; i < top->getNumPoints(); i++)
	{
		tile.top_points[i] = top->getPoint(i);
	}

	/*
	     p6
	p1         p5
	p2         p4
	     p3
	*/
	/*
	switch (type)
	{
		case TILE_FULL:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[1] = top->getPoint(1);
			tile.top_points[2] = top->getPoint(2);
			tile.top_points[3] = top->getPoint(3);
			tile.top_points[4] = top->getPoint(4);
			tile.top_points[5] = top->getPoint(5);
			break;
		}
		case TILE_LEFT:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[1] = top->getPoint(1);
			tile.top_points[2] = top->getPoint(2);
			tile.top_points[5] = top->getPoint(3);
			break;
		}
		case TILE_RIGHT:
		{
			tile.top_points[4] = top->getPoint(0);
			tile.top_points[5] = top->getPoint(1);
			tile.top_points[2] = top->getPoint(2);
			tile.top_points[3] = top->getPoint(3);
			break;
		}
		case TILE_TOP:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[4] = top->getPoint(1);
			tile.top_points[5] = top->getPoint(2);
			break;
		}
		case TILE_BOTTOM:
		{
			tile.top_points[1] = top->getPoint(0);
			tile.top_points[2] = top->getPoint(1);
			tile.top_points[3] = top->getPoint(2);
			break;
		}
		case TILE_MID:
		{
			tile.top_points[0] = top->getPoint(0);
			tile.top_points[1] = top->getPoint(1);
			tile.top_points[3] = top->getPoint(2);
			tile.top_points[4] = top->getPoint(3);
			break;
		}
	}
	*/

	tile.side_bits = 0;

	switch (type)
	{
		case TILE_FULL:
		{
			tile.side_bits |= SIDE_LEFT;
			tile.side_bits |= SIDE_TOP_LEFT;
			tile.side_bits |= SIDE_TOP_RIGHT;
			tile.side_bits |= SIDE_RIGHT;
			tile.side_bits |= SIDE_BOT_RIGHT;
			tile.side_bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_LEFT:
		{
			tile.side_bits |= SIDE_LEFT;
			tile.side_bits |= SIDE_TOP_LEFT;
			tile.side_bits |= SIDE_BOT_LEFT;
			tile.side_bits |= SIDE_MID;
			break;
		}
		case TILE_RIGHT:
		{
			tile.side_bits |= SIDE_RIGHT;
			tile.side_bits |= SIDE_TOP_RIGHT;
			tile.side_bits |= SIDE_BOT_RIGHT;
			tile.side_bits |= SIDE_MID;
			break;
		}
		case TILE_TOP:
		{
			tile.side_bits |= SIDE_TOP_RIGHT;
			tile.side_bits |= SIDE_TOP_LEFT;
			break;
		}
		case TILE_BOTTOM:
		{
			tile.side_bits |= SIDE_BOT_RIGHT;
			tile.side_bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_MID:
		{
			tile.side_bits |= SIDE_LEFT;
			tile.side_bits |= SIDE_RIGHT;
			break;
		}
	}


	for (int i = 0; i < side->getNumPoints(); i++)
	{
		tile.side_points[i] = side->getPoint(i);
	}
	tile.name = name;
	tile.id = m_cumulative_tile_id;
	m_cumulative_tile_id++;

	tile.color = color;
	tile.type = type;

	m_tiles.push_back(tile);
	return tile.id;
}

bool Tilemap::removeTile(int id)
{
	int num_tiles = m_tiles.size();
	for (int i=0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			m_tiles.erase(m_tiles.begin() + i);
			return true;
		}
	}

	return false;
}

void Tilemap::removeTiles()
{
	m_tiles.clear();
}

int Tilemap::getNumTiles()
{
	return m_tiles.size();
}

const Tilemap::Tile* Tilemap::getTile(int index)
{
	assert(index >= 0 && index < m_tiles.size());

	return &m_tiles.at(index);
}

const Tilemap::Tile* Tilemap::getTileById(int id)
{
	int num_tiles = m_tiles.size();
	for (int i=0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			return tile;
		}
	}
	return nullptr;
}

int Tilemap::getTileIndexById(int id)
{
	int num_tiles = m_tiles.size();
	for (int i=0; i < num_tiles; i++)
	{
		Tile* tile = &m_tiles.at(i);
		if (tile->id == id)
		{
			return i;
		}
	}
	return -1;
}