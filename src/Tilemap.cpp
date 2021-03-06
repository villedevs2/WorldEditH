#include "Tilemap.h"

Tilemap::Tilemap(Tileset* tileset, Tilemap::EditCallback* edit_callback, float zbase, float zbase_height, unsigned int flags, AmbientOcclusion* ao)
{
	m_ao = ao;

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


	AdjacentTileCoords adjacent_coords;
	getAdjacentTileCoords(&adjacent_coords, (bucket->x * BUCKET_WIDTH) + bx, (bucket->y * BUCKET_HEIGHT) + by);
	
	AdjacentTiles adjacent_tiles;
	getAdjacentTiles(&adjacent_tiles, &adjacent_coords);

	
	int z_current = (bucket->map[(by * BUCKET_WIDTH) + bx] & Z_MASK) >> Z_SHIFT;


	AOSolution ao_solution;
	makeAOSolution((bucket->x * BUCKET_WIDTH) + bx, (bucket->y * BUCKET_HEIGHT) + by, &ao_solution);

	/*
	int z_left = getZ(adjacent_coords.left.x, adjacent_coords.left.y);
	int z_right = getZ(adjacent_coords.right.x, adjacent_coords.right.y);
	int z_topleft = getZ(adjacent_coords.topleft.x, adjacent_coords.topleft.y);
	int z_topright = getZ(adjacent_coords.topright.x, adjacent_coords.topright.y);
	int z_botleft = getZ(adjacent_coords.botleft.x, adjacent_coords.botleft.y);
	int z_botright = getZ(adjacent_coords.botright.x, adjacent_coords.botright.y);

	int floor_ao_tile = 0;

	const int HEIGHT_OCCLUSION_THRESHOLD = 10;

	if ((z_left - z_current) >= HEIGHT_OCCLUSION_THRESHOLD)
		floor_ao_tile |= AmbientOcclusion::SIDE_LEFT;
	if ((z_right - z_current) >= HEIGHT_OCCLUSION_THRESHOLD)
		floor_ao_tile |= AmbientOcclusion::SIDE_RIGHT;
	if ((z_topleft - z_current) >= HEIGHT_OCCLUSION_THRESHOLD)
		floor_ao_tile |= AmbientOcclusion::SIDE_TOPLEFT;
	if ((z_topright - z_current) >= HEIGHT_OCCLUSION_THRESHOLD)
		floor_ao_tile |= AmbientOcclusion::SIDE_TOPRIGHT;
	if ((z_botleft - z_current) >= HEIGHT_OCCLUSION_THRESHOLD)
		floor_ao_tile |= AmbientOcclusion::SIDE_BOTLEFT;
	if ((z_botright - z_current) >= HEIGHT_OCCLUSION_THRESHOLD)
		floor_ao_tile |= AmbientOcclusion::SIDE_BOTRIGHT;
		*/

	/*
	int floor_ao_tile_x = floor_ao_tile % AmbientOcclusion::NUM_FLOOR_TILES_X;
	int floor_ao_tile_y = floor_ao_tile / AmbientOcclusion::NUM_FLOOR_TILES_Y;

	float floor_amb_tile_w = 1.0 / (float)(AmbientOcclusion::NUM_TOTAL_TILES_X);
	float floor_amb_tile_h = 1.0 / (float)(AmbientOcclusion::NUM_TOTAL_TILES_Y);
	float floor_amb_tile_x = floor_ao_tile_x * floor_amb_tile_w;
	float floor_amb_tile_y = floor_ao_tile_y * floor_amb_tile_h;
	*/

	VBO<HSVertex>* vbo = bucket->tiles;
	float z = z_current * 0.1f;
	int vbo_index = ((by * BUCKET_WIDTH) + bx) * MAX_VERTS;

	int ctile = bucket->map[(by * BUCKET_WIDTH) + bx] & TILE_MASK;

	if (ctile == Tilemap::TILE_EMPTY)
	{
		// make degen geo
		vbo->degenTris(vbo_index, MAX_VERTS);
	}
	else
	{
		const Tileset::Tile* tiledata = m_tileset->getTile(ctile);

		Tilemap::TileDef tiledef;
		tiledef.floor_uvs[0] = tiledata->top_points[0];
		tiledef.floor_uvs[1] = tiledata->top_points[1];
		tiledef.floor_uvs[2] = tiledata->top_points[2];
		tiledef.floor_uvs[3] = tiledata->top_points[3];
		tiledef.floor_uvs[4] = tiledata->top_points[4];
		tiledef.floor_uvs[5] = tiledata->top_points[5];
		tiledef.floor_uvcen = glm::mix(tiledata->top_points[0] + ((tiledata->top_points[4] - tiledata->top_points[0]) * 0.5f), tiledata->top_points[1] + ((tiledata->top_points[3] - tiledata->top_points[1]) * 0.5f), 0.5f);

		tiledef.wallmid_uvs[0] = tiledata->side_points[0];
		tiledef.wallmid_uvs[1] = tiledata->side_points[1];
		tiledef.wallmid_uvs[2] = tiledata->side_points[2];
		tiledef.wallmid_uvs[3] = tiledata->side_points[3];

		tiledef.walltop_uvs[0] = tiledata->sidetop_points[0];
		tiledef.walltop_uvs[1] = tiledata->sidetop_points[1];
		tiledef.walltop_uvs[2] = tiledata->sidetop_points[2];
		tiledef.walltop_uvs[3] = tiledata->sidetop_points[3];

		tiledef.wallbot_uvs[0] = tiledata->sidebot_points[0];
		tiledef.wallbot_uvs[1] = tiledata->sidebot_points[1];
		tiledef.wallbot_uvs[2] = tiledata->sidebot_points[2];
		tiledef.wallbot_uvs[3] = tiledata->sidebot_points[3];

		tiledef.tiletype = tiledata->type;
		tiledef.toptype = tiledata->top_type;

		tiledef.tile_z = z;
		tiledef.top_height = tiledata->top_height;
		tiledef.color = tiledata->color;
		tiledef.shading = Tileset::SHADING_STANDARD;	// TODDODODODOD
		tiledef.tile_width = m_tile_width;
		tiledef.tile_height = m_tile_height;

		tiledef.tile_ao.floor = m_ao->getFloorTile(ao_solution.floor);
		tiledef.tile_ao.wall_left = m_ao->getWallTile(ao_solution.wall_left);
		tiledef.tile_ao.wall_right = m_ao->getWallTile(ao_solution.wall_right);
		tiledef.tile_ao.wall_topleft = m_ao->getWallTile(ao_solution.wall_topleft);
		tiledef.tile_ao.wall_topright = m_ao->getWallTile(ao_solution.wall_topright);
		tiledef.tile_ao.wall_botleft = m_ao->getWallTile(ao_solution.wall_botleft);
		tiledef.tile_ao.wall_botright = m_ao->getWallTile(ao_solution.wall_botright);
		tiledef.tile_ao.wall_sideleft = m_ao->getWallTile(ao_solution.wall_sideleft);
		tiledef.tile_ao.wall_sideright = m_ao->getWallTile(ao_solution.wall_sideright);
		tiledef.tile_ao.wall_midtop = m_ao->getWallTile(ao_solution.wall_midtop);
		tiledef.tile_ao.wall_midbot = m_ao->getWallTile(ao_solution.wall_midbot);
		tiledef.tile_ao.wall_centtop = m_ao->getWallTile(ao_solution.wall_centtop);
		tiledef.tile_ao.wall_centbot = m_ao->getWallTile(ao_solution.wall_centbot);
		tiledef.tile_ao.wall_corntl = m_ao->getWallTile(ao_solution.wall_corntl);
		tiledef.tile_ao.wall_corntr = m_ao->getWallTile(ao_solution.wall_corntr);
		tiledef.tile_ao.wall_cornbl = m_ao->getWallTile(ao_solution.wall_cornbl);
		tiledef.tile_ao.wall_cornbr = m_ao->getWallTile(ao_solution.wall_cornbr);

		tiledef.left_height = getZ(adjacent_coords.left.x, adjacent_coords.left.y) * 0.1f;
		tiledef.right_height = getZ(adjacent_coords.right.x, adjacent_coords.right.y) * 0.1f;
		tiledef.topleft_height = getZ(adjacent_coords.topleft.x, adjacent_coords.topleft.y) * 0.1f;
		tiledef.topright_height = getZ(adjacent_coords.topright.x, adjacent_coords.topright.y) * 0.1f;
		tiledef.botleft_height = getZ(adjacent_coords.botleft.x, adjacent_coords.botleft.y) * 0.1f;
		tiledef.botright_height = getZ(adjacent_coords.botright.x, adjacent_coords.botright.y) * 0.1f;


		int vbsize = Tilemap::makeVBOTile(vbo, vbo_index, tiledef, (bucket->x * BUCKET_WIDTH) + bx, (bucket->y * BUCKET_HEIGHT) + by);
		vbo_index += vbsize;

		if (vbsize < MAX_VERTS)
		{
			vbo->degenTris(vbo_index, MAX_VERTS - vbsize);
			vbo_index += MAX_VERTS - vbsize;
		}
	}


#if 0
	int wall_ao_tile[16];



	VBO<HSVertex>* vbo = bucket->tiles;
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
		glm::vec3 uva2 = glm::vec3(-1.0f, 0.4f, 0.0f);
		glm::vec3 uva3 = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 uva4 = glm::vec3(1.0f, 0.4f, 0.0f);
		glm::vec3 uva5 = glm::vec3(1.0f, -0.4f, 0.0f);
		glm::vec3 uva6 = glm::vec3(0.0f, -1.0f, 0.0f);

		glm::vec3 uvab1 = glm::vec3(uva1.x, uva1.y, 0.5f);
		glm::vec3 uvab2 = glm::vec3(uva2.x, uva2.y, 0.5f);
		glm::vec3 uvab3 = glm::vec3(uva3.x, uva3.y, 0.5f);
		glm::vec3 uvab4 = glm::vec3(uva4.x, uva4.y, 0.5f);
		glm::vec3 uvab5 = glm::vec3(uva5.x, uva5.y, 0.5f);
		glm::vec3 uvab6 = glm::vec3(uva6.x, uva6.y, 0.5f);

		// ambient occlusion map uvs
		AmbientOcclusion::AOFloorTile* floor_tile = m_ao->getFloorTile(ao_solution.floor);
		
		AmbientOcclusion::AOWallTile* wall_tile_l = m_ao->getWallTile(ao_solution.wall_left);
		AmbientOcclusion::AOWallTile* wall_tile_tl = m_ao->getWallTile(ao_solution.wall_topleft);
		AmbientOcclusion::AOWallTile* wall_tile_tr = m_ao->getWallTile(ao_solution.wall_topright);
		AmbientOcclusion::AOWallTile* wall_tile_r = m_ao->getWallTile(ao_solution.wall_right);
		AmbientOcclusion::AOWallTile* wall_tile_br = m_ao->getWallTile(ao_solution.wall_botright);
		AmbientOcclusion::AOWallTile* wall_tile_bl = m_ao->getWallTile(ao_solution.wall_botleft);
		AmbientOcclusion::AOWallTile* wall_tile_sidel = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_sider = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_midtop = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_midbot = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_centtop = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_centbot = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_ctl = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_ctr = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_cbl = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);
		AmbientOcclusion::AOWallTile* wall_tile_cbr = m_ao->getWallTile(AmbientOcclusion::WALLSIDE_FLOOR);



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

		/* wall AO:
			3 0
			2 1
		*/

		glm::vec3 top_norm(0.0f, 0.0f, 1.0f);

		HSVertex tv1(p1, uv1, floor_tile->uv[0], top_norm, tiledata->color);
		HSVertex tv2(p2, uv2, floor_tile->uv[1], top_norm, tiledata->color);
		HSVertex tv3(p3, uv3, floor_tile->uv[2], top_norm, tiledata->color);
		HSVertex tv4(p4, uv4, floor_tile->uv[3], top_norm, tiledata->color);
		HSVertex tv5(p5, uv5, floor_tile->uv[4], top_norm, tiledata->color);
		HSVertex tv6(p6, uv6, floor_tile->uv[5], top_norm, tiledata->color);
		HSVertex tvcen(pcen, uvcen, floor_tile->center, top_norm, tiledata->color);

		HSVertex left_v1(p1, suv1, wall_tile_l->uv[3], glm::vec3(), tiledata->color);
		HSVertex left_v2(p2, suv2, wall_tile_l->uv[0], glm::vec3(), tiledata->color);
		HSVertex left_v3(bp2, suv3, wall_tile_l->uv[1], glm::vec3(), tiledata->color);
		HSVertex left_v4(bp1, suv4, wall_tile_l->uv[2], glm::vec3(), tiledata->color);

		HSVertex topleft_v1(p6, suv1, wall_tile_tl->uv[3], glm::vec3(), tiledata->color);
		HSVertex topleft_v2(p1, suv2, wall_tile_tl->uv[0], glm::vec3(), tiledata->color);
		HSVertex topleft_v3(bp1, suv3, wall_tile_tl->uv[1], glm::vec3(), tiledata->color);
		HSVertex topleft_v4(bp6, suv4, wall_tile_tl->uv[2], glm::vec3(), tiledata->color);

		HSVertex topright_v1(p5, suv1, wall_tile_tr->uv[3], glm::vec3(), tiledata->color);
		HSVertex topright_v2(p6, suv2, wall_tile_tr->uv[0], glm::vec3(), tiledata->color);
		HSVertex topright_v3(bp6, suv3, wall_tile_tr->uv[1], glm::vec3(), tiledata->color);
		HSVertex topright_v4(bp5, suv4, wall_tile_tr->uv[2], glm::vec3(), tiledata->color);

		HSVertex right_v1(p4, suv1, wall_tile_r->uv[3], glm::vec3(), tiledata->color);
		HSVertex right_v2(p5, suv2, wall_tile_r->uv[0], glm::vec3(), tiledata->color);
		HSVertex right_v3(bp5, suv3, wall_tile_r->uv[1], glm::vec3(), tiledata->color);
		HSVertex right_v4(bp4, suv4, wall_tile_r->uv[2], glm::vec3(), tiledata->color);

		HSVertex botright_v1(p3, suv1, wall_tile_br->uv[3], glm::vec3(), tiledata->color);
		HSVertex botright_v2(p4, suv2, wall_tile_br->uv[0], glm::vec3(), tiledata->color);
		HSVertex botright_v3(bp4, suv3, wall_tile_br->uv[1], glm::vec3(), tiledata->color);
		HSVertex botright_v4(bp3, suv4, wall_tile_br->uv[2], glm::vec3(), tiledata->color);

		HSVertex botleft_v1(p2, suv1, wall_tile_bl->uv[3], glm::vec3(), tiledata->color);
		HSVertex botleft_v2(p3, suv2, wall_tile_bl->uv[0], glm::vec3(), tiledata->color);
		HSVertex botleft_v3(bp3, suv3, wall_tile_bl->uv[1], glm::vec3(), tiledata->color);
		HSVertex botleft_v4(bp2, suv4, wall_tile_bl->uv[2], glm::vec3(), tiledata->color);

		HSVertex sideleft_v1(p3, suv1, wall_tile_sidel->uv[3], glm::vec3(), tiledata->color);
		HSVertex sideleft_v2(p6, suv2, wall_tile_sidel->uv[0], glm::vec3(), tiledata->color);
		HSVertex sideleft_v3(bp6, suv3, wall_tile_sidel->uv[1], glm::vec3(), tiledata->color);
		HSVertex sideleft_v4(bp3, suv4, wall_tile_sidel->uv[2], glm::vec3(), tiledata->color);

		HSVertex sideright_v1(p6, suv1, wall_tile_sider->uv[3], glm::vec3(), tiledata->color);
		HSVertex sideright_v2(p3, suv2, wall_tile_sider->uv[0], glm::vec3(), tiledata->color);
		HSVertex sideright_v3(bp3, suv3, wall_tile_sider->uv[1], glm::vec3(), tiledata->color);
		HSVertex sideright_v4(bp6, suv4, wall_tile_sider->uv[2], glm::vec3(), tiledata->color);

		HSVertex midtop_v1(p1, suv1, wall_tile_midtop->uv[3], glm::vec3(), tiledata->color);
		HSVertex midtop_v2(p5, suv2, wall_tile_midtop->uv[0], glm::vec3(), tiledata->color);
		HSVertex midtop_v3(bp5, suv3, wall_tile_midtop->uv[1], glm::vec3(), tiledata->color);
		HSVertex midtop_v4(bp1, suv4, wall_tile_midtop->uv[2], glm::vec3(), tiledata->color);

		HSVertex midbot_v1(p4, suv1, wall_tile_midbot->uv[3], glm::vec3(), tiledata->color);
		HSVertex midbot_v2(p2, suv2, wall_tile_midbot->uv[0], glm::vec3(), tiledata->color);
		HSVertex midbot_v3(bp2, suv3, wall_tile_midbot->uv[1], glm::vec3(), tiledata->color);
		HSVertex midbot_v4(bp4, suv4, wall_tile_midbot->uv[2], glm::vec3(), tiledata->color);

		HSVertex centtop_v1(p5, suv1, wall_tile_centtop->uv[3], glm::vec3(), tiledata->color);
		HSVertex centtop_v2(p1, suv2, wall_tile_centtop->uv[0], glm::vec3(), tiledata->color);
		HSVertex centtop_v3(bp1, suv3, wall_tile_centtop->uv[1], glm::vec3(), tiledata->color);
		HSVertex centtop_v4(bp5, suv4, wall_tile_centtop->uv[2], glm::vec3(), tiledata->color);

		HSVertex centbot_v1(p2, suv1, wall_tile_centbot->uv[3], glm::vec3(), tiledata->color);
		HSVertex centbot_v2(p4, suv2, wall_tile_centbot->uv[0], glm::vec3(), tiledata->color);
		HSVertex centbot_v3(bp4, suv3, wall_tile_centbot->uv[1], glm::vec3(), tiledata->color);
		HSVertex centbot_v4(bp2, suv4, wall_tile_centbot->uv[2], glm::vec3(), tiledata->color);

		HSVertex corntl_v1(p2, suv1, wall_tile_ctl->uv[3], glm::vec3(), tiledata->color);
		HSVertex corntl_v2(p6, suv2, wall_tile_ctl->uv[0], glm::vec3(), tiledata->color);
		HSVertex corntl_v3(bp6, suv3, wall_tile_ctl->uv[1], glm::vec3(), tiledata->color);
		HSVertex corntl_v4(bp2, suv4, wall_tile_ctl->uv[2], glm::vec3(), tiledata->color);

		HSVertex corntr_v1(p6, suv1, wall_tile_ctr->uv[3], glm::vec3(), tiledata->color);
		HSVertex corntr_v2(p4, suv2, wall_tile_ctr->uv[0], glm::vec3(), tiledata->color);
		HSVertex corntr_v3(bp4, suv3, wall_tile_ctr->uv[1], glm::vec3(), tiledata->color);
		HSVertex corntr_v4(bp6, suv4, wall_tile_ctr->uv[2], glm::vec3(), tiledata->color);

		HSVertex cornbl_v1(p3, suv1, wall_tile_cbl->uv[3], glm::vec3(), tiledata->color);
		HSVertex cornbl_v2(p1, suv2, wall_tile_cbl->uv[0], glm::vec3(), tiledata->color);
		HSVertex cornbl_v3(bp1, suv3, wall_tile_cbl->uv[1], glm::vec3(), tiledata->color);
		HSVertex cornbl_v4(bp3, suv4, wall_tile_cbl->uv[2], glm::vec3(), tiledata->color);

		HSVertex cornbr_v1(p5, suv1, wall_tile_cbr->uv[3], glm::vec3(), tiledata->color);
		HSVertex cornbr_v2(p3, suv2, wall_tile_cbr->uv[0], glm::vec3(), tiledata->color);
		HSVertex cornbr_v3(bp3, suv3, wall_tile_cbr->uv[1], glm::vec3(), tiledata->color);
		HSVertex cornbr_v4(bp5, suv4, wall_tile_cbr->uv[2], glm::vec3(), tiledata->color);

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
#endif
}

int Tilemap::get(int x, int y)
{
	if (x < 0 || x >= AREA_WIDTH)
		return TILE_EMPTY;
	if (y < 0 || y >= AREA_HEIGHT)
		return TILE_EMPTY;

	int bin = (y / BUCKET_HEIGHT) * (AREA_WIDTH / BUCKET_WIDTH) + (x / BUCKET_WIDTH);
	if (m_buckets[bin] != nullptr)
	{
		Bucket* bucket = m_buckets[bin];
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return bucket->map[(iy * BUCKET_WIDTH) + ix] & TILE_MASK;
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
		Bucket* bucket = m_buckets[bin];
		int ix = x % BUCKET_WIDTH;
		int iy = y % BUCKET_HEIGHT;
		return (bucket->map[(iy * BUCKET_WIDTH) + ix] & Z_MASK) >> Z_SHIFT;
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

	// retesselate adjacent tiles
	AdjacentTileCoords adjacent_coords;
	AdjacentTiles adjacent_tiles;
	getAdjacentTileCoords(&adjacent_coords, x, y);
	getAdjacentTiles(&adjacent_tiles, &adjacent_coords);
	if (adjacent_tiles.left != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.left.x, adjacent_coords.left.y);
	if (adjacent_tiles.right != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.right.x, adjacent_coords.right.y);
	if (adjacent_tiles.topleft != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.topleft.x, adjacent_coords.topleft.y);
	if (adjacent_tiles.topright != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.topright.x, adjacent_coords.topright.y);
	if (adjacent_tiles.botleft != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.botleft.x, adjacent_coords.botright.y);
	if (adjacent_tiles.botright != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.botright.x, adjacent_coords.botright.y);

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

		// only edit Z if the tile is not empty
		int tile = m_buckets[bin]->map[index] & TILE_MASK;
		if (tile != TILE_EMPTY)
		{
			m_buckets[bin]->map[index] &= ~Z_MASK;
			m_buckets[bin]->map[index] |= (z << Z_SHIFT) & Z_MASK;

			tesselateTile(m_buckets[bin], ix, iy);

			// retesselate adjacent tiles
			AdjacentTileCoords adjacent_coords;
			AdjacentTiles adjacent_tiles;
			getAdjacentTileCoords(&adjacent_coords, x, y);
			getAdjacentTiles(&adjacent_tiles, &adjacent_coords);
			if (adjacent_tiles.left != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.left.x, adjacent_coords.left.y);
			if (adjacent_tiles.right != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.right.x, adjacent_coords.right.y);
			if (adjacent_tiles.topleft != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.topleft.x, adjacent_coords.topleft.y);
			if (adjacent_tiles.topright != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.topright.x, adjacent_coords.topright.y);
			if (adjacent_tiles.botleft != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.botleft.x, adjacent_coords.botright.y);
			if (adjacent_tiles.botright != TILE_EMPTY) retesselateTileByCoords(adjacent_coords.botright.x, adjacent_coords.botright.y);

			m_edit_callback->tilemapModified();
		}
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
	m_buckets[bin]->tiles = new VBO<HSVertex>(BUCKET_WIDTH * BUCKET_HEIGHT * MAX_VERTS);
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

void Tilemap::retesselateTileByCoords(int tx, int ty)
{
	if (tx >= 0 && tx < (AREA_WIDTH / BUCKET_WIDTH) &&
		ty >= 0 && ty < (AREA_HEIGHT / BUCKET_HEIGHT))
	{
		Tilemap::Bucket* bucket = getTileBucket(tx / BUCKET_WIDTH, ty / BUCKET_HEIGHT);
		if (bucket != nullptr)
		{
			int bx = tx % BUCKET_WIDTH;
			int by = ty % BUCKET_HEIGHT;

			int ctile = bucket->map[by * BUCKET_WIDTH + bx] & TILE_MASK;
			if (ctile != TILE_EMPTY)
			{
				tesselateTile(bucket, bx, by);
			}
		}
	}
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

void Tilemap::getAdjacentTileCoords(AdjacentTileCoords* tiles, int tx, int ty)
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

void Tilemap::getAdjacentTiles(AdjacentTiles* tiles, AdjacentTileCoords* coords)
{
	tiles->left = get(coords->left.x, coords->left.y);
	tiles->right = get(coords->right.x, coords->right.y);
	tiles->topleft = get(coords->topleft.x, coords->topleft.y);
	tiles->topright = get(coords->topright.x, coords->topright.y);
	tiles->botleft = get(coords->botleft.x, coords->botleft.y);
	tiles->botright = get(coords->botright.x, coords->botright.y);
}


void Tilemap::makeAOSolution(int tx, int ty, AOSolution* ao)
{
	const int HEIGHT_OCCLUSION_THRESHOLD = 10;

	AdjacentTileCoords coords;
	getAdjacentTileCoords(&coords, tx, ty);

	AdjacentTiles tiles;
	getAdjacentTiles(&tiles, &coords);

	int current_z = getZ(tx, ty);

	int floor_bits = 0;

	int wall_left_bits = 0;
	int wall_right_bits = 0;
	int wall_topleft_bits = 0;
	int wall_topright_bits = 0;
	int wall_botleft_bits = 0;
	int wall_botright_bits = 0;

	// left
	{
		int z = getZ(coords.left.x, coords.left.y);
		if ((z - current_z) >= HEIGHT_OCCLUSION_THRESHOLD)
			floor_bits |= AmbientOcclusion::SIDE_LEFT;

		if (tiles.topleft != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.topleft);
			if (tile->side_bits & Tileset::SIDE_BOT_LEFT)
				wall_left_bits |= AmbientOcclusion::WALLSIDE_RIGHT;
		}
		if (tiles.botleft != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.botleft);
			if (tile->side_bits & Tileset::SIDE_TOP_LEFT)
				wall_left_bits |= AmbientOcclusion::WALLSIDE_LEFT;
		}
	//	if (tiles.left != TILE_EMPTY)
		{
			wall_left_bits |= AmbientOcclusion::WALLSIDE_FLOOR;
		}
	}
	// right
	{
		int z = getZ(coords.right.x, coords.right.y);
		if ((z - current_z) >= HEIGHT_OCCLUSION_THRESHOLD)
			floor_bits |= AmbientOcclusion::SIDE_RIGHT;

		if (tiles.topright != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.topright);
			if (tile->side_bits & Tileset::SIDE_BOT_RIGHT)
				wall_right_bits |= AmbientOcclusion::WALLSIDE_LEFT;
		}
		if (tiles.botright != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.botright);
			if (tile->side_bits & Tileset::SIDE_TOP_RIGHT)
				wall_right_bits |= AmbientOcclusion::WALLSIDE_RIGHT;
		}
	//	if (tiles.right != TILE_EMPTY)
		{
			wall_right_bits |= AmbientOcclusion::WALLSIDE_FLOOR;
		}
	}
	// top left
	{
		int z = getZ(coords.topleft.x, coords.topleft.y);
		if ((z - current_z) >= HEIGHT_OCCLUSION_THRESHOLD)
			floor_bits |= AmbientOcclusion::SIDE_TOPLEFT;

		if (tiles.left != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.left);
			if (tile->side_bits & Tileset::SIDE_TOP_RIGHT)
				wall_topleft_bits |= AmbientOcclusion::WALLSIDE_LEFT;
		}
		if (tiles.topright != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.topright);
			if (tile->side_bits & Tileset::SIDE_LEFT)
				wall_topleft_bits |= AmbientOcclusion::WALLSIDE_RIGHT;
		}
	//	if (tiles.topleft != TILE_EMPTY)
		{
			wall_topleft_bits |= AmbientOcclusion::WALLSIDE_FLOOR;
		}
	}
	// top right
	{
		int z = getZ(coords.topright.x, coords.topright.y);
		if ((z - current_z) >= HEIGHT_OCCLUSION_THRESHOLD)
			floor_bits |= AmbientOcclusion::SIDE_TOPRIGHT;

		if (tiles.topleft != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.topleft);
			if (tile->side_bits & Tileset::SIDE_RIGHT)
				wall_topright_bits |= AmbientOcclusion::WALLSIDE_LEFT;
		}
		if (tiles.right != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.right);
			if (tile->side_bits & Tileset::SIDE_TOP_LEFT)
				wall_topright_bits |= AmbientOcclusion::WALLSIDE_RIGHT;
		}
	//	if (tiles.topright != TILE_EMPTY)
		{
			wall_topright_bits |= AmbientOcclusion::WALLSIDE_FLOOR;
		}
	}
	// bottom left
	{
		int z = getZ(coords.botleft.x, coords.botleft.y);
		if ((z - current_z) >= HEIGHT_OCCLUSION_THRESHOLD)
			floor_bits |= AmbientOcclusion::SIDE_BOTLEFT;

		if (tiles.botright != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.botright);
			if (tile->side_bits & Tileset::SIDE_LEFT)
				wall_botleft_bits |= AmbientOcclusion::WALLSIDE_LEFT;
		}
		if (tiles.left != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.left);
			if (tile->side_bits & Tileset::SIDE_BOT_RIGHT)
				wall_botleft_bits |= AmbientOcclusion::WALLSIDE_RIGHT;
		}
	//	if (tiles.botleft != TILE_EMPTY)
		{
			wall_botleft_bits |= AmbientOcclusion::WALLSIDE_FLOOR;
		}
	}
	// bottom right
	{
		int z = getZ(coords.botright.x, coords.botright.y);
		if ((z - current_z) >= HEIGHT_OCCLUSION_THRESHOLD)
			floor_bits |= AmbientOcclusion::SIDE_BOTRIGHT;

		if (tiles.right != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.right);
			if (tile->side_bits & Tileset::SIDE_BOT_LEFT)
				wall_botright_bits |= AmbientOcclusion::WALLSIDE_LEFT;
		}
		if (tiles.botleft != TILE_EMPTY)
		{
			Tileset::Tile* tile = m_tileset->getTile(tiles.botleft);
			if (tile->side_bits & Tileset::SIDE_RIGHT)
				wall_botright_bits |= AmbientOcclusion::WALLSIDE_RIGHT;
		}
	//	if (tiles.botright != TILE_EMPTY)
		{
			wall_botright_bits |= AmbientOcclusion::WALLSIDE_FLOOR;
		}
	}

	ao->floor = floor_bits;
	ao->wall_left = wall_left_bits;
	ao->wall_right = wall_right_bits;
	ao->wall_topleft = wall_topleft_bits;
	ao->wall_topright = wall_topright_bits;
	ao->wall_botleft = wall_botleft_bits;
	ao->wall_botright = wall_botright_bits;

	// TODO?
	ao->wall_sideleft = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_sideright = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_centbot = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_centtop = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_midtop = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_midbot = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_corntl = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_corntr = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_cornbl = AmbientOcclusion::WALLSIDE_FLOOR;
	ao->wall_cornbr = AmbientOcclusion::WALLSIDE_FLOOR;
}

int Tilemap::makeVBOTile(VBO<HSVertex>* vbo, int vbo_index, const Tilemap::TileDef& tiledef, int x, int y)
{
	enum RenderTileSide {
		TILESIDE_LEFT = 0,
		TILESIDE_RIGHT = 1,
		TILESIDE_TOPLEFT = 2,
		TILESIDE_TOPRIGHT = 3,
		TILESIDE_BOTLEFT = 4,
		TILESIDE_BOTRIGHT = 5
	};

	bool render_sidetop[6] = { true, true, true, true, true, true };
	bool render_sidebot[6] = { true, true, true, true, true, true };
	bool render_side[6] = { true, true, true, true, true, true };

	// side bottom render condition
	if (tiledef.left_height > 0.0f) render_sidebot[TILESIDE_LEFT] = false;
	if (tiledef.right_height > 0.0f) render_sidebot[TILESIDE_RIGHT] = false;
	if (tiledef.topleft_height > 0.0f) render_sidebot[TILESIDE_TOPLEFT] = false;
	if (tiledef.topright_height > 0.0f) render_sidebot[TILESIDE_TOPRIGHT] = false;
	if (tiledef.botleft_height > 0.0f) render_sidebot[TILESIDE_BOTLEFT] = false;
	if (tiledef.botright_height > 0.0f) render_sidebot[TILESIDE_BOTRIGHT] = false;

	// side top render condition
	if (tiledef.left_height >= tiledef.tile_z)
	{
		render_sidetop[TILESIDE_LEFT] = false;
		render_side[TILESIDE_LEFT] = false;
	}
	if (tiledef.right_height >= tiledef.tile_z)
	{
		render_sidetop[TILESIDE_RIGHT] = false;
		render_side[TILESIDE_RIGHT] = false;
	}
	if (tiledef.topleft_height >= tiledef.tile_z)
	{
		render_sidetop[TILESIDE_TOPLEFT] = false;
		render_side[TILESIDE_TOPLEFT] = false;
	}
	if (tiledef.topright_height >= tiledef.tile_z)
	{
		render_sidetop[TILESIDE_TOPRIGHT] = false;
		render_side[TILESIDE_TOPRIGHT] = false;
	}
	if (tiledef.botleft_height >= tiledef.tile_z)
	{
		render_sidetop[TILESIDE_BOTLEFT] = false;
		render_side[TILESIDE_BOTLEFT] = false;
	}
	if (tiledef.botright_height >= tiledef.tile_z)
	{
		render_sidetop[TILESIDE_BOTRIGHT] = false;
		render_side[TILESIDE_BOTRIGHT] = false;
	}


	int vbo_start = vbo_index;

	unsigned int color = tiledef.color;

	float botz = 0.1f;
	float topz = 0.1f;
	float z = tiledef.tile_z;

	float midz = 0.0f;
	if (tiledef.toptype == Tileset::TOP_POINTY)
		midz = z + (tiledef.top_height * 0.1f);
	else if (tiledef.toptype == Tileset::TOP_FLAT)
		midz = z;

	float tx1 = (float)(x) * tiledef.tile_width;
	float tx2 = tx1 + tiledef.tile_width;
	float ty1 = (float)(y) * (tiledef.tile_height / 2);
	float ty2 = ty1 + (tiledef.tile_height / 2);

	if (y & 1)
	{
		tx1 += tiledef.tile_width / 2;
		tx2 += tiledef.tile_width / 2;
	}

	glm::vec3 topt_p1 = glm::vec3(tx1, ty1 + (tiledef.tile_height * (15.0 / 70.0)), z);
	glm::vec3 topt_p2 = glm::vec3(tx1, ty1 + (tiledef.tile_height * (35.0 / 70.0)), z);
	glm::vec3 topt_p3 = glm::vec3(tx1 + (tiledef.tile_width * 0.5), ty1 + (tiledef.tile_height * (50.0 / 70.0)), z);
	glm::vec3 topt_p4 = glm::vec3(tx2, ty1 + (tiledef.tile_height * (35.0 / 70.0)), z);
	glm::vec3 topt_p5 = glm::vec3(tx2, ty1 + (tiledef.tile_height * (15.0 / 70.0)), z);
	glm::vec3 topt_p6 = glm::vec3(tx1 + (tiledef.tile_width * 0.5), ty1, z);

	glm::vec3 topt_pcen = glm::vec3(tx1 + (tiledef.tile_width * 0.5), ty1 + (tiledef.tile_height * (25.0 / 70.0)), midz);

	/*
	glm::vec3 topt_p1 = glm::vec3(0.0f, 0.3f, z);
	glm::vec3 topt_p2 = glm::vec3(0.0f, 0.7f, z);
	glm::vec3 topt_p3 = glm::vec3(0.5f, 1.0f, z);
	glm::vec3 topt_p4 = glm::vec3(1.0f, 0.7f, z);
	glm::vec3 topt_p5 = glm::vec3(1.0f, 0.3f, z);
	glm::vec3 topt_p6 = glm::vec3(0.5f, 0.0f, z);
	glm::vec3 topt_pcen = glm::vec3(0.5f, 0.5f, midz);
	*/

	glm::vec3 topb_p1 = glm::vec3(topt_p1.x, topt_p1.y, z - topz);
	glm::vec3 topb_p2 = glm::vec3(topt_p2.x, topt_p2.y, z - topz);
	glm::vec3 topb_p3 = glm::vec3(topt_p3.x, topt_p3.y, z - topz);
	glm::vec3 topb_p4 = glm::vec3(topt_p4.x, topt_p4.y, z - topz);
	glm::vec3 topb_p5 = glm::vec3(topt_p5.x, topt_p5.y, z - topz);
	glm::vec3 topb_p6 = glm::vec3(topt_p6.x, topt_p6.y, z - topz);

	glm::vec3 botb_p1 = glm::vec3(topt_p1.x, topt_p1.y, 0.0f);
	glm::vec3 botb_p2 = glm::vec3(topt_p2.x, topt_p2.y, 0.0f);
	glm::vec3 botb_p3 = glm::vec3(topt_p3.x, topt_p3.y, 0.0f);
	glm::vec3 botb_p4 = glm::vec3(topt_p4.x, topt_p4.y, 0.0f);
	glm::vec3 botb_p5 = glm::vec3(topt_p5.x, topt_p5.y, 0.0f);
	glm::vec3 botb_p6 = glm::vec3(topt_p6.x, topt_p6.y, 0.0f);

	glm::vec3 bott_p1 = glm::vec3(topt_p1.x, topt_p1.y, 0.0f + botz);
	glm::vec3 bott_p2 = glm::vec3(topt_p2.x, topt_p2.y, 0.0f + botz);
	glm::vec3 bott_p3 = glm::vec3(topt_p3.x, topt_p3.y, 0.0f + botz);
	glm::vec3 bott_p4 = glm::vec3(topt_p4.x, topt_p4.y, 0.0f + botz);
	glm::vec3 bott_p5 = glm::vec3(topt_p5.x, topt_p5.y, 0.0f + botz);
	glm::vec3 bott_p6 = glm::vec3(topt_p6.x, topt_p6.y, 0.0f + botz);

	const AmbientOcclusion::AOFloorTile* floor_ao = tiledef.tile_ao.floor;
	const AmbientOcclusion::AOWallTile* wall_left_ao = tiledef.tile_ao.wall_left;
	const AmbientOcclusion::AOWallTile* wall_right_ao = tiledef.tile_ao.wall_right;
	const AmbientOcclusion::AOWallTile* wall_topleft_ao = tiledef.tile_ao.wall_topleft;
	const AmbientOcclusion::AOWallTile* wall_topright_ao = tiledef.tile_ao.wall_topright;
	const AmbientOcclusion::AOWallTile* wall_botleft_ao = tiledef.tile_ao.wall_botleft;
	const AmbientOcclusion::AOWallTile* wall_botright_ao = tiledef.tile_ao.wall_botright;
	const AmbientOcclusion::AOWallTile* wall_sideleft_ao = tiledef.tile_ao.wall_sideleft;
	const AmbientOcclusion::AOWallTile* wall_sideright_ao = tiledef.tile_ao.wall_sideright;
	const AmbientOcclusion::AOWallTile* wall_midtop_ao = tiledef.tile_ao.wall_midtop;
	const AmbientOcclusion::AOWallTile* wall_midbot_ao = tiledef.tile_ao.wall_midbot;
	const AmbientOcclusion::AOWallTile* wall_centtop_ao = tiledef.tile_ao.wall_centtop;
	const AmbientOcclusion::AOWallTile* wall_centbot_ao = tiledef.tile_ao.wall_centbot;
	const AmbientOcclusion::AOWallTile* wall_corntl_ao = tiledef.tile_ao.wall_corntl;
	const AmbientOcclusion::AOWallTile* wall_corntr_ao = tiledef.tile_ao.wall_corntr;
	const AmbientOcclusion::AOWallTile* wall_cornbl_ao = tiledef.tile_ao.wall_cornbl;
	const AmbientOcclusion::AOWallTile* wall_cornbr_ao = tiledef.tile_ao.wall_cornbr;


	glm::vec3 top_norm(0.0f, 0.0f, 1.0f);

	HSVertex tv1(topt_p1, tiledef.floor_uvs[0], floor_ao->uv[0], top_norm, color);
	HSVertex tv2(topt_p2, tiledef.floor_uvs[1], floor_ao->uv[1], top_norm, color);
	HSVertex tv3(topt_p3, tiledef.floor_uvs[2], floor_ao->uv[2], top_norm, color);
	HSVertex tv4(topt_p4, tiledef.floor_uvs[3], floor_ao->uv[3], top_norm, color);
	HSVertex tv5(topt_p5, tiledef.floor_uvs[4], floor_ao->uv[4], top_norm, color);
	HSVertex tv6(topt_p6, tiledef.floor_uvs[5], floor_ao->uv[5], top_norm, color);
	HSVertex tvcen(topt_pcen, tiledef.floor_uvcen, floor_ao->center, top_norm, color);


	// TODO: depend on tile height?
	float amb_tt = 1.0f;
	float amb_tb = 0.9166666f;
	float amb_bt = 0.0833333f;
	float amb_bb = 0.0f;

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

	switch (tiledef.tiletype)
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

			vbo->makeTriPolyNorm(vbo_index++, tv1, tv6, tvcen);
			vbo->makeTriPolyNorm(vbo_index++, tv6, tv5, tvcen);
			vbo->makeTriPolyNorm(vbo_index++, tv5, tv4, tvcen);
			vbo->makeTriPolyNorm(vbo_index++, tv4, tv3, tvcen);
			vbo->makeTriPolyNorm(vbo_index++, tv3, tv2, tvcen);
			vbo->makeTriPolyNorm(vbo_index++, tv2, tv1, tvcen);

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
	}

	if (render_sides & RENDER_LEFT)
	{
		HSVertex m_v1(topb_p1, tiledef.wallmid_uvs[3], glm::mix(wall_left_ao->uv[3], wall_left_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p2, tiledef.wallmid_uvs[2], glm::mix(wall_left_ao->uv[0], wall_left_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p2, tiledef.wallmid_uvs[1], glm::mix(wall_left_ao->uv[0], wall_left_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p1, tiledef.wallmid_uvs[0], glm::mix(wall_left_ao->uv[3], wall_left_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p1, tiledef.walltop_uvs[3], glm::mix(wall_left_ao->uv[3], wall_left_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p2, tiledef.walltop_uvs[2], glm::mix(wall_left_ao->uv[0], wall_left_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p2, tiledef.walltop_uvs[1], glm::mix(wall_left_ao->uv[0], wall_left_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p1, tiledef.walltop_uvs[0], glm::mix(wall_left_ao->uv[3], wall_left_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p1, tiledef.wallbot_uvs[3], glm::mix(wall_left_ao->uv[3], wall_left_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p2, tiledef.wallbot_uvs[2], glm::mix(wall_left_ao->uv[0], wall_left_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p2, tiledef.wallbot_uvs[1], glm::mix(wall_left_ao->uv[0], wall_left_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p1, tiledef.wallbot_uvs[0], glm::mix(wall_left_ao->uv[3], wall_left_ao->uv[2], amb_bb), color);

		if (render_side[TILESIDE_LEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		if (render_sidetop[TILESIDE_LEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		if (render_sidebot[TILESIDE_LEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_TOPLEFT)
	{
		HSVertex m_v1(topb_p6, tiledef.wallmid_uvs[3], glm::mix(wall_topleft_ao->uv[3], wall_topleft_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p1, tiledef.wallmid_uvs[2], glm::mix(wall_topleft_ao->uv[0], wall_topleft_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p1, tiledef.wallmid_uvs[1], glm::mix(wall_topleft_ao->uv[0], wall_topleft_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p6, tiledef.wallmid_uvs[0], glm::mix(wall_topleft_ao->uv[3], wall_topleft_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p6, tiledef.walltop_uvs[3], glm::mix(wall_topleft_ao->uv[3], wall_topleft_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p1, tiledef.walltop_uvs[2], glm::mix(wall_topleft_ao->uv[0], wall_topleft_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p1, tiledef.walltop_uvs[1], glm::mix(wall_topleft_ao->uv[0], wall_topleft_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p6, tiledef.walltop_uvs[0], glm::mix(wall_topleft_ao->uv[3], wall_topleft_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p6, tiledef.wallbot_uvs[3], glm::mix(wall_topleft_ao->uv[3], wall_topleft_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p1, tiledef.wallbot_uvs[2], glm::mix(wall_topleft_ao->uv[0], wall_topleft_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p1, tiledef.wallbot_uvs[1], glm::mix(wall_topleft_ao->uv[0], wall_topleft_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p6, tiledef.wallbot_uvs[0], glm::mix(wall_topleft_ao->uv[3], wall_topleft_ao->uv[2], amb_bb), color);

		if (render_side[TILESIDE_TOPLEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		if (render_sidetop[TILESIDE_TOPLEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		if (render_sidebot[TILESIDE_TOPLEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_TOPRIGHT)
	{
		HSVertex m_v1(topb_p5, tiledef.wallmid_uvs[3], glm::mix(wall_topright_ao->uv[3], wall_topright_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p6, tiledef.wallmid_uvs[2], glm::mix(wall_topright_ao->uv[0], wall_topright_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p6, tiledef.wallmid_uvs[1], glm::mix(wall_topright_ao->uv[0], wall_topright_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p5, tiledef.wallmid_uvs[0], glm::mix(wall_topright_ao->uv[3], wall_topright_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p5, tiledef.walltop_uvs[3], glm::mix(wall_topright_ao->uv[3], wall_topright_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p6, tiledef.walltop_uvs[2], glm::mix(wall_topright_ao->uv[0], wall_topright_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p6, tiledef.walltop_uvs[1], glm::mix(wall_topright_ao->uv[0], wall_topright_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p5, tiledef.walltop_uvs[0], glm::mix(wall_topright_ao->uv[3], wall_topright_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p5, tiledef.wallbot_uvs[3], glm::mix(wall_topright_ao->uv[3], wall_topright_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p6, tiledef.wallbot_uvs[2], glm::mix(wall_topright_ao->uv[0], wall_topright_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p6, tiledef.wallbot_uvs[1], glm::mix(wall_topright_ao->uv[0], wall_topright_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p5, tiledef.wallbot_uvs[0], glm::mix(wall_topright_ao->uv[3], wall_topright_ao->uv[2], amb_bb), color);

		if (render_side[TILESIDE_TOPRIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		if (render_sidetop[TILESIDE_TOPRIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		if (render_sidebot[TILESIDE_TOPRIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_RIGHT)
	{
		HSVertex m_v1(topb_p4, tiledef.wallmid_uvs[3], glm::mix(wall_right_ao->uv[3], wall_right_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p5, tiledef.wallmid_uvs[2], glm::mix(wall_right_ao->uv[0], wall_right_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p5, tiledef.wallmid_uvs[1], glm::mix(wall_right_ao->uv[0], wall_right_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p4, tiledef.wallmid_uvs[0], glm::mix(wall_right_ao->uv[3], wall_right_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p4, tiledef.walltop_uvs[3], glm::mix(wall_right_ao->uv[3], wall_right_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p5, tiledef.walltop_uvs[2], glm::mix(wall_right_ao->uv[0], wall_right_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p5, tiledef.walltop_uvs[1], glm::mix(wall_right_ao->uv[0], wall_right_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p4, tiledef.walltop_uvs[0], glm::mix(wall_right_ao->uv[3], wall_right_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p4, tiledef.wallbot_uvs[3], glm::mix(wall_right_ao->uv[3], wall_right_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p5, tiledef.wallbot_uvs[2], glm::mix(wall_right_ao->uv[0], wall_right_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p5, tiledef.wallbot_uvs[1], glm::mix(wall_right_ao->uv[0], wall_right_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p4, tiledef.wallbot_uvs[0], glm::mix(wall_right_ao->uv[3], wall_right_ao->uv[2], amb_bb), color);

		if (render_side[TILESIDE_RIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		if (render_sidetop[TILESIDE_RIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		if (render_sidebot[TILESIDE_RIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_BOTRIGHT)
	{
		HSVertex m_v1(topb_p3, tiledef.wallmid_uvs[3], glm::mix(wall_botright_ao->uv[3], wall_botright_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p4, tiledef.wallmid_uvs[2], glm::mix(wall_botright_ao->uv[0], wall_botright_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p4, tiledef.wallmid_uvs[1], glm::mix(wall_botright_ao->uv[0], wall_botright_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p3, tiledef.wallmid_uvs[0], glm::mix(wall_botright_ao->uv[3], wall_botright_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p3, tiledef.walltop_uvs[3], glm::mix(wall_botright_ao->uv[3], wall_botright_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p4, tiledef.walltop_uvs[2], glm::mix(wall_botright_ao->uv[0], wall_botright_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p4, tiledef.walltop_uvs[1], glm::mix(wall_botright_ao->uv[0], wall_botright_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p3, tiledef.walltop_uvs[0], glm::mix(wall_botright_ao->uv[3], wall_botright_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p3, tiledef.wallbot_uvs[3], glm::mix(wall_botright_ao->uv[3], wall_botright_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p4, tiledef.wallbot_uvs[2], glm::mix(wall_botright_ao->uv[0], wall_botright_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p4, tiledef.wallbot_uvs[1], glm::mix(wall_botright_ao->uv[0], wall_botright_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p3, tiledef.wallbot_uvs[0], glm::mix(wall_botright_ao->uv[3], wall_botright_ao->uv[2], amb_bb), color);

		if (render_side[TILESIDE_BOTRIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		if (render_sidetop[TILESIDE_BOTRIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		if (render_sidebot[TILESIDE_BOTRIGHT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_BOTLEFT)
	{
		HSVertex m_v1(topb_p2, tiledef.wallmid_uvs[3], glm::mix(wall_botleft_ao->uv[3], wall_botleft_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p3, tiledef.wallmid_uvs[2], glm::mix(wall_botleft_ao->uv[0], wall_botleft_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p3, tiledef.wallmid_uvs[1], glm::mix(wall_botleft_ao->uv[0], wall_botleft_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p2, tiledef.wallmid_uvs[0], glm::mix(wall_botleft_ao->uv[3], wall_botleft_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p2, tiledef.walltop_uvs[3], glm::mix(wall_botleft_ao->uv[3], wall_botleft_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p3, tiledef.walltop_uvs[2], glm::mix(wall_botleft_ao->uv[0], wall_botleft_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p3, tiledef.walltop_uvs[1], glm::mix(wall_botleft_ao->uv[0], wall_botleft_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p2, tiledef.walltop_uvs[0], glm::mix(wall_botleft_ao->uv[3], wall_botleft_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p2, tiledef.wallbot_uvs[3], glm::mix(wall_botleft_ao->uv[3], wall_botleft_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p3, tiledef.wallbot_uvs[2], glm::mix(wall_botleft_ao->uv[0], wall_botleft_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p3, tiledef.wallbot_uvs[1], glm::mix(wall_botleft_ao->uv[0], wall_botleft_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p2, tiledef.wallbot_uvs[0], glm::mix(wall_botleft_ao->uv[3], wall_botleft_ao->uv[2], amb_bb), color);

		if (render_side[TILESIDE_BOTLEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		if (render_sidetop[TILESIDE_BOTLEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		if (render_sidebot[TILESIDE_BOTLEFT]) vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_SIDELEFT)
	{
		HSVertex m_v1(topb_p3, tiledef.wallmid_uvs[3], glm::mix(wall_sideleft_ao->uv[3], wall_sideleft_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p6, tiledef.wallmid_uvs[2], glm::mix(wall_sideleft_ao->uv[0], wall_sideleft_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p6, tiledef.wallmid_uvs[1], glm::mix(wall_sideleft_ao->uv[0], wall_sideleft_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p3, tiledef.wallmid_uvs[0], glm::mix(wall_sideleft_ao->uv[3], wall_sideleft_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p3, tiledef.walltop_uvs[3], glm::mix(wall_sideleft_ao->uv[3], wall_sideleft_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p6, tiledef.walltop_uvs[2], glm::mix(wall_sideleft_ao->uv[0], wall_sideleft_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p6, tiledef.walltop_uvs[1], glm::mix(wall_sideleft_ao->uv[0], wall_sideleft_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p3, tiledef.walltop_uvs[0], glm::mix(wall_sideleft_ao->uv[3], wall_sideleft_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p3, tiledef.wallbot_uvs[3], glm::mix(wall_sideleft_ao->uv[3], wall_sideleft_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p6, tiledef.wallbot_uvs[2], glm::mix(wall_sideleft_ao->uv[0], wall_sideleft_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p6, tiledef.wallbot_uvs[1], glm::mix(wall_sideleft_ao->uv[0], wall_sideleft_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p3, tiledef.wallbot_uvs[0], glm::mix(wall_sideleft_ao->uv[3], wall_sideleft_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_SIDERIGHT)
	{
		HSVertex m_v1(topb_p6, tiledef.wallmid_uvs[3], glm::mix(wall_sideright_ao->uv[3], wall_sideright_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p3, tiledef.wallmid_uvs[2], glm::mix(wall_sideright_ao->uv[0], wall_sideright_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p3, tiledef.wallmid_uvs[1], glm::mix(wall_sideright_ao->uv[0], wall_sideright_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p6, tiledef.wallmid_uvs[0], glm::mix(wall_sideright_ao->uv[3], wall_sideright_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p6, tiledef.walltop_uvs[3], glm::mix(wall_sideright_ao->uv[3], wall_sideright_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p3, tiledef.walltop_uvs[2], glm::mix(wall_sideright_ao->uv[0], wall_sideright_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p3, tiledef.walltop_uvs[1], glm::mix(wall_sideright_ao->uv[0], wall_sideright_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p6, tiledef.walltop_uvs[0], glm::mix(wall_sideright_ao->uv[3], wall_sideright_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p6, tiledef.wallbot_uvs[3], glm::mix(wall_sideright_ao->uv[3], wall_sideright_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p3, tiledef.wallbot_uvs[2], glm::mix(wall_sideright_ao->uv[0], wall_sideright_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p3, tiledef.wallbot_uvs[1], glm::mix(wall_sideright_ao->uv[0], wall_sideright_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p6, tiledef.wallbot_uvs[0], glm::mix(wall_sideright_ao->uv[3], wall_sideright_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_MIDTOP)
	{
		HSVertex m_v1(topb_p1, tiledef.wallmid_uvs[3], glm::mix(wall_midtop_ao->uv[3], wall_midtop_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p5, tiledef.wallmid_uvs[2], glm::mix(wall_midtop_ao->uv[0], wall_midtop_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p5, tiledef.wallmid_uvs[1], glm::mix(wall_midtop_ao->uv[0], wall_midtop_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p1, tiledef.wallmid_uvs[0], glm::mix(wall_midtop_ao->uv[3], wall_midtop_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p1, tiledef.walltop_uvs[3], glm::mix(wall_midtop_ao->uv[3], wall_midtop_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p5, tiledef.walltop_uvs[2], glm::mix(wall_midtop_ao->uv[0], wall_midtop_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p5, tiledef.walltop_uvs[1], glm::mix(wall_midtop_ao->uv[0], wall_midtop_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p1, tiledef.walltop_uvs[0], glm::mix(wall_midtop_ao->uv[3], wall_midtop_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p1, tiledef.wallbot_uvs[3], glm::mix(wall_midtop_ao->uv[3], wall_midtop_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p5, tiledef.wallbot_uvs[2], glm::mix(wall_midtop_ao->uv[0], wall_midtop_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p5, tiledef.wallbot_uvs[1], glm::mix(wall_midtop_ao->uv[0], wall_midtop_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p1, tiledef.wallbot_uvs[0], glm::mix(wall_midtop_ao->uv[3], wall_midtop_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_MIDBOT)
	{
		HSVertex m_v1(topb_p4, tiledef.wallmid_uvs[3], glm::mix(wall_midbot_ao->uv[3], wall_midbot_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p2, tiledef.wallmid_uvs[2], glm::mix(wall_midbot_ao->uv[0], wall_midbot_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p2, tiledef.wallmid_uvs[1], glm::mix(wall_midbot_ao->uv[0], wall_midbot_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p4, tiledef.wallmid_uvs[0], glm::mix(wall_midbot_ao->uv[3], wall_midbot_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p4, tiledef.walltop_uvs[3], glm::mix(wall_midbot_ao->uv[3], wall_midbot_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p2, tiledef.walltop_uvs[2], glm::mix(wall_midbot_ao->uv[0], wall_midbot_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p2, tiledef.walltop_uvs[1], glm::mix(wall_midbot_ao->uv[0], wall_midbot_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p4, tiledef.walltop_uvs[0], glm::mix(wall_midbot_ao->uv[3], wall_midbot_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p4, tiledef.wallbot_uvs[3], glm::mix(wall_midbot_ao->uv[3], wall_midbot_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p2, tiledef.wallbot_uvs[2], glm::mix(wall_midbot_ao->uv[0], wall_midbot_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p2, tiledef.wallbot_uvs[1], glm::mix(wall_midbot_ao->uv[0], wall_midbot_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p4, tiledef.wallbot_uvs[0], glm::mix(wall_midbot_ao->uv[3], wall_midbot_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_CENTER_TOP)
	{
		HSVertex m_v1(topb_p5, tiledef.wallmid_uvs[3], glm::mix(wall_centtop_ao->uv[3], wall_centtop_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p1, tiledef.wallmid_uvs[2], glm::mix(wall_centtop_ao->uv[0], wall_centtop_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p1, tiledef.wallmid_uvs[1], glm::mix(wall_centtop_ao->uv[0], wall_centtop_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p5, tiledef.wallmid_uvs[0], glm::mix(wall_centtop_ao->uv[3], wall_centtop_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p5, tiledef.walltop_uvs[3], glm::mix(wall_centtop_ao->uv[3], wall_centtop_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p1, tiledef.walltop_uvs[2], glm::mix(wall_centtop_ao->uv[0], wall_centtop_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p1, tiledef.walltop_uvs[1], glm::mix(wall_centtop_ao->uv[0], wall_centtop_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p5, tiledef.walltop_uvs[0], glm::mix(wall_centtop_ao->uv[3], wall_centtop_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p5, tiledef.wallbot_uvs[3], glm::mix(wall_centtop_ao->uv[3], wall_centtop_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p1, tiledef.wallbot_uvs[2], glm::mix(wall_centtop_ao->uv[0], wall_centtop_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p1, tiledef.wallbot_uvs[1], glm::mix(wall_centtop_ao->uv[0], wall_centtop_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p5, tiledef.wallbot_uvs[0], glm::mix(wall_centtop_ao->uv[3], wall_centtop_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_CENTER_BOT)
	{
		HSVertex m_v1(topb_p2, tiledef.wallmid_uvs[3], glm::mix(wall_centbot_ao->uv[3], wall_centbot_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p4, tiledef.wallmid_uvs[2], glm::mix(wall_centbot_ao->uv[0], wall_centbot_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p4, tiledef.wallmid_uvs[1], glm::mix(wall_centbot_ao->uv[0], wall_centbot_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p2, tiledef.wallmid_uvs[0], glm::mix(wall_centbot_ao->uv[3], wall_centbot_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p2, tiledef.walltop_uvs[3], glm::mix(wall_centbot_ao->uv[3], wall_centbot_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p4, tiledef.walltop_uvs[2], glm::mix(wall_centbot_ao->uv[0], wall_centbot_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p4, tiledef.walltop_uvs[1], glm::mix(wall_centbot_ao->uv[0], wall_centbot_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p2, tiledef.walltop_uvs[0], glm::mix(wall_centbot_ao->uv[3], wall_centbot_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p2, tiledef.wallbot_uvs[3], glm::mix(wall_centbot_ao->uv[3], wall_centbot_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p4, tiledef.wallbot_uvs[2], glm::mix(wall_centbot_ao->uv[0], wall_centbot_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p4, tiledef.wallbot_uvs[1], glm::mix(wall_centbot_ao->uv[0], wall_centbot_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p2, tiledef.wallbot_uvs[0], glm::mix(wall_centbot_ao->uv[3], wall_centbot_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_CORNER_TL)
	{
		HSVertex m_v1(topb_p2, tiledef.wallmid_uvs[3], glm::mix(wall_corntl_ao->uv[3], wall_corntl_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p6, tiledef.wallmid_uvs[2], glm::mix(wall_corntl_ao->uv[0], wall_corntl_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p6, tiledef.wallmid_uvs[1], glm::mix(wall_corntl_ao->uv[0], wall_corntl_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p2, tiledef.wallmid_uvs[0], glm::mix(wall_corntl_ao->uv[3], wall_corntl_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p2, tiledef.walltop_uvs[3], glm::mix(wall_corntl_ao->uv[3], wall_corntl_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p6, tiledef.walltop_uvs[2], glm::mix(wall_corntl_ao->uv[0], wall_corntl_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p6, tiledef.walltop_uvs[1], glm::mix(wall_corntl_ao->uv[0], wall_corntl_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p2, tiledef.walltop_uvs[0], glm::mix(wall_corntl_ao->uv[3], wall_corntl_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p2, tiledef.wallbot_uvs[3], glm::mix(wall_corntl_ao->uv[3], wall_corntl_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p6, tiledef.wallbot_uvs[2], glm::mix(wall_corntl_ao->uv[0], wall_corntl_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p6, tiledef.wallbot_uvs[1], glm::mix(wall_corntl_ao->uv[0], wall_corntl_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p2, tiledef.wallbot_uvs[0], glm::mix(wall_corntl_ao->uv[3], wall_corntl_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_CORNER_TR)
	{
		HSVertex m_v1(topb_p6, tiledef.wallmid_uvs[3], glm::mix(wall_corntr_ao->uv[3], wall_corntr_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p4, tiledef.wallmid_uvs[2], glm::mix(wall_corntr_ao->uv[0], wall_corntr_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p4, tiledef.wallmid_uvs[1], glm::mix(wall_corntr_ao->uv[0], wall_corntr_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p6, tiledef.wallmid_uvs[0], glm::mix(wall_corntr_ao->uv[3], wall_corntr_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p6, tiledef.walltop_uvs[3], glm::mix(wall_corntr_ao->uv[3], wall_corntr_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p4, tiledef.walltop_uvs[2], glm::mix(wall_corntr_ao->uv[0], wall_corntr_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p4, tiledef.walltop_uvs[1], glm::mix(wall_corntr_ao->uv[0], wall_corntr_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p6, tiledef.walltop_uvs[0], glm::mix(wall_corntr_ao->uv[3], wall_corntr_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p6, tiledef.wallbot_uvs[3], glm::mix(wall_corntr_ao->uv[3], wall_corntr_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p4, tiledef.wallbot_uvs[2], glm::mix(wall_corntr_ao->uv[0], wall_corntr_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p4, tiledef.wallbot_uvs[1], glm::mix(wall_corntr_ao->uv[0], wall_corntr_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p6, tiledef.wallbot_uvs[0], glm::mix(wall_corntr_ao->uv[3], wall_corntr_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_CORNER_BL)
	{
		HSVertex m_v1(topb_p3, tiledef.wallmid_uvs[3], glm::mix(wall_cornbl_ao->uv[3], wall_cornbl_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p1, tiledef.wallmid_uvs[2], glm::mix(wall_cornbl_ao->uv[0], wall_cornbl_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p1, tiledef.wallmid_uvs[1], glm::mix(wall_cornbl_ao->uv[0], wall_cornbl_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p3, tiledef.wallmid_uvs[0], glm::mix(wall_cornbl_ao->uv[3], wall_cornbl_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p3, tiledef.walltop_uvs[3], glm::mix(wall_cornbl_ao->uv[3], wall_cornbl_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p1, tiledef.walltop_uvs[2], glm::mix(wall_cornbl_ao->uv[0], wall_cornbl_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p1, tiledef.walltop_uvs[1], glm::mix(wall_cornbl_ao->uv[0], wall_cornbl_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p3, tiledef.walltop_uvs[0], glm::mix(wall_cornbl_ao->uv[3], wall_cornbl_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p3, tiledef.wallbot_uvs[3], glm::mix(wall_cornbl_ao->uv[3], wall_cornbl_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p1, tiledef.wallbot_uvs[2], glm::mix(wall_cornbl_ao->uv[0], wall_cornbl_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p1, tiledef.wallbot_uvs[1], glm::mix(wall_cornbl_ao->uv[0], wall_cornbl_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p3, tiledef.wallbot_uvs[0], glm::mix(wall_cornbl_ao->uv[3], wall_cornbl_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}
	if (render_sides & RENDER_CORNER_BR)
	{
		HSVertex m_v1(topb_p5, tiledef.wallmid_uvs[3], glm::mix(wall_cornbr_ao->uv[3], wall_cornbr_ao->uv[2], amb_tb), color);
		HSVertex m_v2(topb_p3, tiledef.wallmid_uvs[2], glm::mix(wall_cornbr_ao->uv[0], wall_cornbr_ao->uv[1], amb_tb), color);
		HSVertex m_v3(bott_p3, tiledef.wallmid_uvs[1], glm::mix(wall_cornbr_ao->uv[0], wall_cornbr_ao->uv[1], amb_bt), color);
		HSVertex m_v4(bott_p5, tiledef.wallmid_uvs[0], glm::mix(wall_cornbr_ao->uv[3], wall_cornbr_ao->uv[2], amb_bt), color);
		HSVertex t_v1(topt_p5, tiledef.walltop_uvs[3], glm::mix(wall_cornbr_ao->uv[3], wall_cornbr_ao->uv[2], amb_tt), color);
		HSVertex t_v2(topt_p3, tiledef.walltop_uvs[2], glm::mix(wall_cornbr_ao->uv[0], wall_cornbr_ao->uv[1], amb_tt), color);
		HSVertex t_v3(topb_p3, tiledef.walltop_uvs[1], glm::mix(wall_cornbr_ao->uv[0], wall_cornbr_ao->uv[1], amb_tb), color);
		HSVertex t_v4(topb_p5, tiledef.walltop_uvs[0], glm::mix(wall_cornbr_ao->uv[3], wall_cornbr_ao->uv[2], amb_tb), color);
		HSVertex b_v1(bott_p5, tiledef.wallbot_uvs[3], glm::mix(wall_cornbr_ao->uv[3], wall_cornbr_ao->uv[2], amb_bt), color);
		HSVertex b_v2(bott_p3, tiledef.wallbot_uvs[2], glm::mix(wall_cornbr_ao->uv[0], wall_cornbr_ao->uv[1], amb_bt), color);
		HSVertex b_v3(botb_p3, tiledef.wallbot_uvs[1], glm::mix(wall_cornbr_ao->uv[0], wall_cornbr_ao->uv[1], amb_bb), color);
		HSVertex b_v4(botb_p5, tiledef.wallbot_uvs[0], glm::mix(wall_cornbr_ao->uv[3], wall_cornbr_ao->uv[2], amb_bb), color);

		vbo_index += vbo->makeQuadPolyNorm(vbo_index, m_v1, m_v2, m_v3, m_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, t_v1, t_v2, t_v3, t_v4);
		vbo_index += vbo->makeQuadPolyNorm(vbo_index, b_v1, b_v2, b_v3, b_v4);
	}

	
	/*
	if (vbo_index < vbo->getCapacity())
		vbo->degenTris(vbo_index, m_vbo->getCapacity() - vbo_index);
		*/

	return vbo_index - vbo_start;
}