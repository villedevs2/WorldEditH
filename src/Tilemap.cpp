#include "Tilemap.h"

Tilemap::Tilemap()
{
	m_tile_width = 1.0f;
	m_tile_height = 1.4f;

	m_cumulative_tile_id = 1;

	m_buckets = new Bucket*[(AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT)];
	int size = (AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT);
	for (int i = 0; i < size; i++)
	{
		m_buckets[i] = nullptr;
	}
}

Tilemap::~Tilemap()
{
	if (m_buckets != nullptr)
	{
		int size = (AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT);
		for (int i = 0; i < size; i++)
		{
			if (m_buckets[i] != nullptr)
				deallocBucket(i);
		}

		delete[] m_buckets;
	}
}

void Tilemap::reset()
{
	removeTiles();

	int size = (AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT);
	for (int i = 0; i < size; i++)
	{
		if (m_buckets[i] != nullptr)
			deallocBucket(i);
	}
}

float Tilemap::getTileWidth()
{
	return m_tile_width;
}

float Tilemap::getTileHeight()
{
	return m_tile_height;
}

void Tilemap::tesselateTile(Bucket* bucket, int bx, int by)
{
	assert(bx >= 0 && bx < BUCKET_WIDTH);
	assert(by >= 0 && by < BUCKET_HEIGHT);

	VBO* vbotile = bucket->tiles;
	VBO* vbo3d = bucket->preview;
	float z = ((bucket->map[(by * BUCKET_WIDTH) + bx] & Z_MASK) >> Z_SHIFT) * 0.1f;

	int ctile = bucket->map[(by * BUCKET_WIDTH) + bx] & TILE_MASK;

	int vbotile_index = ((by * BUCKET_WIDTH) + bx) * 4;
	int vbo3d_index = ((by * BUCKET_WIDTH) + bx) * 16;

	float tx1 = (float)((bucket->x * BUCKET_WIDTH) + bx) * m_tile_width;
	float tx2 = tx1 + m_tile_width;
	float ty1 = (float)((bucket->y * BUCKET_HEIGHT) + by) * (m_tile_height / 2);
	float ty2 = ty1 + (m_tile_height / 2);

	if (by & 1)
	{
		tx1 += m_tile_width / 2;
		tx2 += m_tile_width / 2;
	}

	if (ctile == Tilemap::TILE_EMPTY)
	{
		// make degen geo
		vbotile->degenTris(vbotile_index, 4);
		vbo3d->degenTris(vbo3d_index, 16);
	}
	else
	{
		const Tilemap::Tile* tiledata = getTile(ctile);

		glm::vec2 uv1 = tiledata->top_points[0];
		glm::vec2 uv2 = tiledata->top_points[1];
		glm::vec2 uv3 = tiledata->top_points[2];
		glm::vec2 uv4 = tiledata->top_points[3];
		glm::vec2 uv5 = tiledata->top_points[4];
		glm::vec2 uv6 = tiledata->top_points[5];

		glm::vec2 suv1 = tiledata->side_points[0];
		glm::vec2 suv2 = tiledata->side_points[1];
		glm::vec2 suv3 = tiledata->side_points[2];
		glm::vec2 suv4 = tiledata->side_points[3];

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

		glm::vec3 bp1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), 0.0f);
		glm::vec3 bp2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), 0.0f);
		glm::vec3 bp3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), 0.0f);
		glm::vec3 bp4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), 0.0f);
		glm::vec3 bp5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), 0.0f);
		glm::vec3 bp6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, 0.0f);

		glm::vec3 tp1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), 100.0f);
		glm::vec3 tp2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), 100.0f);
		glm::vec3 tp3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), 100.0f);
		glm::vec3 tp4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), 100.0f);
		glm::vec3 tp5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), 100.0f);
		glm::vec3 tp6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, 100.0f);

		enum
		{
			RENDER_LEFT = 0x1,
			RENDER_TOPLEFT = 0x2,
			RENDER_TOPRIGHT = 0x4,
			RENDER_RIGHT = 0x8,
			RENDER_BOTRIGHT = 0x10,
			RENDER_BOTLEFT = 0x20,
			RENDER_SIDELEFT = 0x40,
			RENDER_SIDERIGHT = 0x80,
			RENDER_MIDTOP = 0x100,
			RENDER_MIDBOT = 0x200,
			RENDER_CORNER_TL = 0x400,
			RENDER_CORNER_TR = 0x800,
			RENDER_CORNER_BL = 0x1000,
			RENDER_CORNER_BR = 0x2000,
			RENDER_CENTER_TOP = 0x4000,
			RENDER_CENTER_BOT = 0x8000,
		};

		int render_sides = 0;

		switch (tiledata->type)
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

				vbotile->makeTri(vbotile_index + 0, tp1, tp6, tp5, uv1, uv6, uv5, tiledata->color);
				vbotile->makeTri(vbotile_index + 1, tp1, tp5, tp4, uv1, uv5, uv4, tiledata->color);
				vbotile->makeTri(vbotile_index + 2, tp1, tp4, tp3, uv1, uv4, uv3, tiledata->color);
				vbotile->makeTri(vbotile_index + 3, tp1, tp3, tp2, uv1, uv3, uv2, tiledata->color);

				vbo3d->makeTri(vbo3d_index++, p1, p6, p5, uv1, uv6, uv5, tiledata->color);
				vbo3d->makeTri(vbo3d_index++, p1, p5, p4, uv1, uv5, uv4, tiledata->color);
				vbo3d->makeTri(vbo3d_index++, p1, p4, p3, uv1, uv4, uv3, tiledata->color);
				vbo3d->makeTri(vbo3d_index++, p1, p3, p2, uv1, uv3, uv2, tiledata->color);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_BOTLEFT;
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
				vbotile->makeTri(vbotile_index + 0, tp1, tp6, tp3, uv1, uv4, uv3, tiledata->color);
				vbotile->makeTri(vbotile_index + 1, tp1, tp3, tp2, uv1, uv3, uv2, tiledata->color);
				vbotile->degenTris(vbotile_index + 2, 2);

				vbo3d->makeTri(vbo3d_index++, p1, p6, p3, uv1, uv4, uv3, tiledata->color);
				vbo3d->makeTri(vbo3d_index++, p1, p3, p2, uv1, uv3, uv2, tiledata->color);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_BOTLEFT;
				render_sides |= RENDER_SIDELEFT;
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
				vbotile->makeTri(vbotile_index + 0, tp6, tp5, tp4, uv2, uv1, uv4, tiledata->color);
				vbotile->makeTri(vbotile_index + 1, tp6, tp4, tp3, uv2, uv4, uv3, tiledata->color);
				vbotile->degenTris(vbotile_index + 2, 2);
				
				vbo3d->makeTri(vbo3d_index++, p6, p5, p4, uv2, uv1, uv4, tiledata->color);
				vbo3d->makeTri(vbo3d_index++, p6, p4, p3, uv2, uv4, uv3, tiledata->color);

				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_SIDERIGHT;
				break;
			}
			case Tilemap::TILE_TOP:
			{
				/*
				     /\
				    /__\
				*/
				vbotile->makeTri(vbotile_index + 0, tp1, tp6, tp5, uv1, uv3, uv2, tiledata->color);
				vbotile->degenTris(vbotile_index + 1, 3);

				vbo3d->makeTri(vbo3d_index++, p1, p6, p5, uv1, uv3, uv2, tiledata->color);

				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_MIDTOP;
				break;
			}
			case Tilemap::TILE_BOTTOM:
			{
				/*  ____
				    \  /
				     \/
				*/
				vbotile->makeTri(vbotile_index + 0, tp2, tp4, tp3, uv1, uv3, uv2, tiledata->color);
				vbotile->degenTris(vbotile_index + 1, 3);

				vbo3d->makeTri(vbo3d_index++, p2, p4, p3, uv1, uv3, uv2, tiledata->color);				

				render_sides |= RENDER_BOTLEFT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_MIDBOT;
				break;
			}
			case Tilemap::TILE_MID:
			{
				/*  ______
				    |    |
				    |____|
				*/
				vbotile->makeTri(vbotile_index + 0, tp1, tp5, tp4, uv1, uv4, uv3, tiledata->color);
				vbotile->makeTri(vbotile_index + 1, tp1, tp4, tp2, uv1, uv3, uv2, tiledata->color);
				vbotile->degenTris(vbotile_index + 2, 2);

				vbo3d->makeTri(vbo3d_index++, p1, p5, p4, uv1, uv4, uv3, tiledata->color);
				vbo3d->makeTri(vbo3d_index++, p1, p4, p2, uv1, uv3, uv2, tiledata->color);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_CENTER_TOP;
				render_sides |= RENDER_CENTER_BOT;
				break;
			}
			case Tilemap::TILE_CORNER_TL:
			{
				/*
				      /.
				     / .
				    | . 
				    |.  				   
				*/
				vbotile->makeTri(vbotile_index + 0, tp1, tp6, tp2, uv1, uv2, uv3, tiledata->color);
				vbotile->degenTris(vbotile_index + 1, 3);

				vbo3d->makeTri(vbo3d_index++, p1, p6, p2, uv1, uv2, uv3, tiledata->color);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_CORNER_TL;
				break;
			}
			case Tilemap::TILE_CORNER_TR:
			{
				/*
				    .\
				    . \
				     . |
				      .|				      
				*/
				vbotile->makeTri(vbotile_index + 0, tp6, tp5, tp4, uv1, uv2, uv3, tiledata->color);
				vbotile->degenTris(vbotile_index + 1, 3);

				vbo3d->makeTri(vbo3d_index++, p6, p5, p4, uv1, uv2, uv3, tiledata->color);

				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_CORNER_TR;
				break;
			}
			case Tilemap::TILE_CORNER_BL:
			{
				/*
				    |.  
				    | . 
				     \ .
				      \.
				*/
				vbotile->makeTri(vbotile_index + 0, tp1, tp3, tp2, uv1, uv2, uv3, tiledata->color);
				vbotile->degenTris(vbotile_index + 1, 3);

				vbo3d->makeTri(vbo3d_index++, p1, p3, p2, uv1, uv2, uv3, tiledata->color);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_BOTLEFT;
				render_sides |= RENDER_CORNER_BL;
				break;
			}
			case Tilemap::TILE_CORNER_BR:
			{
				/*
			  	      .|
				     . |
				    . /
				    ./
				*/
				vbotile->makeTri(vbotile_index + 0, tp3, tp5, tp4, uv1, uv2, uv3, tiledata->color);
				vbotile->degenTris(vbotile_index + 1, 3);

				vbo3d->makeTri(vbo3d_index++, p3, p5, p4, uv1, uv2, uv3, tiledata->color);

				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_CORNER_BR;
				break;
			}

			default:
			{
				// make degen geo
				vbotile->degenTris(vbotile_index, 4);
				vbo3d->degenTris(vbo3d_index, 16);
				break;
			}
		}

		if (render_sides & RENDER_LEFT)		
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p1, p2, bp2, bp1, suv1, suv2, suv3, suv4, tiledata->color);		
		if (render_sides & RENDER_TOPLEFT)		
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p6, p1, bp1, bp6, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_TOPRIGHT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p5, p6, bp6, bp5, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_RIGHT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p4, p5, bp5, bp4, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_BOTRIGHT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p3, p4, bp4, bp3, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_BOTLEFT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p2, p3, bp3, bp2, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_SIDELEFT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p3, p6, bp6, bp3, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_SIDERIGHT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p6, p3, bp3, bp6, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_MIDTOP)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p1, p5, bp5, bp1, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_MIDBOT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p4, p2, bp2, bp4, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_CENTER_TOP)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p5, p1, bp1, bp5, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_CENTER_BOT)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p2, p4, bp4, bp2, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_CORNER_TL)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p2, p6, bp6, bp2, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_CORNER_TR)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p6, p4, bp4, bp6, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_CORNER_BL)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p3, p1, bp1, bp3, suv1, suv2, suv3, suv4, tiledata->color);
		if (render_sides & RENDER_CORNER_BR)
			vbo3d_index += vbo3d->makeQuad(vbo3d_index, p5, p3, bp3, bp5, suv1, suv2, suv3, suv4, tiledata->color);

		if (vbo3d_index < 16)
			vbo3d->degenTris(vbo3d_index, 16 - vbo3d_index);
	}
}

float* Tilemap::getVBO(int bx, int by)
{
	int bin = (by * (AREA_WIDTH / BUCKET_WIDTH)) + bx;

	if (m_buckets[bin] != nullptr)
	{
		return (float*)m_buckets[bin]->tiles->getPointer();
	}
	else
	{
		return nullptr;
	}
}

int Tilemap::numTris(int bx, int by)
{
	int bin = (by * (AREA_WIDTH / BUCKET_WIDTH)) + bx;

	if (m_buckets[bin] != nullptr)
	{
		return m_buckets[bin]->tiles->getCapacity();
	}
	else
	{
		return 0;
	}
}

int Tilemap::get(int x, int y)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return m_buckets[bin]->map[(iy * BUCKET_WIDTH) + ix] & TILE_MASK;
	}
	else
	{
		return TILE_EMPTY;
	}
}

int Tilemap::getZ(int x, int y)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return (m_buckets[bin]->map[(iy * BUCKET_WIDTH) + ix] & Z_MASK) >> Z_SHIFT;
	}
	else
	{
		return 0;
	}
}

void Tilemap::edit(int x, int y, int tile)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);

	if (m_buckets[bin] == nullptr)
	{
		allocBucket(bin);
	}

	int ix = x % BUCKET_WIDTH;
	int iy = y % BUCKET_HEIGHT;

	int index = (iy * BUCKET_WIDTH) + ix;

	m_buckets[bin]->map[index] &= ~TILE_MASK;
	m_buckets[bin]->map[index] |= tile & TILE_MASK;

	// if empty tile, reset Z as well
	if (tile == TILE_EMPTY)
	{
		m_buckets[bin]->map[index] &= ~Z_MASK;
	}

	// mark tile
	if (tile != TILE_EMPTY)
		m_buckets[bin]->coverage |= uint64_t(1) << index;
	else
		m_buckets[bin]->coverage &= ~(uint64_t(1) << index);

	tesselateTile(m_buckets[bin], ix, iy);

	if (m_buckets[bin]->coverage == 0)
	{
		deallocBucket(bin);
	}
}

void Tilemap::editZ(int x, int y, int z)
{
	assert(x >= 0 && x < AREA_WIDTH);
	assert(y >= 0 && y < AREA_HEIGHT);

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);

	if (m_buckets[bin] != nullptr)		// do nothing if not mapped
	{
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;

		int index = (iy * BUCKET_WIDTH) + ix;

		m_buckets[bin]->map[index] &= ~Z_MASK;
		m_buckets[bin]->map[index] |= (z << Z_SHIFT) & Z_MASK;

		tesselateTile(m_buckets[bin], ix, iy);
	}
}

int Tilemap::getSideBits(Tilemap::TileType type)
{
	int bits = 0;

	switch (type)
	{
		case TILE_FULL:
		{
			bits |= SIDE_LEFT;
			bits |= SIDE_TOP_LEFT;
			bits |= SIDE_TOP_RIGHT;
			bits |= SIDE_RIGHT;
			bits |= SIDE_BOT_RIGHT;
			bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_LEFT:
		{
			bits |= SIDE_LEFT;
			bits |= SIDE_TOP_LEFT;
			bits |= SIDE_BOT_LEFT;
			bits |= SIDE_MID;
			break;
		}
		case TILE_RIGHT:
		{
			bits |= SIDE_RIGHT;
			bits |= SIDE_TOP_RIGHT;
			bits |= SIDE_BOT_RIGHT;
			bits |= SIDE_MID;
			break;
		}
		case TILE_TOP:
		{
			bits |= SIDE_TOP_RIGHT;
			bits |= SIDE_TOP_LEFT;
			break;
		}
		case TILE_BOTTOM:
		{
			bits |= SIDE_BOT_RIGHT;
			bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_MID:
		{
			bits |= SIDE_LEFT;
			bits |= SIDE_RIGHT;
			break;
		}
		case TILE_CORNER_TL:
		{
			bits |= SIDE_LEFT;
			bits |= SIDE_TOP_LEFT;
			break;
		}
		case TILE_CORNER_TR:
		{
			bits |= SIDE_RIGHT;
			bits |= SIDE_TOP_RIGHT;
			break;
		}
		case TILE_CORNER_BL:
		{
			bits |= SIDE_LEFT;
			bits |= SIDE_BOT_LEFT;
			break;
		}
		case TILE_CORNER_BR:
		{
			bits |= SIDE_RIGHT;
			bits |= SIDE_BOT_RIGHT;
			break;
		}
	}

	return bits;
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

	tile.side_bits = getSideBits(type);

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

int Tilemap::replaceTile(int index, std::string name, PolygonDef* top, PolygonDef* side, unsigned int color, Tilemap::TileType type)
{
	Tile* tile = &m_tiles.at(index);
	for (int i = 0; i < top->getNumPoints(); i++)
	{
		tile->top_points[i] = top->getPoint(i);
	}

	tile->side_bits = getSideBits(type);

	for (int i = 0; i < side->getNumPoints(); i++)
	{
		tile->side_points[i] = side->getPoint(i);
	}
	tile->name = name;

	tile->color = color;
	tile->type = type;

	tesselateAllByTile(index);

	return tile->id;
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

Tilemap::Tile* Tilemap::getTile(int index)
{
	assert(index >= 0 && index < m_tiles.size());

	return &m_tiles.at(index);
}

Tilemap::Tile* Tilemap::getTileById(int id)
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

void Tilemap::allocBucket(int bin)
{
	assert(m_buckets[bin] == nullptr);

	int bx = bin % (AREA_WIDTH / BUCKET_WIDTH);
	int by = bin / (AREA_WIDTH / BUCKET_WIDTH);

	m_buckets[bin] = new Bucket();

	m_buckets[bin]->coverage = 0;
	m_buckets[bin]->map = new unsigned int[BUCKET_WIDTH * BUCKET_HEIGHT];
	m_buckets[bin]->tiles = new VBO(BUCKET_WIDTH * BUCKET_HEIGHT * 4);
	m_buckets[bin]->preview = new VBO(BUCKET_WIDTH * BUCKET_HEIGHT * 16);
	m_buckets[bin]->x = bx;
	m_buckets[bin]->y = by;
	
	for (int i = 0; i < BUCKET_WIDTH*BUCKET_HEIGHT; i++)
	{
		m_buckets[bin]->map[i] = TILE_EMPTY;

		m_buckets[bin]->tiles->degenTris(i * 4, 4);
		m_buckets[bin]->preview->degenTris(i * 16, 16);
	}
}

void Tilemap::deallocBucket(int bin)
{
	assert(m_buckets[bin] != nullptr);

	delete[] m_buckets[bin]->map;
	delete m_buckets[bin]->tiles;
	delete m_buckets[bin]->preview;
	delete m_buckets[bin];

	m_buckets[bin] = nullptr;
}

Tilemap::Bucket* Tilemap::getTileBucket(int bx, int by)
{
	assert(bx >= 0 && bx < (AREA_WIDTH / BUCKET_WIDTH));
	assert(by >= 0 && by < (AREA_HEIGHT / BUCKET_HEIGHT));

	return m_buckets[(by * (AREA_WIDTH / BUCKET_WIDTH)) + bx];
}

Tilemap::Bucket* Tilemap::getTileBucket(int index)
{
	assert(index >= 0 && index < ((AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT)));

	return m_buckets[index];
}

void Tilemap::tesselateAllByTile(int tile)
{
	int total_buckets = (AREA_WIDTH / BUCKET_WIDTH) * (AREA_HEIGHT / BUCKET_HEIGHT);
	for (int b = 0; b < total_buckets; b++)
	{
		if (m_buckets[b] != nullptr)
		{
			for (int y = 0; y < BUCKET_HEIGHT; y++)
			{
				for (int x = 0; x < BUCKET_WIDTH; x++)
				{
					int ctile = m_buckets[b]->map[y * BUCKET_WIDTH + x] & TILE_MASK;
					if (ctile == tile)
					{
						tesselateTile(m_buckets[b], x, y);
					}
				}
			}
		}
	}
}