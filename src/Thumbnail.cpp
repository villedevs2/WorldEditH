#include "Thumbnail.h"

void Thumbnail::fillTriangle(QImage* image, QImage* texture, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3)
{
	float x1, x2, z1, z2, w1, w2;

	glm::vec4 p4;

	if (p2.y < p1.y)
	{
		glm::vec4 tmp = p1;
		p1 = p2;
		p2 = tmp;
	}
	if (p3.y < p1.y)
	{
		glm::vec4 tmp = p1;
		p1 = p3;
		p3 = tmp;
	}
	if (p3.y < p2.y)
	{
		glm::vec4 tmp = p2;
		p2 = p3;
		p3 = tmp;
	}

	float dx31 = (p3.x - p1.x) / (p3.y - p1.y);
	float dz31 = (p3.z - p1.z) / (p3.y - p1.y);
	float dw31 = (p3.w - p1.w) / (p3.y - p1.y);

	p4 = glm::vec4(p1.x + ((p2.y - p1.y) * dx31),
				   p2.y,
				   p1.z + ((p2.y - p1.y) * dz31),
				   p1.w + ((p2.y - p1.y) * dw31));

	if (p2.x > p4.x)
	{
		glm::vec4 tmp = p2;
		p2 = p4;
		p4 = tmp;
	}

	float dx21 = (p2.x - p1.x) / (p2.y - p1.y);
	float dz21 = (p2.z - p1.z) / (p2.y - p1.y);
	float dw21 = (p2.w - p1.w) / (p2.y - p1.y);

	float dx41 = (p4.x - p1.x) / (p4.y - p1.y);
	float dz41 = (p4.z - p1.z) / (p4.y - p1.y);
	float dw41 = (p4.w - p1.w) / (p4.y - p1.y);

	float dx32 = (p3.x - p2.x) / (p3.y - p2.y);
	float dz32 = (p3.z - p2.z) / (p3.y - p2.y);
	float dw32 = (p3.w - p2.w) / (p3.y - p2.y);

	float dx34 = (p3.x - p4.x) / (p3.y - p4.y);
	float dz34 = (p3.z - p4.z) / (p3.y - p4.y);
	float dw34 = (p3.w - p4.w) / (p3.y - p4.y);

	int y1 = (int)(p1.y);
	int y2 = (int)(p2.y);
	int y3 = (int)(p3.y);

	x1 = p1.x;
	x2 = p1.x;
	z1 = p1.z;
	z2 = p1.z;
	w1 = p1.w;
	w2 = p1.w;

	for (int j=y1; j < y2; j++)
	{
		int sx = (int)(x1);
		int ex = (int)(x2);

		float zd = (z2 - z1) / (x2 - x1);
		float wd = (w2 - w1) / (x2 - x1);
		float z = z1;
		float w = w1;

		for (int k=sx; k < ex; k++)
		{
			int u = (int)(z);
			int v = (int)(w);

			QRgb texel = texture->pixel(u, v);
			image->setPixel(k, j, texel);
			z += zd;
			w += wd;
		}

		x1 += dx21;
		x2 += dx41;
		z1 += dz21;
		z2 += dz41;
		w1 += dw21;
		w2 += dw41;
	}

	x1 = p2.x;
	x2 = p4.x;
	z1 = p2.z;
	z2 = p4.z;
	w1 = p2.w;
	w2 = p4.w;

	for (int j=y2; j < y3; j++)
	{
		int sx = (int)(x1);
		int ex = (int)(x2);

		float zd = (z2 - z1) / (x2 - x1);
		float wd = (w2 - w1) / (x2 - x1);
		float z = z1;
		float w = w1;

		for (int k=sx; k < ex; k++)
		{
			int u = (int)(z);
			int v = (int)(w);

			QRgb texel = texture->pixel(u, v);
			image->setPixel(k, j, texel);

			z += zd;
			w += wd;
		}

		x1 += dx32;
		x2 += dx34;
		z1 += dz32;
		z2 += dz34;
		w1 += dw32;
		w2 += dw34;
	}
}


void Thumbnail::fromPoly(QImage* image, QImage* texture, const glm::vec4* points, int num_points)
{
	float texwidth = (float)texture->width();
	float texheight = (float)texture->height();

	float imgwidth = (float)image->width() - 1;
	float imgheight = (float)image->height() - 1;

	image->fill(0);

	for (int i=2; i < num_points; i++)
	{
		float x1 = floor((points[0].x) * imgwidth + 0.5f);
		float y1 = floor((points[0].y) * imgheight + 0.5f);
		float x2 = floor((points[i-1].x) * imgwidth + 0.5f);
		float y2 = floor((points[i-1].y) * imgheight + 0.5f);
		float x3 = floor((points[i].x) * imgwidth + 0.5f);
		float y3 = floor((points[i].y) * imgheight + 0.5f);

		float u1 = points[0].z * texwidth;
		float v1 = points[0].w * texheight;
		float u2 = points[i-1].z * texwidth;
		float v2 = points[i-1].w * texheight;
		float u3 = points[i].z * texwidth;
		float v3 = points[i].w * texheight;

		fillTriangle(image, texture, glm::vec4(x1, y1, u1, v1), glm::vec4(x2, y2, u2, v2), glm::vec4(x3, y3, u3, v3));
	}
}

void Thumbnail::fromRect(QImage* image, QImage* texture, const glm::vec2* points)
{
	float texwidth = (float)texture->width();
	float texheight = (float)texture->height();

	float imgwidth = (float)image->width() - 1;
	float imgheight = (float)image->height() - 1;

	glm::vec4 p[4];

	p[0].x = 0.0f;		p[0].y = 0.0f;		p[0].z = points[0].x * texwidth;		p[0].w = points[0].y * texheight;
	p[1].x = 0.0f;		p[1].y = imgheight;	p[1].z = points[1].x * texwidth;		p[1].w = points[1].y * texheight;
	p[2].x = imgwidth;	p[2].y = imgheight;	p[2].z = points[2].x * texwidth;		p[2].w = points[2].y * texheight;
	p[3].x = imgwidth;	p[3].y = 0.0f;		p[3].z = points[3].x * texwidth;		p[3].w = points[3].y * texheight;

	fillTriangle(image, texture, p[0], p[1], p[2]);
	fillTriangle(image, texture, p[0], p[2], p[3]);
}
/*
void Thumbnail::fromTileType(QImage* image, QImage* texture, const glm::vec2* top, int numtop, const glm::vec2* side, int numside, Tilemap::TileType type)
{
	float texwidth = (float)texture->width();
	float texheight = (float)texture->height();

	float imgwidth = (float)image->width() - 1;
	float imgheight = (float)image->height() - 1;

	float minx = top[0].x;
	float maxx = top[0].x;
	float miny = top[0].y;
	float maxy = top[0].y;

	for (int i = 0; i < numtop; i++)
	{
		if (top[i].x < minx)
			minx = top[i].x;
		if (top[i].x > maxx)
			maxx = top[i].x;
		if (top[i].y < miny)
			miny = top[i].y;
		if (top[i].y > maxy)
			maxy = top[i].y;
	}

	float side_length_x = maxx - minx;
	float side_length_y = maxy - miny;
	float side_length;
	float side_offset_x = 0.0f;
	float side_offset_y = 0.0f;
	if (side_length_y > side_length_x)
	{
		side_length = side_length_y;
		side_offset_x = (side_length_y - side_length_x) / 2;
	}
	else
	{
		side_length = side_length_x;
		side_offset_y = (side_length_x - side_length_y) / 2;
	}

	glm::vec4 p[6];

	for (int i = 0; i < numtop; i++)
	{
		p[i].x = (((top[i].x - minx + side_offset_x) / side_length) ) * imgwidth;
		p[i].y = (((top[i].y - miny + side_offset_y) / side_length) ) * imgheight;
		p[i].z = top[i].x * texwidth;
		p[i].w = top[i].y * texheight;
	}

	for (int i = 2; i < numtop; i++)
	{
		fillTriangle(image, texture, p[0], p[i - 1], p[i]);
	}
}
*/