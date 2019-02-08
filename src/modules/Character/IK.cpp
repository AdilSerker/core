#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "character/IK.h"

IK::IK()
	: fade(0.075), threshold(0.8), smoothness(0.5), heel_height(5.0), toe_height(4.0)
{
	memset(lock, 4, sizeof(float));
	memset(position, 4, sizeof(glm::vec3));
	memset(height, 4, sizeof(float));
}

void IK::two_joint(
	glm::vec3 a, glm::vec3 b,
	glm::vec3 c, glm::vec3 t, float eps,
	glm::mat4 &a_pR, glm::mat4 &b_pR,
	glm::mat4 &a_gR, glm::mat4 &b_gR,
	glm::mat4 &a_lR, glm::mat4 &b_lR)
{

	float lc = glm::length(b - a);
	float la = glm::length(b - c);
	float lt = glm::clamp(glm::length(t - a), eps, lc + la - eps);

	if (glm::length(c - t) < eps)
	{
		return;
	}

	float ac_ab_0 = acosf(glm::clamp(glm::dot(glm::normalize(c - a), glm::normalize(b - a)), -1.0f, 1.0f));
	float ba_bc_0 = acosf(glm::clamp(glm::dot(glm::normalize(a - b), glm::normalize(c - b)), -1.0f, 1.0f));
	float ac_at_0 = acosf(glm::clamp(glm::dot(glm::normalize(c - a), glm::normalize(t - a)), -1.0f, 1.0f));

	float ac_ab_1 = acosf(glm::clamp((la * la - lc * lc - lt * lt) / (-2 * lc * lt), -1.0f, 1.0f));
	float ba_bc_1 = acosf(glm::clamp((lt * lt - lc * lc - la * la) / (-2 * lc * la), -1.0f, 1.0f));

	glm::vec3 a0 = glm::normalize(glm::cross(b - a, c - a));
	glm::vec3 a1 = glm::normalize(glm::cross(t - a, c - a));

	glm::mat3 r0 = glm::mat3(glm::rotate(ac_ab_1 - ac_ab_0, -a0));
	glm::mat3 r1 = glm::mat3(glm::rotate(ba_bc_1 - ba_bc_0, -a0));
	glm::mat3 r2 = glm::mat3(glm::rotate(ac_at_0, -a1));

	glm::mat3 a_lRR = glm::inverse(glm::mat3(a_pR)) * (r2 * r0 * glm::mat3(a_gR));
	glm::mat3 b_lRR = glm::inverse(glm::mat3(b_pR)) * (r1 * glm::mat3(b_gR));

	for (int x = 0; x < 3; x++)
		for (int y = 0; y < 3; y++)
		{
			a_lR[x][y] = a_lRR[x][y];
			b_lR[x][y] = b_lRR[x][y];
		}
}

void IK::reset(glm::vec3 root_position)
{
	position[IK::HL] = glm::vec3(0, 0, 0);
	lock[IK::HL] = 0;
	height[IK::HL] = root_position.y;
	position[IK::HR] = glm::vec3(0, 0, 0);
	lock[IK::HR] = 0;
	height[IK::HR] = root_position.y;
	position[IK::TL] = glm::vec3(0, 0, 0);
	lock[IK::TL] = 0;
	height[IK::TL] = root_position.y;
	position[IK::TR] = glm::vec3(0, 0, 0);
	lock[IK::TR] = 0;
	height[IK::TR] = root_position.y;
}
