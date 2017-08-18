#include "AmbientOcclusion.h"

AmbientOcclusion::AmbientOcclusion()
{

}

AmbientOcclusion::~AmbientOcclusion()
{

}

void AmbientOcclusion::makeRaysFloor(std::vector<glm::vec3>& rays, int numrays)
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
	glm::vec2 lv = glm::normalize(wall_point1 - sample_point);
	glm::vec2 rv = glm::normalize(wall_point2 - sample_point);

	glm::vec2 ray2d = glm::normalize(glm::vec2(ray));

	glm::vec2 n = glm::vec2(-ray2d.y, ray2d.x);		// normal of 2d ray

	float dot1 = glm::dot(n, lv);
	float dot2 = glm::dot(n, rv);

	if (dot1 >= 0 && dot2 <= 0)
	{
		glm::vec2 plane = wall_point2 - wall_point1;
		glm::vec2 plane_n(-plane.y, plane.x);

		
		glm::vec3 p1(wall_point1.x, wall_point1.y, 0.0f);
		glm::vec3 pn(plane_n.x, plane_n.y, 0.0f);

		glm::vec3 vp(sample_point.x, sample_point.y, 0.0f);


		float dist = 0.0f;
		bool result = glm::intersectRayPlane(vp, ray, p1, pn, dist);

		glm::vec3 point = vp + ray * dist;

		if (dist >= 0 && dist < 0.1f && point.z >= 0 && point.z < 0.5f)
			return true;
		//if (point.z < 0.5f)
		//	return true;
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

					//if (sides & SIDE_LEFT)
					/*
					if (1)
					{
						glm::vec2 lv = glm::vec2(0.0f, 0.7f) - p;
						glm::vec2 rv = glm::vec2(0.0f, 0.3f) - p;

						glm::vec2 ncv = glm::vec2(-cr.y, cr.x);

						float dot1 = glm::dot(ncv, lv);
						float dot2 = glm::dot(ncv, rv);

						if (dot1 <= 0 && dot2 <= 0)
							hit = true;
					}
					*/
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

void AmbientOcclusion::calculate()
{
	int width = 32;
	int height = 32;

	int* buffer[64];
	for (int i = 0; i < 64; i++)
	{
		buffer[i] = new int[width * height];
	}

	std::vector<glm::vec3> rays;

	makeRaysFloor(rays, 2000);

	for (int i = 0; i < 64; i++)
	{
		calculateFloor(i, width, height, buffer[i], rays);
	}


	
	/*
	glm::vec2 p1(0.0f, 0.7f);
	glm::vec2 p2(0.0f, 0.3f);
	glm::vec2 v = p2 - p1;
	glm::vec2 pn(-v.y, v.x);

	glm::vec2 px(0.5f, 0.5f);
	glm::vec2 pd(-0.5f, 0.0f);

	float dist = 0.0f;

	bool result = glm::intersectRayPlane(px, glm::normalize(p1-px), p1, pn, dist);
	*/
	/*
	glm::vec3 p1(0.0f, 0.7f, 0.0f);
	glm::vec3 pn(1.0f, 0.0f, 0.0f);

	glm::vec3 v(0.5f, 0.5f, 0.0);
	glm::vec3 nv = glm::normalize(glm::vec3(-1.0f, 0.0f, 0.3f));

	float dist = 0.0f;
	bool result = glm::intersectRayPlane(v, nv, p1, pn, dist);

	glm::vec3 point = v + nv * dist;
	*/




	QImage* img = new QImage(width*8, height*8, QImage::Format_ARGB32);
	for (int x = 0; x < 64; x++)
	{
		int ix = x % 8;
		int iy = x / 8;
		for (int j = 0; j < height; j++)
		{
			QRgb* line = (QRgb*)img->scanLine(j+(iy*height));
			for (int i = 0; i < width; i++)
			{
				line[i+(ix*width)] = buffer[x][(j * width) + i];
			}
		}
	}

	img->save("ambient.png", "PNG");

	delete img;

	for (int i = 0; i < 64; i++)
	{
		delete[] buffer[i];
	}
}