#include "Tilemap.h"

Tilemap::Tilemap(Tileset* tileset, Tilemap::EditCallback* edit_callback, float zbase, float zbase_height, unsigned int flags)
{
	m_tileset = tileset;
	m_edit_callback = edit_callback;

	m_tile_width = 1.0f;
	m_tile_height = 1.4f;

	m_zbase = zbase;
	m_zbase_height = zbase_height;
	m_flags = flags;

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

unsigned int Tilemap::getTileColor(unsigned int basecolor, float lum)
{
	int r = (basecolor >> 16) & 0xff;
	int g = (basecolor >> 8) & 0xff;
	int b = basecolor & 0xff;

	r = (float)(r) * lum;
	g = (float)(g) * lum;
	b = (float)(b) * lum;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;

	return (basecolor & 0xff000000) | (r << 16) | (g << 8) | (b);
}

void Tilemap::tesselateTile(Bucket* bucket, int bx, int by)
{
	assert(bx >= 0 && bx < BUCKET_WIDTH);
	assert(by >= 0 && by < BUCKET_HEIGHT);


	AdjacentTiles adjacent_tiles;
	getAdjacentTileCoords(&adjacent_tiles, (bucket->x * BUCKET_WIDTH) + bx, (bucket->y * BUCKET_HEIGHT) + by);

	int z_current = (bucket->map[(by * BUCKET_WIDTH) + bx] & Z_MASK) >> Z_SHIFT;

	int z_left = getZ(adjacent_tiles.left.x, adjacent_tiles.left.y);
	int z_right = getZ(adjacent_tiles.right.x, adjacent_tiles.right.y);
	int z_topleft = getZ(adjacent_tiles.topleft.x, adjacent_tiles.topleft.y);
	int z_topright = getZ(adjacent_tiles.topright.x, adjacent_tiles.topright.y);
	int z_botleft = getZ(adjacent_tiles.botleft.x, adjacent_tiles.botleft.y);
	int z_botright = getZ(adjacent_tiles.botright.x, adjacent_tiles.botright.y);

	int ao_tile = 0;

	const int OCCLUSION_THRESHOLD = 10;

	if ((z_left - z_current) >= OCCLUSION_THRESHOLD)
		ao_tile |= AmbientOcclusion::SIDE_LEFT;
	if ((z_right - z_current) >= OCCLUSION_THRESHOLD)
		ao_tile |= AmbientOcclusion::SIDE_RIGHT;
	if ((z_topleft - z_current) >= OCCLUSION_THRESHOLD)
		ao_tile |= AmbientOcclusion::SIDE_TOPLEFT;
	if ((z_topright - z_current) >= OCCLUSION_THRESHOLD)
		ao_tile |= AmbientOcclusion::SIDE_TOPRIGHT;
	if ((z_botleft - z_current) >= OCCLUSION_THRESHOLD)
		ao_tile |= AmbientOcclusion::SIDE_BOTLEFT;
	if ((z_botright - z_current) >= OCCLUSION_THRESHOLD)
		ao_tile |= AmbientOcclusion::SIDE_BOTRIGHT;

	int ao_tile_x = ao_tile % AmbientOcclusion::FLOOR_TILES_X;
	int ao_tile_y = ao_tile / AmbientOcclusion::FLOOR_TILES_Y;

	float amb_tile_w = 1.0 / 8.0;
	float amb_tile_h = 1.0 / 8.0;
	float amb_tile_x = ao_tile_x * amb_tile_w;
	float amb_tile_y = ao_tile_y * amb_tile_h;


	VBO* vbo = bucket->tiles;
	float z = z_current * 0.1f;

	int ctile = bucket->map[(by * BUCKET_WIDTH) + bx] & TILE_MASK;

	int vbo_index = ((by * BUCKET_WIDTH) + bx) * MAX_VERTS;

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
		vbo->degenTris(vbo_index, MAX_VERTS);
	}
	else
	{
		const Tileset::Tile* tiledata = m_tileset->getTile(ctile);

		glm::vec2 uv1 = tiledata->top_points[0];
		glm::vec2 uv2 = tiledata->top_points[1];
		glm::vec2 uv3 = tiledata->top_points[2];
		glm::vec2 uv4 = tiledata->top_points[3];
		glm::vec2 uv5 = tiledata->top_points[4];
		glm::vec2 uv6 = tiledata->top_points[5];

		glm::vec2 uvcen = glm::mix(uv1 + ((uv5 - uv1) * 0.5f), uv2 + ((uv4 - uv2) * 0.5f), 0.5f);

		glm::vec2 suv1 = tiledata->side_points[0];
		glm::vec2 suv2 = tiledata->side_points[1];
		glm::vec2 suv3 = tiledata->side_points[2];
		glm::vec2 suv4 = tiledata->side_points[3];

		/*
		glm::vec3 uva1 = glm::vec3(0.0f, 0.0f + (15.0 / 50.0), 1.0f);
		glm::vec3 uva2 = glm::vec3(0.0f, 0.0f + (35.0 / 50.0), 1.0f);
		glm::vec3 uva3 = glm::vec3(0.5f, 1.0f, 1.0f);
		glm::vec3 uva4 = glm::vec3(1.0f, 0.0f + (35.0 / 50.0), 1.0f);
		glm::vec3 uva5 = glm::vec3(1.0f, 0.0f + (15.0 / 50.0), 1.0f);
		glm::vec3 uva6 = glm::vec3(0.5f, 0.0f, 1.0f);
		*/		

		glm::vec3 uva1 = glm::vec3(-1.0f, -0.4f, 0.0f);
		glm::vec3 uva2 = glm::vec3(-1.0f,  0.4f, 0.0f);
		glm::vec3 uva3 = glm::vec3( 0.0f,  1.0f, 0.0f);
		glm::vec3 uva4 = glm::vec3( 1.0f,  0.4f, 0.0f);
		glm::vec3 uva5 = glm::vec3( 1.0f, -0.4f, 0.0f);
		glm::vec3 uva6 = glm::vec3( 0.0f, -1.0f, 0.0f);

		glm::vec3 uvab1 = glm::vec3(uva1.x, uva1.y, 0.5f);
		glm::vec3 uvab2 = glm::vec3(uva2.x, uva2.y, 0.5f);
		glm::vec3 uvab3 = glm::vec3(uva3.x, uva3.y, 0.5f);
		glm::vec3 uvab4 = glm::vec3(uva4.x, uva4.y, 0.5f);
		glm::vec3 uvab5 = glm::vec3(uva5.x, uva5.y, 0.5f);
		glm::vec3 uvab6 = glm::vec3(uva6.x, uva6.y, 0.5f);

		// ambient occlusion map uvs
		glm::vec2 amb_uv1 = glm::vec2(amb_tile_x, amb_tile_y + (0.3f * amb_tile_h));
		glm::vec2 amb_uv2 = glm::vec2(amb_tile_x, amb_tile_y + (0.7f * amb_tile_h));
		glm::vec2 amb_uv3 = glm::vec2(amb_tile_x + (0.5f * amb_tile_w), amb_tile_y + amb_tile_h);
		glm::vec2 amb_uv4 = glm::vec2(amb_tile_x + amb_tile_w, amb_tile_y + (0.7f * amb_tile_h));
		glm::vec2 amb_uv5 = glm::vec2(amb_tile_x + amb_tile_w, amb_tile_y + (0.3f * amb_tile_h));
		glm::vec2 amb_uv6 = glm::vec2(amb_tile_x + (0.5f * amb_tile_w), amb_tile_y);

		glm::vec2 amb_uvcen = glm::mix(amb_uv1 + ((amb_uv5 - amb_uv1) * 0.5f), amb_uv2 + ((amb_uv4 - amb_uv2) * 0.5f), 0.5f);

		// TODOOOO
		glm::vec2 amb_suv1 = glm::vec2(0.0f, 0.0375f);
		glm::vec2 amb_suv2 = glm::vec2(0.125f, 0.0375f);
		glm::vec2 amb_suv3 = glm::vec2(0.125f, 0.0875f);
		glm::vec2 amb_suv4 = glm::vec2(0.0f, 0.0875f);



		/*
		         p6
		    p1         p5
		    p2         p4
		         p3
		*/

		float bot_z = 0.0f;
		float top_z = 0.0f;

		if (m_flags & FLAGS_FIXED_Z)
		{
			bot_z = m_zbase * 0.1f;
			top_z = (m_zbase + m_zbase_height) * 0.1f;
		}
		else
		{
			bot_z = m_zbase * 0.1f;
			top_z = (m_zbase * 0.1f) + z;
		}

		glm::vec3 p1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), top_z);
		glm::vec3 p2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), top_z);
		glm::vec3 p3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), top_z);
		glm::vec3 p4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), top_z);
		glm::vec3 p5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), top_z);
		glm::vec3 p6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, top_z);

		glm::vec3 pcen = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (20.0 / 70.0)), top_z + (tiledata->top_height * 0.1f));

		glm::vec3 bp1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), bot_z);
		glm::vec3 bp2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), bot_z);
		glm::vec3 bp3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), bot_z);
		glm::vec3 bp4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), bot_z);
		glm::vec3 bp5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), bot_z);
		glm::vec3 bp6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, bot_z);

		/*
		glm::vec3 tp1 = glm::vec3(tx1, ty1 + (m_tile_height * (15.0 / 70.0)), 100.0f);
		glm::vec3 tp2 = glm::vec3(tx1, ty1 + (m_tile_height * (35.0 / 70.0)), 100.0f);
		glm::vec3 tp3 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1 + (m_tile_height * (50.0 / 70.0)), 100.0f);
		glm::vec3 tp4 = glm::vec3(tx2, ty1 + (m_tile_height * (35.0 / 70.0)), 100.0f);
		glm::vec3 tp5 = glm::vec3(tx2, ty1 + (m_tile_height * (15.0 / 70.0)), 100.0f);
		glm::vec3 tp6 = glm::vec3(tx1 + (m_tile_width * 0.5), ty1, 100.0f);
		*/

		glm::vec3 top_norm(0.0f, 0.0f, 1.0f);

		VBO::Vertex tv1(p1, uv1, amb_uv1, top_norm, tiledata->color);
		VBO::Vertex tv2(p2, uv2, amb_uv2, top_norm, tiledata->color);
		VBO::Vertex tv3(p3, uv3, amb_uv3, top_norm, tiledata->color);
		VBO::Vertex tv4(p4, uv4, amb_uv4, top_norm, tiledata->color);
		VBO::Vertex tv5(p5, uv5, amb_uv5, top_norm, tiledata->color);
		VBO::Vertex tv6(p6, uv6, amb_uv6, top_norm, tiledata->color);
		VBO::Vertex tvcen(pcen, uvcen, amb_uvcen, top_norm, tiledata->color);

		VBO::Vertex left_v1(p1, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex left_v2(p2, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex left_v3(bp2, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex left_v4(bp1, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex topleft_v1(p6, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex topleft_v2(p1, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex topleft_v3(bp1, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex topleft_v4(bp6, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex topright_v1(p5, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex topright_v2(p6, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex topright_v3(bp6, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex topright_v4(bp5, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex right_v1(p4, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex right_v2(p5, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex right_v3(bp5, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex right_v4(bp4, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex botright_v1(p3, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex botright_v2(p4, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex botright_v3(bp4, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex botright_v4(bp3, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex botleft_v1(p2, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex botleft_v2(p3, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex botleft_v3(bp3, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex botleft_v4(bp2, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex sideleft_v1(p3, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex sideleft_v2(p6, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex sideleft_v3(bp6, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex sideleft_v4(bp3, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex sideright_v1(p6, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex sideright_v2(p3, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex sideright_v3(bp3, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex sideright_v4(bp6, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex midtop_v1(p1, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex midtop_v2(p5, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex midtop_v3(bp5, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex midtop_v4(bp1, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex midbot_v1(p4, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex midbot_v2(p2, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex midbot_v3(bp2, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex midbot_v4(bp4, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex centtop_v1(p5, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex centtop_v2(p1, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex centtop_v3(bp1, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex centtop_v4(bp5, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex centbot_v1(p2, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex centbot_v2(p4, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex centbot_v3(bp4, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex centbot_v4(bp2, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex corntl_v1(p2, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex corntl_v2(p6, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex corntl_v3(bp6, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex corntl_v4(bp2, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex corntr_v1(p6, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex corntr_v2(p4, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex corntr_v3(bp4, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex corntr_v4(bp6, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex cornbl_v1(p3, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex cornbl_v2(p1, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex cornbl_v3(bp1, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex cornbl_v4(bp3, suv4, amb_suv4, glm::vec3(), tiledata->color);

		VBO::Vertex cornbr_v1(p5, suv1, amb_suv1, glm::vec3(), tiledata->color);
		VBO::Vertex cornbr_v2(p3, suv2, amb_suv2, glm::vec3(), tiledata->color);
		VBO::Vertex cornbr_v3(bp3, suv3, amb_suv3, glm::vec3(), tiledata->color);
		VBO::Vertex cornbr_v4(bp5, suv4, amb_suv4, glm::vec3(), tiledata->color);

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

		const float lum_topleft = 1.0f;
		const float lum_left = 0.8f;
		const float lum_topright = 0.6f;
		const float lum_right = 0.4f;
		const float lum_botleft = 0.4f;
		const float lum_botright = 0.2f;
		const float lum_up = 0.9f;
		const float lum_down = 0.3f;






		int render_sides = 0;

		switch (tiledata->type)
		{
			case Tileset::TILE_FULL:
			{
				/*
					  /\
					 /  \
					|    |
					|    |
					 \  /
					  \/
				*/

				/*
				vbo3d->makeTri(vbo3d_index++, tv1, tv6, tv5);
				vbo3d->makeTri(vbo3d_index++, tv1, tv5, tv4);
				vbo3d->makeTri(vbo3d_index++, tv1, tv4, tv3);
				vbo3d->makeTri(vbo3d_index++, tv1, tv3, tv2);
				*/

				if (tiledata->top_type == Tileset::TOP_POINTY)
				{
					vbo->makeTriPolyNorm(vbo_index++, tv1, tv6, tvcen);
					vbo->makeTriPolyNorm(vbo_index++, tv6, tv5, tvcen);
					vbo->makeTriPolyNorm(vbo_index++, tv5, tv4, tvcen);
					vbo->makeTriPolyNorm(vbo_index++, tv4, tv3, tvcen);
					vbo->makeTriPolyNorm(vbo_index++, tv3, tv2, tvcen);
					vbo->makeTriPolyNorm(vbo_index++, tv2, tv1, tvcen);
				}
				else if (tiledata->top_type == Tileset::TOP_FLAT)
				{
					vbo->makeTriPolyNorm(vbo_index++, tv1, tv6, tv5);
					vbo->makeTriPolyNorm(vbo_index++, tv1, tv5, tv4);
					vbo->makeTriPolyNorm(vbo_index++, tv1, tv4, tv3);
					vbo->makeTriPolyNorm(vbo_index++, tv1, tv3, tv2);
				}

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_BOTLEFT;
				break;
			}
			case Tileset::TILE_LEFT:
			{
				/*
				      /|
				     / |
				    |  |
				    |  |
				     \ |
				      \|
				*/

				vbo->makeTri(vbo_index++, tv1, tv6, tv3);
				vbo->makeTri(vbo_index++, tv1, tv3, tv2);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_BOTLEFT;
				render_sides |= RENDER_SIDELEFT;
				break;
			}
			case Tileset::TILE_RIGHT:
			{
				/*
				    |\
				    | \
				    |  |
				    |  |
				    | /
				    |/
				*/

				vbo->makeTri(vbo_index++, tv6, tv5, tv4);
				vbo->makeTri(vbo_index++, tv6, tv4, tv3);

				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_SIDERIGHT;
				break;
			}
			case Tileset::TILE_TOP:
			{
				/*
				     /\
				    /__\
				*/

				vbo->makeTri(vbo_index++, tv1, tv6, tv5);

				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_MIDTOP;
				break;
			}
			case Tileset::TILE_BOTTOM:
			{
				/*  ____
				    \  /
				     \/
				*/

				vbo->makeTri(vbo_index++, tv2, tv4, tv3);

				render_sides |= RENDER_BOTLEFT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_MIDBOT;
				break;
			}
			case Tileset::TILE_MID:
			{
				/*  ______
				    |    |
				    |____|
				*/

				vbo->makeTri(vbo_index++, tv1, tv5, tv4);
				vbo->makeTri(vbo_index++, tv1, tv4, tv2);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_CENTER_TOP;
				render_sides |= RENDER_CENTER_BOT;
				break;
			}
			case Tileset::TILE_CORNER_TL:
			{
				/*
				      /.
				     / .
				    | . 
				    |.  				   
				*/

				vbo->makeTri(vbo_index++, tv1, tv6, tv2);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_TOPLEFT;
				render_sides |= RENDER_CORNER_TL;
				break;
			}
			case Tileset::TILE_CORNER_TR:
			{
				/*
				    .\
				    . \
				     . |
				      .|				      
				*/

				vbo->makeTri(vbo_index++, tv6, tv5, tv4);

				render_sides |= RENDER_TOPRIGHT;
				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_CORNER_TR;
				break;
			}
			case Tileset::TILE_CORNER_BL:
			{
				/*
				    |.  
				    | . 
				     \ .
				      \.
				*/

				vbo->makeTri(vbo_index++, tv1, tv3, tv2);

				render_sides |= RENDER_LEFT;
				render_sides |= RENDER_BOTLEFT;
				render_sides |= RENDER_CORNER_BL;
				break;
			}
			case Tileset::TILE_CORNER_BR:
			{
				/*
			  	      .|
				     . |
				    . /
				    ./
				*/

				vbo->makeTri(vbo_index++, tv3, tv5, tv4);

				render_sides |= RENDER_RIGHT;
				render_sides |= RENDER_BOTRIGHT;
				render_sides |= RENDER_CORNER_BR;
				break;
			}

			default:
			{
				// make degen geo
				vbo->degenTris(vbo_index, MAX_VERTS);
				break;
			}
		}

		if (render_sides & RENDER_LEFT)		
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, left_v1, left_v2, left_v3, left_v4);		
		if (render_sides & RENDER_TOPLEFT)		
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, topleft_v1, topleft_v2, topleft_v3, topleft_v4);
		if (render_sides & RENDER_TOPRIGHT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, topright_v1, topright_v2, topright_v3, topright_v4);
		if (render_sides & RENDER_RIGHT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, right_v1, right_v2, right_v3, right_v4);
		if (render_sides & RENDER_BOTRIGHT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, botright_v1, botright_v2, botright_v3, botright_v4);
		if (render_sides & RENDER_BOTLEFT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, botleft_v1, botleft_v2, botleft_v3, botleft_v4);
		if (render_sides & RENDER_SIDELEFT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, sideleft_v1, sideleft_v2, sideleft_v3, sideleft_v4);
		if (render_sides & RENDER_SIDERIGHT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, sideright_v1, sideright_v2, sideright_v3, sideright_v4);
		if (render_sides & RENDER_MIDTOP)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, midtop_v1, midtop_v2, midtop_v3, midtop_v4);
		if (render_sides & RENDER_MIDBOT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, midbot_v1, midbot_v2, midbot_v3, midbot_v4);
		if (render_sides & RENDER_CENTER_TOP)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, centtop_v1, centtop_v2, centtop_v3, centtop_v4);
		if (render_sides & RENDER_CENTER_BOT)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, centbot_v1, centbot_v2, centbot_v3, centbot_v4);
		if (render_sides & RENDER_CORNER_TL)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, corntl_v1, corntl_v2, corntl_v3, corntl_v4);
		if (render_sides & RENDER_CORNER_TR)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, corntr_v1, corntr_v2, corntr_v3, corntr_v4);
		if (render_sides & RENDER_CORNER_BL)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, cornbl_v1, cornbl_v2, cornbl_v3, cornbl_v4);
		if (render_sides & RENDER_CORNER_BR)
			vbo_index += vbo->makeQuadPolyNorm(vbo_index, cornbr_v1, cornbr_v2, cornbr_v3, cornbr_v4);

		if (vbo_index < MAX_VERTS)
			vbo->degenTris(vbo_index, MAX_VERTS - vbo_index);
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

	m_edit_callback->tilemapModified();
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

		m_edit_callback->tilemapModified();
	}
}

void Tilemap::allocBucket(int bin)
{
	assert(m_buckets[bin] == nullptr);

	int bx = bin % (AREA_WIDTH / BUCKET_WIDTH);
	int by = bin / (AREA_WIDTH / BUCKET_WIDTH);

	m_buckets[bin] = new Bucket();

	m_buckets[bin]->coverage = 0;
	m_buckets[bin]->map = new unsigned int[BUCKET_WIDTH * BUCKET_HEIGHT];
	m_buckets[bin]->tiles = new VBO(BUCKET_WIDTH * BUCKET_HEIGHT * MAX_VERTS);
	m_buckets[bin]->x = bx;
	m_buckets[bin]->y = by;
	
	for (int i = 0; i < BUCKET_WIDTH*BUCKET_HEIGHT; i++)
	{
		m_buckets[bin]->map[i] = TILE_EMPTY;

		m_buckets[bin]->tiles->degenTris(i * MAX_VERTS, MAX_VERTS);
	}
}

void Tilemap::deallocBucket(int bin)
{
	assert(m_buckets[bin] != nullptr);

	delete[] m_buckets[bin]->map;
	delete m_buckets[bin]->tiles;
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

void Tilemap::tileChanged(int index)
{
	tesselateAllByTile(index);
}

void Tilemap::getAdjacentTileCoords(AdjacentTiles* tiles, int tx, int ty)
{
	tiles->left.x = tx - 1;
	tiles->left.y = ty;
	tiles->right.x = tx + 1;
	tiles->right.y = ty;

	if (ty & 1)
	{

		tiles->topleft.x = tx;		// top left
		tiles->topleft.y = ty - 1;
		tiles->topright.x = tx + 1;	// top right
		tiles->topright.y = ty - 1;
		tiles->botleft.x = tx;		// bottom left
		tiles->botleft.y = ty + 1;
		tiles->botright.x = tx + 1;	// bottom right
		tiles->botright.y = ty + 1;
	}
	else
	{
		tiles->topleft.x = tx - 1;	// top left
		tiles->topleft.y = ty - 1;
		tiles->topright.x = tx;		// top right
		tiles->topright.y = ty - 1;
		tiles->botleft.x = tx - 1;	// bottom left
		tiles->botleft.y = ty + 1;
		tiles->botright.x = tx;		// bottom right
		tiles->botright.y = ty + 1;
	}
}