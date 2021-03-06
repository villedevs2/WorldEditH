#pragma once

#include <glm.hpp>
#include <qimage.h>

#include "Tilemap.h"

Q_DECLARE_METATYPE(QImage*);

namespace Thumbnail
{	
	void fromPoly(QImage* image, QImage* texture, const glm::vec4* points, int num_points);
	void fromRect(QImage* image, QImage* texture, const glm::vec2* points);
	//void fromTileType(QImage* image, QImage* texture, const glm::vec2* top, int numtop, const glm::vec2* side, int numside, Tilemap::TileType type);
	
	void fillTriangle(QImage* image, QImage* texture, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3);

};