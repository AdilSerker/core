#ifndef CHARACTER_H
#define CHARACTER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <fstream>

#include "Trajectory.h"
#include "Options.h"
#include "IK.cpp"

#include "../Shader.h"

#include "../Scene/Heightmap.h"
#include "../Scene/Areas.h"

#include "../LightDirectional.h"
#include "../CameraOrbit.h"

using namespace std;

class Character
{
	public:
	Character();
	~Character();

	void draw(LightDirectional *light, CameraOrbit *camera);
	glm::vec3 getPosition();
	void reset_position(glm::vec2 position, Heightmap *heightmap, Areas *areas);
	void update_move(glm::vec2 direction_velocity, glm::vec3 cam_direct, int vel, int strafe, bool is_crouched);

	private:
	Trajectory *trajectory;
	IK *ik;

	Shader *shader;	

	enum
	{
		JOINT_NUM = 31
	};

	GLuint vbo, tbo;
	int ntri, nvtx;
	float phase;
	float strafe_amount;
	float strafe_target;
	float crouched_amount;
	float crouched_target;
	float responsive;

	glm::vec3 joint_positions[JOINT_NUM];
	glm::vec3 joint_velocities[JOINT_NUM];
	glm::mat3 joint_rotations[JOINT_NUM];

	glm::mat4 joint_anim_xform[JOINT_NUM];
	glm::mat4 joint_rest_xform[JOINT_NUM];
	glm::mat4 joint_mesh_xform[JOINT_NUM];
	glm::mat4 joint_global_rest_xform[JOINT_NUM];
	glm::mat4 joint_global_anim_xform[JOINT_NUM];

	int joint_parents[JOINT_NUM];

	glm::vec3 root_position;
	glm::mat3 root_rotation;

	Heightmap *heightmap;
	Areas *areas;

	enum
	{
		JOINT_ROOT_L = 1,
		JOINT_HIP_L = 2,
		JOINT_KNEE_L = 3,
		JOINT_HEEL_L = 4,
		JOINT_TOE_L = 5,

		JOINT_ROOT_R = 6,
		JOINT_HIP_R = 7,
		JOINT_KNEE_R = 8,
		JOINT_HEEL_R = 9,
		JOINT_TOE_R = 10
	};
	
	void load(const char *filename_v, const char *filename_t, const char *filename_p, const char *filename_r);
	
	void build_local_transform();

	void forward_kinematics();

	void set_ik();

	void rotate_hip_knee(glm::vec3 key_hl, glm::vec3 key_hr);

	void rotate_heel(glm::vec4 ik_weight, glm::vec3 key_tl, glm::vec3 key_tr);

	void rotate_toe(glm::vec4 ik_weight);

	void update_locks(glm::vec4 ik_weight);
};

#endif // !CHARACTER_H
