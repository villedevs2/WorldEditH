#include "AmbientOcclusion.h"

AmbientOcclusion::AmbientOcclusion()
{

}

AmbientOcclusion::~AmbientOcclusion()
{

}

void AmbientOcclusion::makeRays(std::vector<glm::vec3> rays, int numrays, const glm::vec3& origin, const glm::vec3& normal)
{
	std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
	std::default_random_engine generator;
}