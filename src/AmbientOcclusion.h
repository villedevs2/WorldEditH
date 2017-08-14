#pragma once

#include <glm/glm.hpp>
#include <gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <random>


class AmbientOcclusion
{
public:
	AmbientOcclusion();
	~AmbientOcclusion();

private:
	void makeRays(std::vector<glm::vec3> rays, int numrays, const glm::vec3& origin, const glm::vec3& normal);
};