#include "Trajectory.h"

glm::vec3 Trajectory::get_center_position()
{
	return glm::vec3(
		positions[LENGTH / 2].x,
		heights[LENGTH / 2] + 150,
		positions[LENGTH / 2].z);
}

void Trajectory::update_gait(int vel, float crouched_amount, float extra_gait_smooth)
{

	if (glm::length(target_vel) < 0.1)
	{
		float stand_amount = 1.0f - glm::clamp(glm::length(target_vel) / 0.1f, 0.0f, 1.0f);
		gait_stand[LENGTH / 2] = glm::mix(gait_stand[LENGTH / 2], stand_amount, extra_gait_smooth);
		gait_walk[LENGTH / 2] = glm::mix(gait_walk[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_jog[LENGTH / 2] = glm::mix(gait_jog[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_crouch[LENGTH / 2] = glm::mix(gait_crouch[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_jump[LENGTH / 2] = glm::mix(gait_jump[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_bump[LENGTH / 2] = glm::mix(gait_bump[LENGTH / 2], 0.0f, extra_gait_smooth);
	}
	else if (crouched_amount > 0.1)
	{
		gait_stand[LENGTH / 2] = glm::mix(gait_stand[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_walk[LENGTH / 2] = glm::mix(gait_walk[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_jog[LENGTH / 2] = glm::mix(gait_jog[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_crouch[LENGTH / 2] = glm::mix(gait_crouch[LENGTH / 2], crouched_amount, extra_gait_smooth);
		gait_jump[LENGTH / 2] = glm::mix(gait_jump[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_bump[LENGTH / 2] = glm::mix(gait_bump[LENGTH / 2], 0.0f, extra_gait_smooth);
	}
	else if ((vel / 32768.0) + 1.0)
	{
		gait_stand[LENGTH / 2] = glm::mix(gait_stand[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_walk[LENGTH / 2] = glm::mix(gait_walk[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_jog[LENGTH / 2] = glm::mix(gait_jog[LENGTH / 2], 1.0f, extra_gait_smooth);
		gait_crouch[LENGTH / 2] = glm::mix(gait_crouch[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_jump[LENGTH / 2] = glm::mix(gait_jump[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_bump[LENGTH / 2] = glm::mix(gait_bump[LENGTH / 2], 0.0f, extra_gait_smooth);
	}
	else
	{
		gait_stand[LENGTH / 2] = glm::mix(gait_stand[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_walk[LENGTH / 2] = glm::mix(gait_walk[LENGTH / 2], 1.0f, extra_gait_smooth);
		gait_jog[LENGTH / 2] = glm::mix(gait_jog[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_crouch[LENGTH / 2] = glm::mix(gait_crouch[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_jump[LENGTH / 2] = glm::mix(gait_jump[LENGTH / 2], 0.0f, extra_gait_smooth);
		gait_bump[LENGTH / 2] = glm::mix(gait_bump[LENGTH / 2], 0.0f, extra_gait_smooth);
	}
}

void Trajectory::predict(float responsive, float strafe_amount, Areas *areas)
{
	predict_trajectory(responsive, strafe_amount, areas);
	predict_jump(areas),
		predict_crouch(areas),
		predict_wall(areas);
}

void Trajectory::predict_trajectory(float responsive, float strafe_amount, Areas *areas)
{
	glm::vec3 trajectory_positions_blend[LENGTH];
	trajectory_positions_blend[LENGTH / 2] = positions[LENGTH / 2];

	for (int i = LENGTH / 2 + 1; i < LENGTH; i++)
	{

		float bias_pos = responsive ? glm::mix(2.0f, 2.0f, strafe_amount) : glm::mix(0.5f, 1.0f, strafe_amount);
		float bias_dir = responsive ? glm::mix(5.0f, 3.0f, strafe_amount) : glm::mix(2.0f, 0.5f, strafe_amount);

		float scale_pos = (1.0f - powf(1.0f - ((float)(i - LENGTH / 2) / (LENGTH / 2)), bias_pos));
		float scale_dir = (1.0f - powf(1.0f - ((float)(i - LENGTH / 2) / (LENGTH / 2)), bias_dir));

		trajectory_positions_blend[i] = trajectory_positions_blend[i - 1] + glm::mix(
																				positions[i] - positions[i - 1],
																				target_vel,
																				scale_pos);

		/* Collide with walls */
		for (int j = 0; j < areas->num_walls(); j++)
		{
			glm::vec2 trjpoint = glm::vec2(trajectory_positions_blend[i].x, trajectory_positions_blend[i].z);
			if (glm::length(trjpoint - ((areas->wall_start[j] + areas->wall_stop[j]) / 2.0f)) >
				glm::length(areas->wall_start[j] - areas->wall_stop[j]))
			{
				continue;
			}
			glm::vec2 segpoint = segment_nearest(areas->wall_start[j], areas->wall_stop[j], trjpoint);
			float segdist = glm::length(segpoint - trjpoint);
			if (segdist < areas->wall_width[j] + 100.0)
			{
				glm::vec2 prjpoint0 = (areas->wall_width[j] + 0.0f) * glm::normalize(trjpoint - segpoint) + segpoint;
				glm::vec2 prjpoint1 = (areas->wall_width[j] + 100.0f) * glm::normalize(trjpoint - segpoint) + segpoint;
				glm::vec2 prjpoint = glm::mix(prjpoint0, prjpoint1, glm::clamp((segdist - areas->wall_width[j]) / 100.0f, 0.0f, 1.0f));
				trajectory_positions_blend[i].x = prjpoint.x;
				trajectory_positions_blend[i].z = prjpoint.y;
			}
		}

		directions[i] = mix_directions(directions[i], target_dir, scale_dir);

		heights[i] = heights[LENGTH / 2];
		gait_stand[i] = gait_stand[LENGTH / 2];
		gait_walk[i] = gait_walk[LENGTH / 2];
		gait_jog[i] = gait_jog[LENGTH / 2];
		gait_crouch[i] = gait_crouch[LENGTH / 2];
		gait_jump[i] = gait_jump[LENGTH / 2];
		gait_bump[i] = gait_bump[LENGTH / 2];
	}

	for (int i = LENGTH / 2 + 1; i < LENGTH; i++)
	{
		positions[i] = trajectory_positions_blend[i];
	}
}

void Trajectory::predict_jump(Areas *areas)
{
	for (int i = LENGTH / 2; i < LENGTH; i++)
	{
		gait_jump[i] = 0.0;
		for (int j = 0; j < areas->num_jumps(); j++)
		{
			float dist = glm::length(positions[i] - areas->jump_pos[j]);
			gait_jump[i] = std::max(gait_jump[i], 1.0f - glm::clamp((dist - areas->jump_size[j]) / areas->jump_falloff[j], 0.0f, 1.0f));
		}
	}
}

void Trajectory::predict_crouch(Areas *areas)
{
	for (int i = LENGTH / 2; i < LENGTH; i++)
	{
		for (int j = 0; j < areas->num_crouches(); j++)
		{
			float dist_x = abs(positions[i].x - areas->crouch_pos[j].x);
			float dist_z = abs(positions[i].z - areas->crouch_pos[j].z);
			float height = (sinf(positions[i].x / Areas::CROUCH_WAVE) + 1.0) / 2.0;
			gait_crouch[i] = glm::mix(1.0f - height, gait_crouch[i],
									  glm::clamp(
										  ((dist_x - (areas->crouch_size[j].x / 2)) +
										   (dist_z - (areas->crouch_size[j].y / 2))) /
											  100.0f,
										  0.0f, 1.0f));
		}
	}
}

void Trajectory::predict_wall(Areas *areas)
{
	for (int i = 0; i < LENGTH; i++)
	{
		gait_bump[i] = 0.0;
		for (int j = 0; j < areas->num_walls(); j++)
		{
			glm::vec2 trjpoint = glm::vec2(positions[i].x, positions[i].z);
			glm::vec2 segpoint = segment_nearest(areas->wall_start[j], areas->wall_stop[j], trjpoint);
			float segdist = glm::length(segpoint - trjpoint);
			gait_bump[i] = glm::max(gait_bump[i], 1.0f - glm::clamp((segdist - areas->wall_width[j]) / 10.0f, 0.0f, 1.0f));
		}
	}
}

void Trajectory::input(Heightmap *heightmap, int JOINT_NUM,
					   glm::vec3 *root_position, glm::mat3 *root_rotation,
					   glm::vec3 *joint_positions,
					   glm::vec3 *joint_velocities)
{
	for (int i = 0; i < LENGTH; i++)
	{
		rotations[i] = mat3(rotate(atan2f(
									   directions[i].x,
									   directions[i].z),
								   vec3(0, 1, 0)));
	}

	for (int i = LENGTH / 2; i < LENGTH; i++)
	{
		positions[i].y = heightmap->sample(glm::vec2(positions[i].x, positions[i].z));
	}

	heights[LENGTH / 2] = 0.0;
	for (int i = 0; i < LENGTH; i += 10)
	{
		heights[LENGTH / 2] += (positions[i].y / ((LENGTH) / 10));
	}

	*root_position = glm::vec3(
		positions[LENGTH / 2].x,
		heights[LENGTH / 2],
		positions[LENGTH / 2].z);

	*root_rotation = rotations[LENGTH / 2];

	for (int i = 0; i < LENGTH; i += 10)
	{
		int w = (LENGTH) / 10;
		int o = LENGTH + JOINT_NUM * 3 * 2;

		glm::vec3 pos = glm::inverse(*root_rotation) * (positions[i] - *root_position);
		glm::vec3 dir = glm::inverse(*root_rotation) * directions[i];

		input_position(pos, w, i);
		input_direction(dir, w, i);

		input_gaits(w, i);

		glm::vec3 position_r = positions[i] + (rotations[i] * glm::vec3(width, 0, 0));
		glm::vec3 position_l = positions[i] + (rotations[i] * glm::vec3(-width, 0, 0));

		float R = heightmap->sample(glm::vec2(position_r.x, position_r.z));
		float L = heightmap->sample(glm::vec2(position_l.x, position_l.z));

		input_heights(
			*root_position,
			R, L,
			o, w, i);
	}

	glm::vec3 prev_root_position = glm::vec3(
		positions[LENGTH / 2 - 1].x,
		heights[LENGTH / 2 - 1],
		positions[LENGTH / 2 - 1].z);

	glm::mat3 prev_root_rotation = rotations[LENGTH / 2 - 1];

	for (int i = 0; i < JOINT_NUM; i++)
	{
		glm::vec3 position = glm::inverse(prev_root_rotation) * (joint_positions[i] - prev_root_position);
		glm::vec3 previous = glm::inverse(prev_root_rotation) * joint_velocities[i];
		input_previous_state(position, previous, i, JOINT_NUM);
	}
}

void Trajectory::input_position(vec3 pos, int w, int i)
{
	pfnn->Xp((w * 0) + i / 10) = pos.x;
	pfnn->Xp((w * 1) + i / 10) = pos.z;
}

void Trajectory::input_direction(vec3 dir, int w, int i)
{
	pfnn->Xp((w * 2) + i / 10) = dir.x;
	pfnn->Xp((w * 3) + i / 10) = dir.z;
}

void Trajectory::input_gaits(int w, int i)
{
	pfnn->Xp((w * 4) + i / 10) = gait_stand[i];
	pfnn->Xp((w * 5) + i / 10) = gait_walk[i];
	pfnn->Xp((w * 6) + i / 10) = gait_jog[i];
	pfnn->Xp((w * 7) + i / 10) = gait_crouch[i];
	pfnn->Xp((w * 8) + i / 10) = gait_jump[i];
	pfnn->Xp((w * 9) + i / 10) = 0.0; // Unused.
}

void Trajectory::input_previous_state(vec3 pos, vec3 prv, int i, int JOINT_NUM)
{
	pfnn->Xp(LENGTH + (JOINT_NUM * 3 * 0) + i * 3 + 0) = pos.x;
	pfnn->Xp(LENGTH + (JOINT_NUM * 3 * 0) + i * 3 + 1) = pos.y;
	pfnn->Xp(LENGTH + (JOINT_NUM * 3 * 0) + i * 3 + 2) = pos.z;
	pfnn->Xp(LENGTH + (JOINT_NUM * 3 * 1) + i * 3 + 0) = prv.x;
	pfnn->Xp(LENGTH + (JOINT_NUM * 3 * 1) + i * 3 + 1) = prv.y;
	pfnn->Xp(LENGTH + (JOINT_NUM * 3 * 1) + i * 3 + 2) = prv.z;
}

void Trajectory::input_heights(vec3 root_position, float position_r, float position_l, int o, int w, int i)
{
	pfnn->Xp(o + (w * 0) + (i / 10)) = position_r - root_position.y;
	pfnn->Xp(o + (w * 1) + (i / 10)) = positions[i].y - root_position.y;
	pfnn->Xp(o + (w * 2) + (i / 10)) = position_l - root_position.y;
}

vec3 Trajectory::getPosition(int opos, int i)
{
	return vec3(pfnn->Yp(opos + i * 3 + 0), pfnn->Yp(opos + i * 3 + 1), pfnn->Yp(opos + i * 3 + 2));
}

vec3 Trajectory::getVelocity(int ovel, int i)
{
	return vec3(pfnn->Yp(ovel + i * 3 + 0), pfnn->Yp(ovel + i * 3 + 1), pfnn->Yp(ovel + i * 3 + 2));
}

vec3 Trajectory::getRotation(int orot, int i)
{
	return vec3(pfnn->Yp(orot + i * 3 + 0), pfnn->Yp(orot + i * 3 + 1), pfnn->Yp(orot + i * 3 + 2));
}

void Trajectory::post_update(float *phase, Areas *areas)
{
	update_past();

	float stand_amount = powf(1.0f - gait_stand[LENGTH / 2], 0.25f);

	update_current(stand_amount);

	for (int j = 0; j < areas->num_walls(); j++)
	{
		glm::vec2 trjpoint = glm::vec2(positions[LENGTH / 2].x, positions[LENGTH / 2].z);
		glm::vec2 segpoint = segment_nearest(areas->wall_start[j], areas->wall_stop[j], trjpoint);
		float segdist = glm::length(segpoint - trjpoint);
		if (segdist < areas->wall_width[j] + 100.0)
		{
			glm::vec2 prjpoint0 = (areas->wall_width[j] + 0.0f) * glm::normalize(trjpoint - segpoint) + segpoint;
			glm::vec2 prjpoint1 = (areas->wall_width[j] + 100.0f) * glm::normalize(trjpoint - segpoint) + segpoint;
			glm::vec2 prjpoint = glm::mix(prjpoint0, prjpoint1, glm::clamp((segdist - areas->wall_width[j]) / 100.0f, 0.0f, 1.0f));
			positions[LENGTH / 2].x = prjpoint.x;
			positions[LENGTH / 2].z = prjpoint.y;
		}
	}
	update_future();
	*phase = fmod(*phase + (stand_amount * 0.9f + 0.1f) * 2 * M_PI * pfnn->Yp(3), 2 * M_PI);
}

void Trajectory::update_past()
{
	for (int i = 0; i < LENGTH / 2; i++)
	{
		positions[i] = positions[i + 1];
		directions[i] = directions[i + 1];
		rotations[i] = rotations[i + 1];
		heights[i] = heights[i + 1];
		gait_stand[i] = gait_stand[i + 1];
		gait_walk[i] = gait_walk[i + 1];
		gait_jog[i] = gait_jog[i + 1];
		gait_crouch[i] = gait_crouch[i + 1];
		gait_jump[i] = gait_jump[i + 1];
		gait_bump[i] = gait_bump[i + 1];
	}
}

void Trajectory::update_current(float stand_amount)
{

	glm::vec3 trajectory_update = (rotations[LENGTH / 2] * glm::vec3(pfnn->Yp(0), 0, pfnn->Yp(1)));
	positions[LENGTH / 2] = positions[LENGTH / 2] + stand_amount * trajectory_update;
	directions[LENGTH / 2] = glm::mat3(glm::rotate(stand_amount * -pfnn->Yp(2), glm::vec3(0, 1, 0))) * directions[LENGTH / 2];
	rotations[LENGTH / 2] = glm::mat3(glm::rotate(atan2f(
													  directions[LENGTH / 2].x,
													  directions[LENGTH / 2].z),
												  glm::vec3(0, 1, 0)));
}

void Trajectory::update_future()
{
	for (int i = LENGTH / 2 + 1; i < LENGTH; i++)
	{
		int w = (LENGTH / 2) / 10;
		float m = fmod(((float)i - (LENGTH / 2)) / 10.0, 1.0);
		positions[i].x = (1 - m) * pfnn->Yp(8 + (w * 0) + (i / 10) - w) + m * pfnn->Yp(8 + (w * 0) + (i / 10) - w + 1);
		positions[i].z = (1 - m) * pfnn->Yp(8 + (w * 1) + (i / 10) - w) + m * pfnn->Yp(8 + (w * 1) + (i / 10) - w + 1);
		directions[i].x = (1 - m) * pfnn->Yp(8 + (w * 2) + (i / 10) - w) + m * pfnn->Yp(8 + (w * 2) + (i / 10) - w + 1);
		directions[i].z = (1 - m) * pfnn->Yp(8 + (w * 3) + (i / 10) - w) + m * pfnn->Yp(8 + (w * 3) + (i / 10) - w + 1);
		positions[i] = (rotations[LENGTH / 2] * positions[i]) + positions[LENGTH / 2];
		directions[i] = glm::normalize((rotations[LENGTH / 2] * directions[i]));
		rotations[i] = glm::mat3(glm::rotate(atan2f(directions[i].x, directions[i].z), glm::vec3(0, 1, 0)));
	}
}

void Trajectory::reset(glm::vec3 root_position, glm::mat3 root_rotation)
{
	for (int i = 0; i < LENGTH; i++)
	{
		positions[i] = root_position;
		rotations[i] = root_rotation;
		directions[i] = glm::vec3(0, 0, 1);
		heights[i] = root_position.y;
		gait_stand[i] = 0.0;
		gait_walk[i] = 0.0;
		gait_jog[i] = 0.0;
		gait_crouch[i] = 0.0;
		gait_jump[i] = 0.0;
		gait_bump[i] = 0.0;
	}
}
