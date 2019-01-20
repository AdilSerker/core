#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Options.h"
#include "PFNN.h"

#include "../Scene/Areas.h"
#include "../Scene/Heightmap.h"

#include "../helpers.hpp"

using namespace glm;

class Trajectory
{
	public:
	enum
	{
		LENGTH = 120
	};

	PFNN *pfnn;

	vec3 target_dir, target_vel;

	Trajectory()
		: width(25), target_dir(vec3(0, 0, 1)), target_vel(vec3(0))
	{
		this->pfnn = new PFNN(MODE_CONSTANT);
	}

	~Trajectory() {
		delete this->pfnn;
	}

	vec3 get_center_position();

	void reset(vec3 root_position, mat3 root_rotation);

	void update_gait(int vel, float crouched_amount, float extra_gait_smooth);

	void predict(float responsive, float strafe_amount, Areas *areas);

	void input(Heightmap *heightmap, int JOINT_NUM,
		vec3 *root_position, mat3 *root_rotation, vec3 *joint_positions, vec3 *joint_velocities);


	vec3 getPosition(int opos, int i);
	vec3 getVelocity(int ovel, int i);
	vec3 getRotation(int orot, int i);

	void post_update(float *phase, Areas *areas);

	private:
	float width;

	vec3 positions[LENGTH];
	vec3 directions[LENGTH];
	mat3 rotations[LENGTH];
	float heights[LENGTH];

	float gait_stand[LENGTH];
	float gait_walk[LENGTH];
	float gait_jog[LENGTH];
	float gait_crouch[LENGTH];
	float gait_jump[LENGTH];
	float gait_bump[LENGTH];

	void predict_trajectory(float responsive, float strafe_amount, Areas *areas);
	void predict_jump(Areas *areas);
	void predict_crouch(Areas *areas);
	void predict_wall(Areas *areas);

	void input_position(vec3 pos, int w, int i);
	void input_direction(vec3 dir, int w, int i);
	void input_gaits(int w, int i);
	void input_previous_state(vec3 pos, vec3 prv, int i, int JOINT_NUM);
	void input_heights(vec3 root_position, float position_r, float position_l, int o, int w, int i);

	void update_past();
	void update_current(float stand_amount);
	void update_future();

};

#endif // !TRAJECTORY_H