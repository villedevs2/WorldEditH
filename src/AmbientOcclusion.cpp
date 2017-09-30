#include "AmbientOcclusion.h"

const float AmbientOcclusion::RAY_FALLOFF = 0.5f;

AmbientOcclusion::AmbientOcclusion()
{
	m_map = nullptr;

	for (int i = 0; i < 64; i++)
	{
		int ao_tile_x = i % NUM_FLOOR_TILES_X;
		int ao_tile_y = i / NUM_FLOOR_TILES_Y;

		float tile_w = 1.0f / (float)(NUM_TOTAL_TILES_X);
		float tile_h = 1.0f / (float)(NUM_TOTAL_TILES_Y);
		float tile_x = ao_tile_x * tile_w;
		float tile_y = ao_tile_y * tile_h;

		m_floor_tiles[i].uv[0] = glm::vec2(tile_x, tile_y + (0.3f * tile_h));
		m_floor_tiles[i].uv[1] = glm::vec2(tile_x, tile_y + (0.7f * tile_h));
		m_floor_tiles[i].uv[2] = glm::vec2(tile_x + (0.5f * tile_w), tile_y + tile_h);
		m_floor_tiles[i].uv[3] = glm::vec2(tile_x + tile_w, tile_y + (0.7f * tile_h));
		m_floor_tiles[i].uv[4] = glm::vec2(tile_x + tile_w, tile_y + (0.3f * tile_h));
		m_floor_tiles[i].uv[5] = glm::vec2(tile_x + (0.5f * tile_w), tile_y);

		m_floor_tiles[i].center = glm::vec2(tile_x + (tile_w * 0.5f), tile_y + (tile_h * 0.5f));
	}

	for (int i = 0; i < 8; i++)
	{
		int ao_tile_x = i % NUM_TOTAL_TILES_X;
		int ao_tile_y = NUM_FLOOR_TILES_Y;

		float tile_w = 1.0f / (float)(NUM_TOTAL_TILES_X);
		float tile_h = 1.0f / (float)(NUM_TOTAL_TILES_Y);
		float tile_x = ao_tile_x * tile_w;
		float tile_y = ao_tile_y * tile_h;

		m_wall_tiles[i].uv[0] = glm::vec2(tile_x, tile_y);
		m_wall_tiles[i].uv[1] = glm::vec2(tile_x + tile_w, tile_y);
		m_wall_tiles[i].uv[2] = glm::vec2(tile_x + tile_w, tile_y + tile_h);
		m_wall_tiles[i].uv[3] = glm::vec2(tile_x, tile_y + tile_h);
	}
}

AmbientOcclusion::~AmbientOcclusion()
{
	if (m_map != nullptr)
		delete m_map;
}

void AmbientOcclusion::makeRays(std::vector<glm::vec3>& rays, int numrays)
{
	std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
	std::default_random_engine generator;

	float scale = 1.0f;

	for (int i = 0; i < numrays; i++)
	{
		glm::vec3 sample(
			random_floats(generator) * 2.0 - 1.0,
			random_floats(generator) * 2.0 - 1.0,
			random_floats(generator)
		);
		sample = glm::normalize(sample);
		sample *= scale;

		rays.push_back(sample);
	}
}

bool AmbientOcclusion::sampleFloorHit(const glm::vec2& wall_point1, const glm::vec2& wall_point2, const glm::vec2& sample_point, const glm::vec3& ray)
{
	const float WALL_HEIGHT = 0.5f;

	glm::vec2 lv = glm::normalize(wall_point1 - sample_point);
	glm::vec2 rv = glm::normalize(wall_point2 - sample_point);

	glm::vec2 nlv(-lv.y, lv.x);
	glm::vec2 nrv(-rv.y, rv.x);

	glm::vec2 ray2d = glm::normalize(glm::vec2(ray));

	glm::vec2 n = glm::vec2(-ray2d.y, ray2d.x);		// normal of 2d ray

	float dot1 = glm::dot(ray2d, nlv);
	float dot2 = glm::dot(ray2d, nrv);

	if (dot1 >= 0 && dot2 <= 0)
	{
		glm::vec2 plane = wall_point2 - wall_point1;
		glm::vec2 plane_n = glm::normalize(glm::vec2(-plane.y, plane.x));

		
		glm::vec3 p1(wall_point1.x, wall_point1.y, 0.0f);
		glm::vec3 pn(plane_n.x, plane_n.y, 0.0f);

		glm::vec3 vp(sample_point.x, sample_point.y, 0.0f);


		float dist = 0.0f;
		bool result = glm::intersectRayPlane(vp, ray, p1, pn, dist);

		glm::vec3 point = vp + ray * dist;

		float raylen = glm::length(point - vp);

		if (raylen >= 0.0f && raylen < AmbientOcclusion::RAY_FALLOFF && point.z >= 0 && point.z < WALL_HEIGHT)
			return true;
	}

	return false;
}

void AmbientOcclusion::calculateFloor(int sides, int width, int height, int* buffer, std::vector<glm::vec3>& rays)
{
	PolygonDef poly(6);

	poly.insertPoint(glm::vec2(0.0f, 0.3f));
	poly.insertPoint(glm::vec2(0.0f, 0.7f));
	poly.insertPoint(glm::vec2(0.5f, 1.0f));
	poly.insertPoint(glm::vec2(1.0f, 0.7f));
	poly.insertPoint(glm::vec2(1.0f, 0.3f));
	poly.insertPoint(glm::vec2(0.5f, 0.0f));

	double oow = 1.0 / width;
	double ooh = 1.0 / height;

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			int hit_samples = 0;

			glm::vec2 p((float)(i) * oow, (float)(j) * ooh);
			if (poly.isPointInside(p))
			{
#if 1
				for (int r = 0; r < rays.size(); r++)
				{
					glm::vec3& cr = rays.at(r);

					bool hit = false;

					if (!hit && (sides & SIDE_LEFT))		// left
						hit = sampleFloorHit(glm::vec2(0.0f, 0.7f), glm::vec2(0.0f, 0.3f), p, cr);
					if (!hit && (sides & SIDE_TOPLEFT))		// top left
						hit = sampleFloorHit(glm::vec2(0.0f, 0.3f), glm::vec2(0.5f, 0.0f), p, cr);
					if (!hit && (sides & SIDE_TOPRIGHT))	// top right
						hit = sampleFloorHit(glm::vec2(0.5f, 0.0f), glm::vec2(1.0f, 0.3f), p, cr);
					if (!hit && (sides & SIDE_RIGHT))		// right
						hit = sampleFloorHit(glm::vec2(1.0f, 0.3f), glm::vec2(1.0f, 0.7f), p, cr);
					if (!hit && (sides & SIDE_BOTRIGHT))	// bottom right
						hit = sampleFloorHit(glm::vec2(1.0f, 0.7f), glm::vec2(0.5f, 1.0f), p, cr);
					if (!hit && (sides & SIDE_BOTLEFT))		// bottom left
						hit = sampleFloorHit(glm::vec2(0.5f, 1.0f), glm::vec2(0.0f, 0.7f), p, cr);

					if (hit)
						hit_samples++;
				}

				// monte carlo on ray hits
				float value = 1.0f - ((float)(hit_samples) / (float)(rays.size()));

				buffer[(j * width) + i] = 0xffff0000 | (int)(value * 255.0f) << 8;
#endif
#if 0
				glm::vec2 w1(0.0f, 0.3f);
				glm::vec2 w2(0.0f, 0.7f);

				glm::vec2 lv = w2 - p;
				glm::vec2 rv = w1 - p;

				float dot = glm::dot(lv, rv);
				float ang = cos(dot) * 180.0f / M_PI;

				//float ang1 = atan2(w1.y - p.y, w1.x - p.x) * 180.0f / M_PI;
				//float ang2 = atan2(w2.y - p.y, w2.x - p.y) * 180.0f / M_PI;

				//float diff = ang2 - ang1;

				float value = ang / 360.0f;
				buffer[(j * width) + i] = 0xffff0000 | (int)(value * 255.0f) << 8;
#endif
			}
		}
	}
}

void AmbientOcclusion::calculateWall(int sides, int width, int height, int* buffer, std::vector<glm::vec3>& rays)
{
	glm::vec3 pt_row[4][2];

	pt_row[0][0] = glm::vec3(0.0f, 0.0f, 0.5f);	pt_row[0][1] = glm::vec3(1.0f, 0.0f, 0.5f);
	pt_row[1][0] = glm::vec3(0.0f, 0.3f, 0.0f); pt_row[1][1] = glm::vec3(1.0f, 0.3f, 0.0f);
	pt_row[2][0] = glm::vec3(0.0f, 0.7f, 0.0f); pt_row[2][1] = glm::vec3(1.0f, 0.7f, 0.0f);
	pt_row[3][0] = glm::vec3(0.0f, 1.0f, 0.5f); pt_row[3][1] = glm::vec3(1.0f, 1.0f, 0.5f);

	glm::vec3 pt_top = glm::vec3(1.0f, 0.3f, 1.0f);
	glm::vec3 pt_bot = glm::vec3(1.0f, 0.7f, 1.0f);

	double oow = (1.0 / width) * 1.0f;
	double ooh = (1.0 / height) * (0.7f - 0.3f);
	double hoow = oow * 0.5;
	double hooh = ooh * 0.5;

	const float RAY_FALLOFF = 0.5f;

	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			int hit_samples = 0;

			glm::vec3 p(((float)(i) * oow) + hoow, ((float)(j)* ooh) + hooh + 0.3f, 0.0f);
			for (int r = 0; r < rays.size(); r++)
			{
				glm::vec3 bary_pos;
				glm::vec3& cr = rays.at(r);

				bool hit = false;

				if (sides & WALLSIDE_LEFT)
				{
					if (glm::intersectRayTriangle(p, cr, pt_row[1][0], pt_row[0][0], pt_row[0][1], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
					if (glm::intersectRayTriangle(p, cr, pt_row[1][0], pt_row[0][1], pt_row[1][1], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
				}
				if (sides & WALLSIDE_RIGHT)
				{
					if (glm::intersectRayTriangle(p, cr, pt_row[2][1], pt_row[3][1], pt_row[3][0], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
					if (glm::intersectRayTriangle(p, cr, pt_row[2][1], pt_row[3][0], pt_row[2][0], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
				}
				if (sides & WALLSIDE_FLOOR)
				{
					if (glm::intersectRayTriangle(p, cr, pt_top, pt_row[1][1], pt_row[0][1], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
					if (glm::intersectRayTriangle(p, cr, pt_top, pt_bot, pt_row[1][1], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
					if (glm::intersectRayTriangle(p, cr, pt_bot, pt_row[2][1], pt_row[1][1], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
					if (glm::intersectRayTriangle(p, cr, pt_bot, pt_row[3][1], pt_row[2][1], bary_pos))
					{
						if (bary_pos.z < AmbientOcclusion::RAY_FALLOFF)
							hit = true;
					}
				}
				if (hit)
					hit_samples++;
			}

			// monte carlo on ray hits
			float value = 1.0f - ((float)(hit_samples) / (float)(rays.size()));

			buffer[(j * width) + i] = 0xffff0000 | (int)(value * 255.0f) << 8;
		}
	}
}


void AmbientOcclusion::calculate()
{
	m_map = new QImage(MAP_WIDTH*NUM_FLOOR_TILES_X, MAP_HEIGHT*NUM_TOTAL_TILES_Y, QImage::Format_ARGB32);

	int* floor_buffer[64];
	int* wall_buffer[8];
	for (int i = 0; i < 64; i++)
	{
		floor_buffer[i] = new int[MAP_WIDTH * MAP_HEIGHT];
	}
	for (int i = 0; i < 8; i++)
	{
		wall_buffer[i] = new int[MAP_WIDTH * MAP_HEIGHT];
	}

	std::vector<glm::vec3> rays;

	makeRays(rays, 2000);

	for (int i = 0; i < 64; i++)
	{
		calculateFloor(i, MAP_WIDTH, MAP_HEIGHT, floor_buffer[i], rays);
	}
	
	for (int i = 0; i < 8; i++)
	{
		calculateWall(i, MAP_WIDTH, MAP_HEIGHT, wall_buffer[i], rays);
	}

	
	for (int x = 0; x < 64; x++)
	{
		int ix = x % NUM_FLOOR_TILES_X;
		int iy = x / NUM_FLOOR_TILES_Y;
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			QRgb* line = (QRgb*)m_map->scanLine(j+(iy*MAP_HEIGHT));
			for (int i = 0; i < MAP_WIDTH; i++)
			{
				line[i+(ix*MAP_WIDTH)] = floor_buffer[x][(j * MAP_WIDTH) + i];
			}
		}
	}

	for (int x = 0; x < 8; x++)
	{
		int ix = x % NUM_FLOOR_TILES_X;
		int iy = 64 / NUM_FLOOR_TILES_Y;
		for (int j = 0; j < MAP_HEIGHT; j++)
		{
			QRgb* line = (QRgb*)m_map->scanLine(j + (iy * MAP_HEIGHT));
			for (int i = 0; i < MAP_WIDTH; i++)
			{
				line[i + (ix * MAP_WIDTH)] = wall_buffer[x][(j * MAP_WIDTH) + i];
			}
		}
	}

	m_map->save("ambient.png", "PNG");

	for (int i = 0; i < 64; i++)
	{
		delete[] floor_buffer[i];
	}
	for (int i = 0; i < 8; i++)
	{
		delete[] wall_buffer[i];
	}
}

QImage* AmbientOcclusion::getMap()
{
	return m_map;
}

bool AmbientOcclusion::load(QString name)
{
	m_map = new QImage();
	if (m_map->load(name, "PNG"))
		return true;

	delete m_map;
	return false;
}

const AmbientOcclusion::AOFloorTile& AmbientOcclusion::getFloorTile(int tile)
{
	assert(tile >= 0 && tile < 64);

	return m_floor_tiles[tile];
}

const AmbientOcclusion::AOWallTile& AmbientOcclusion::getWallTile(int tile)
{
	assert(tile >= 0 && tile < 8);

	return m_wall_tiles[tile];
}