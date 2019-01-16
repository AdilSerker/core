#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

/* Helper Functions */

static glm::vec3 mix_directions(glm::vec3 x, glm::vec3 y, float a)
{
	glm::quat x_q = glm::angleAxis(atan2f(x.x, x.z), glm::vec3(0, 1, 0));
	glm::quat y_q = glm::angleAxis(atan2f(y.x, y.z), glm::vec3(0, 1, 0));
	glm::quat z_q = glm::slerp(x_q, y_q, a);
	return z_q * glm::vec3(0, 0, 1);
}

static glm::mat4 mix_transforms(glm::mat4 x, glm::mat4 y, float a)
{
	glm::mat4 out = glm::mat4(glm::slerp(glm::quat(x), glm::quat(y), a));
	out[3] = mix(x[3], y[3], a);
	return out;
}

static glm::quat quat_exp(glm::vec3 l)
{
	float w = glm::length(l);
	glm::quat q = w < 0.01 ? glm::quat(1, 0, 0, 0) : glm::quat(cosf(w), l.x * (sinf(w) / w), l.y * (sinf(w) / w), l.z * (sinf(w) / w));
	return q / sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

static glm::vec2 segment_nearest(glm::vec2 v, glm::vec2 w, glm::vec2 p)
{
	float l2 = glm::dot(v - w, v - w);
	if (l2 == 0.0)
		return v;
	float t = glm::clamp(glm::dot(p - v, w - v) / l2, 0.0f, 1.0f);
	return v + t * (w - v);
}
