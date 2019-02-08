#ifndef IK_H
#define IK_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class IK
{
  public:
	enum
	{
		HL = 0,
		HR = 1,
		TL = 2,
		TR = 3
	};

	float lock[4];
	glm::vec3 position[4];
	float height[4];
	float fade;
	float threshold;
	float smoothness;
	float heel_height;
	float toe_height;

	IK();

	void two_joint(
		glm::vec3 a, glm::vec3 b,
		glm::vec3 c, glm::vec3 t, float eps,
		glm::mat4 &a_pR, glm::mat4 &b_pR,
		glm::mat4 &a_gR, glm::mat4 &b_gR,
		glm::mat4 &a_lR, glm::mat4 &b_lR);

	void reset(glm::vec3 root_position);
};

#endif //IK_H