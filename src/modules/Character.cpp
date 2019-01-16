#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <fstream>

#include "helpers.cpp"

#include "Trajectory.cpp"
#include "CharacterOptions.cpp"
#include "IK.cpp"

#include "Heightmap.h"
#include "Areas.h"

struct Character
{
	CharacterOptions *options;

	Trajectory *trajectory;
	IK *ik;

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

	Character(Trajectory *trajectory, IK *ik, CharacterOptions *options)
		: vbo(0), tbo(0), ntri(66918), nvtx(11200), phase(0), strafe_amount(0), strafe_target(0), crouched_amount(0), crouched_target(0), responsive(0), trajectory(trajectory), ik(ik), options(options) {}

	~Character()
	{
		if (vbo != 0)
		{
			glDeleteBuffers(1, &vbo);
			vbo = 0;
		}
		if (tbo != 0)
		{
			glDeleteBuffers(1, &tbo);
			tbo = 0;
		}
	}

	glm::vec3 getPosition()
	{
		return glm::vec3(
			trajectory->positions[Trajectory::LENGTH / 2].x,
			trajectory->heights[Trajectory::LENGTH / 2] + 100,
			trajectory->positions[Trajectory::LENGTH / 2].z);
	}

	void reset_position(glm::vec2 position, Heightmap *heightmap)
	{
		ArrayXf Yp = trajectory->pfnn->getYmean();

		glm::vec3 root_position = glm::vec3(position.x, heightmap->sample(position), position.y);
		glm::mat3 root_rotation = glm::mat3();

		trajectory->reset(root_position, root_rotation);

		for (int i = 0; i < JOINT_NUM; i++)
		{
			int opos = 8 + (((Trajectory::LENGTH / 2) / 10) * 4) + (JOINT_NUM * 3 * 0);
			int ovel = 8 + (((Trajectory::LENGTH / 2) / 10) * 4) + (JOINT_NUM * 3 * 1);
			int orot = 8 + (((Trajectory::LENGTH / 2) / 10) * 4) + (JOINT_NUM * 3 * 2);

			glm::vec3 pos = (root_rotation * glm::vec3(Yp(opos + i * 3 + 0), Yp(opos + i * 3 + 1), Yp(opos + i * 3 + 2))) + root_position;
			glm::vec3 vel = (root_rotation * glm::vec3(Yp(ovel + i * 3 + 0), Yp(ovel + i * 3 + 1), Yp(ovel + i * 3 + 2)));
			glm::mat3 rot = (root_rotation * glm::toMat3(quat_exp(glm::vec3(Yp(orot + i * 3 + 0), Yp(orot + i * 3 + 1), Yp(orot + i * 3 + 2)))));

			joint_positions[i] = pos;
			joint_velocities[i] = vel;
			joint_rotations[i] = rot;
		}

		phase = 0.0;

		ik->reset(root_position);
	}

	void load(const char *filename_v, const char *filename_t, const char *filename_p, const char *filename_r)
	{

		printf("Read Character '%s %s'\n", filename_v, filename_t);

		if (vbo != 0)
		{
			glDeleteBuffers(1, &vbo);
			vbo = 0;
		}
		if (tbo != 0)
		{
			glDeleteBuffers(1, &tbo);
			tbo = 0;
		}

		glGenBuffers(1, &vbo);
		glGenBuffers(1, &tbo);

		FILE *f;

		f = fopen(filename_v, "rb");
		float *vbo_data = (float *)malloc(sizeof(float) * 15 * nvtx);
		fread(vbo_data, sizeof(float) * 15 * nvtx, 1, f);
		fclose(f);

		f = fopen(filename_t, "rb");
		uint32_t *tbo_data = (uint32_t *)malloc(sizeof(uint32_t) * ntri);
		fread(tbo_data, sizeof(uint32_t) * ntri, 1, f);
		fclose(f);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 15 * nvtx, vbo_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * ntri, tbo_data, GL_STATIC_DRAW);

		free(vbo_data);
		free(tbo_data);

		f = fopen(filename_p, "rb");
		float fparents[JOINT_NUM];
		fread(fparents, sizeof(float) * JOINT_NUM, 1, f);
		for (int i = 0; i < JOINT_NUM; i++)
		{
			joint_parents[i] = (int)fparents[i];
		}
		fclose(f);

		f = fopen(filename_r, "rb");
		fread(glm::value_ptr(joint_rest_xform[0]), sizeof(float) * JOINT_NUM * 4 * 4, 1, f);
		for (int i = 0; i < JOINT_NUM; i++)
		{
			joint_rest_xform[i] = glm::transpose(joint_rest_xform[i]);
		}
		fclose(f);
	}

	void update(Heightmap *heightmap)
	{
		trajectory->rotation();

		for (int i = Trajectory::LENGTH / 2; i < Trajectory::LENGTH; i++)
		{
			trajectory->positions[i].y = heightmap->sample(glm::vec2(trajectory->positions[i].x, trajectory->positions[i].z));
		}

		trajectory->heights[Trajectory::LENGTH / 2] = 0.0;
		for (int i = 0; i < Trajectory::LENGTH; i += 10)
		{
			trajectory->heights[Trajectory::LENGTH / 2] += (trajectory->positions[i].y / ((Trajectory::LENGTH) / 10));
		}

		root_position = glm::vec3(
			trajectory->positions[Trajectory::LENGTH / 2].x,
			trajectory->heights[Trajectory::LENGTH / 2],
			trajectory->positions[Trajectory::LENGTH / 2].z);

		root_rotation = trajectory->rotations[Trajectory::LENGTH / 2];

		for (int i = 0; i < Trajectory::LENGTH; i += 10)
		{
			int w = (Trajectory::LENGTH) / 10;
			int o = Trajectory::LENGTH + JOINT_NUM * 3 * 2;

			glm::vec3 pos = glm::inverse(root_rotation) * (trajectory->positions[i] - root_position);
			glm::vec3 dir = glm::inverse(root_rotation) * trajectory->directions[i];

			trajectory->input_position(pos, w, i);
			trajectory->input_direction(dir, w, i);

			trajectory->input_gaits(w, i);

			glm::vec3 position_r = trajectory->positions[i] + (trajectory->rotations[i] * glm::vec3(trajectory->width, 0, 0));
			glm::vec3 position_l = trajectory->positions[i] + (trajectory->rotations[i] * glm::vec3(-trajectory->width, 0, 0));

			float R = heightmap->sample(glm::vec2(position_r.x, position_r.z));
			float L = heightmap->sample(glm::vec2(position_l.x, position_l.z));

			trajectory->input_heights(
				root_position,
				R, L,
				o, w, i);
		}

		glm::vec3 prev_root_position = glm::vec3(
			trajectory->positions[Trajectory::LENGTH / 2 - 1].x,
			trajectory->heights[Trajectory::LENGTH / 2 - 1],
			trajectory->positions[Trajectory::LENGTH / 2 - 1].z);

		glm::mat3 prev_root_rotation = trajectory->rotations[Trajectory::LENGTH / 2 - 1];

		for (int i = 0; i < JOINT_NUM; i++)
		{
			glm::vec3 position = glm::inverse(prev_root_rotation) * (joint_positions[i] - prev_root_position);
			glm::vec3 previous = glm::inverse(prev_root_rotation) * joint_velocities[i];
			trajectory->input_previous_state(position, previous, i, JOINT_NUM);
		}
	}

	void post_update_trajectory(Areas *areas)
	{
		trajectory->update_past();

		float stand_amount = trajectory->get_stand_amount();

		trajectory->update_current(stand_amount);

		check_collide(areas);

		trajectory->update_future();
		phase = fmod(phase + (stand_amount * 0.9f + 0.1f) * 2 * M_PI * trajectory->pfnn->Yp(3), 2 * M_PI);
	}

	void build_local_transform()
	{

		for (int i = 0; i < JOINT_NUM; i++)
		{
			int opos = 8 + (((Trajectory::LENGTH / 2) / 10) * 4) + (JOINT_NUM * 3 * 0);
			int ovel = 8 + (((Trajectory::LENGTH / 2) / 10) * 4) + (JOINT_NUM * 3 * 1);
			int orot = 8 + (((Trajectory::LENGTH / 2) / 10) * 4) + (JOINT_NUM * 3 * 2);

			glm::vec3 pos = (root_rotation * trajectory->getPosition(opos, i)) + root_position;
			glm::vec3 vel = (root_rotation * trajectory->getVelocity(ovel, i));
			glm::mat3 rot = (root_rotation * glm::toMat3(quat_exp(trajectory->getRotation(orot, i))));

			joint_positions[i] = glm::mix(joint_positions[i] + vel, pos, options->extra_joint_smooth);
			joint_velocities[i] = vel;
			joint_rotations[i] = rot;

			joint_global_anim_xform[i] = glm::transpose(glm::mat4(
				rot[0][0], rot[1][0], rot[2][0], pos[0],
				rot[0][1], rot[1][1], rot[2][1], pos[1],
				rot[0][2], rot[1][2], rot[2][2], pos[2],
				0, 0, 0, 1));

			if (i == 0)
			{
				joint_anim_xform[i] = joint_global_anim_xform[i];
			}
			else
			{
				joint_anim_xform[i] = glm::inverse(joint_global_anim_xform[joint_parents[i]]) * joint_global_anim_xform[i];
			}
		}

		forward_kinematics();
	}

	void update_move(int x_vel, int y_vel, glm::vec3 cam_direct, int vel, int strafe)
	{
		if (abs(x_vel) + abs(y_vel) < 10000)
		{
			x_vel = 0;
			y_vel = 0;
		};

		glm::vec3 trajectory_target_direction_new = glm::normalize(glm::vec3(cam_direct.x, 0.0, cam_direct.z));
		glm::mat3 trajectory_target_rotation = glm::mat3(glm::rotate(atan2f(
																		 trajectory_target_direction_new.x,
																		 trajectory_target_direction_new.z),
																	 glm::vec3(0, 1, 0)));

		float target_vel_speed = 2.5 + 2.5 * ((vel / 32768.0) + 1.0);

		glm::vec3 trajectory_target_velocity_new = target_vel_speed * (trajectory_target_rotation * glm::vec3(x_vel / 32768.0, 0, y_vel / 32768.0));
		trajectory->target_vel = glm::mix(trajectory->target_vel, trajectory_target_velocity_new, options->extra_velocity_smooth);

		strafe_target = ((strafe / 32768.0) + 1.0) / 2.0;
		strafe_amount = glm::mix(strafe_amount, strafe_target, options->extra_strafe_smooth);

		glm::vec3 trajectory_target_velocity_dir = glm::length(trajectory->target_vel) < 1e-05 ? trajectory->target_dir : glm::normalize(trajectory->target_vel);
		trajectory_target_direction_new = mix_directions(trajectory_target_velocity_dir, trajectory_target_direction_new, strafe_amount);
		trajectory->target_dir = mix_directions(trajectory->target_dir, trajectory_target_direction_new, options->extra_direction_smooth);

		crouched_amount = glm::mix(crouched_amount, crouched_target, options->extra_crouched_smooth);

		trajectory->update_gait(vel, crouched_amount, options->extra_gait_smooth);
	}

	void forecast(Areas *areas)
	{
		forecast_trajectory(areas);
		forecast_jump(areas);
		forecast_crouch(areas);
		forecast_wall(areas);
	}

	void forecast_trajectory(Areas *areas)
	{
		glm::vec3 trajectory_positions_blend[Trajectory::LENGTH];
		trajectory_positions_blend[Trajectory::LENGTH / 2] = trajectory->positions[Trajectory::LENGTH / 2];

		for (int i = Trajectory::LENGTH / 2 + 1; i < Trajectory::LENGTH; i++)
		{

			float bias_pos = responsive ? glm::mix(2.0f, 2.0f, strafe_amount) : glm::mix(0.5f, 1.0f, strafe_amount);
			float bias_dir = responsive ? glm::mix(5.0f, 3.0f, strafe_amount) : glm::mix(2.0f, 0.5f, strafe_amount);

			float scale_pos = (1.0f - powf(1.0f - ((float)(i - Trajectory::LENGTH / 2) / (Trajectory::LENGTH / 2)), bias_pos));
			float scale_dir = (1.0f - powf(1.0f - ((float)(i - Trajectory::LENGTH / 2) / (Trajectory::LENGTH / 2)), bias_dir));

			trajectory_positions_blend[i] = trajectory_positions_blend[i - 1] + glm::mix(
																					trajectory->positions[i] - trajectory->positions[i - 1],
																					trajectory->target_vel,
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

			trajectory->directions[i] = mix_directions(trajectory->directions[i], trajectory->target_dir, scale_dir);

			trajectory->predict(i);
		}

		for (int i = Trajectory::LENGTH / 2 + 1; i < Trajectory::LENGTH; i++)
		{
			trajectory->positions[i] = trajectory_positions_blend[i];
		}
	}

	void forecast_jump(Areas *areas)
	{
		for (int i = Trajectory::LENGTH / 2; i < Trajectory::LENGTH; i++)
		{
			trajectory->gait_jump[i] = 0.0;
			for (int j = 0; j < areas->num_jumps(); j++)
			{
				float dist = glm::length(trajectory->positions[i] - areas->jump_pos[j]);
				trajectory->gait_jump[i] = std::max(trajectory->gait_jump[i],
													1.0f - glm::clamp((dist - areas->jump_size[j]) / areas->jump_falloff[j], 0.0f, 1.0f));
			}
		}
	}

	void forecast_crouch(Areas *areas)
	{
		for (int i = Trajectory::LENGTH / 2; i < Trajectory::LENGTH; i++)
		{
			for (int j = 0; j < areas->num_crouches(); j++)
			{
				float dist_x = abs(trajectory->positions[i].x - areas->crouch_pos[j].x);
				float dist_z = abs(trajectory->positions[i].z - areas->crouch_pos[j].z);
				float height = (sinf(trajectory->positions[i].x / Areas::CROUCH_WAVE) + 1.0) / 2.0;
				trajectory->gait_crouch[i] = glm::mix(1.0f - height, trajectory->gait_crouch[i],
													  glm::clamp(
														  ((dist_x - (areas->crouch_size[j].x / 2)) +
														   (dist_z - (areas->crouch_size[j].y / 2))) /
															  100.0f,
														  0.0f, 1.0f));
			}
		}
	}

	void forecast_wall(Areas *areas)
	{
		for (int i = 0; i < Trajectory::LENGTH; i++)
		{
			trajectory->gait_bump[i] = 0.0;
			for (int j = 0; j < areas->num_walls(); j++)
			{
				glm::vec2 trjpoint = glm::vec2(trajectory->positions[i].x, trajectory->positions[i].z);
				glm::vec2 segpoint = segment_nearest(areas->wall_start[j], areas->wall_stop[j], trjpoint);
				float segdist = glm::length(segpoint - trjpoint);
				trajectory->gait_bump[i] = glm::max(trajectory->gait_bump[i], 1.0f - glm::clamp((segdist - areas->wall_width[j]) / 10.0f, 0.0f, 1.0f));
			}
		}
	}

	void predict_pfnn()
	{
		trajectory->pfnn->predict(phase);
	}

	void forward_kinematics()
	{

		for (int i = 0; i < JOINT_NUM; i++)
		{
			joint_global_anim_xform[i] = joint_anim_xform[i];
			joint_global_rest_xform[i] = joint_rest_xform[i];
			int j = joint_parents[i];
			while (j != -1)
			{
				joint_global_anim_xform[i] = joint_anim_xform[j] * joint_global_anim_xform[i];
				joint_global_rest_xform[i] = joint_rest_xform[j] * joint_global_rest_xform[i];
				j = joint_parents[j];
			}
			joint_mesh_xform[i] = joint_global_anim_xform[i] * glm::inverse(joint_global_rest_xform[i]);
		}
	}

	void set_ik(Heightmap *heightmap)
	{

		glm::vec4 ik_weight = glm::vec4(
			trajectory->pfnn->Yp(4 + 0),
			trajectory->pfnn->Yp(4 + 1),
			trajectory->pfnn->Yp(4 + 2),
			trajectory->pfnn->Yp(4 + 3));

		glm::vec3 key_hl = glm::vec3(joint_global_anim_xform[JOINT_HEEL_L][3]);
		glm::vec3 key_tl = glm::vec3(joint_global_anim_xform[JOINT_TOE_L][3]);
		glm::vec3 key_hr = glm::vec3(joint_global_anim_xform[JOINT_HEEL_R][3]);
		glm::vec3 key_tr = glm::vec3(joint_global_anim_xform[JOINT_TOE_R][3]);

		key_hl = glm::mix(key_hl, ik->position[IK::HL], ik->lock[IK::HL]);
		key_tl = glm::mix(key_tl, ik->position[IK::TL], ik->lock[IK::TL]);
		key_hr = glm::mix(key_hr, ik->position[IK::HR], ik->lock[IK::HR]);
		key_tr = glm::mix(key_tr, ik->position[IK::TR], ik->lock[IK::TR]);

		ik->height[IK::HL] = glm::mix(ik->height[IK::HL], heightmap->sample(glm::vec2(key_hl.x, key_hl.z)) + ik->heel_height, ik->smoothness);
		ik->height[IK::TL] = glm::mix(ik->height[IK::TL], heightmap->sample(glm::vec2(key_tl.x, key_tl.z)) + ik->toe_height, ik->smoothness);
		ik->height[IK::HR] = glm::mix(ik->height[IK::HR], heightmap->sample(glm::vec2(key_hr.x, key_hr.z)) + ik->heel_height, ik->smoothness);
		ik->height[IK::TR] = glm::mix(ik->height[IK::TR], heightmap->sample(glm::vec2(key_tr.x, key_tr.z)) + ik->toe_height, ik->smoothness);

		key_hl.y = glm::max(key_hl.y, ik->height[IK::HL]);
		key_tl.y = glm::max(key_tl.y, ik->height[IK::TL]);
		key_hr.y = glm::max(key_hr.y, ik->height[IK::HR]);
		key_tr.y = glm::max(key_tr.y, ik->height[IK::TR]);

		rotate_hip_knee(key_hl, key_hr);
		rotate_heel(heightmap, ik_weight, key_tl, key_tr);
		rotate_toe(heightmap, ik_weight);
		update_locks(ik_weight);
	}

	void rotate_hip_knee(glm::vec3 key_hl, glm::vec3 key_hr)
	{
		glm::vec3 hip_l = glm::vec3(joint_global_anim_xform[JOINT_HIP_L][3]);
		glm::vec3 knee_l = glm::vec3(joint_global_anim_xform[JOINT_KNEE_L][3]);
		glm::vec3 heel_l = glm::vec3(joint_global_anim_xform[JOINT_HEEL_L][3]);

		glm::vec3 hip_r = glm::vec3(joint_global_anim_xform[JOINT_HIP_R][3]);
		glm::vec3 knee_r = glm::vec3(joint_global_anim_xform[JOINT_KNEE_R][3]);
		glm::vec3 heel_r = glm::vec3(joint_global_anim_xform[JOINT_HEEL_R][3]);

		ik->two_joint(hip_l, knee_l, heel_l, key_hl, 1.0,
					  joint_global_anim_xform[JOINT_ROOT_L],
					  joint_global_anim_xform[JOINT_HIP_L],
					  joint_global_anim_xform[JOINT_HIP_L],
					  joint_global_anim_xform[JOINT_KNEE_L],
					  joint_anim_xform[JOINT_HIP_L],
					  joint_anim_xform[JOINT_KNEE_L]);

		ik->two_joint(hip_r, knee_r, heel_r, key_hr, 1.0,
					  joint_global_anim_xform[JOINT_ROOT_R],
					  joint_global_anim_xform[JOINT_HIP_R],
					  joint_global_anim_xform[JOINT_HIP_R],
					  joint_global_anim_xform[JOINT_KNEE_R],
					  joint_anim_xform[JOINT_HIP_R],
					  joint_anim_xform[JOINT_KNEE_R]);

		forward_kinematics();
	}

	void rotate_heel(Heightmap *heightmap, glm::vec4 ik_weight, glm::vec3 key_tl, glm::vec3 key_tr)
	{
		const float heel_max_bend_s = 4;
		const float heel_max_bend_u = 4;
		const float heel_max_bend_d = 4;

		glm::vec4 ik_toe_pos_blend = glm::clamp(ik_weight * 2.5f, 0.0f, 1.0f);

		glm::vec3 heel_l = glm::vec3(joint_global_anim_xform[JOINT_HEEL_L][3]);
		glm::vec4 side_h0_l = joint_global_anim_xform[JOINT_HEEL_L] * glm::vec4(10, 0, 0, 1);
		glm::vec4 side_h1_l = joint_global_anim_xform[JOINT_HEEL_L] * glm::vec4(-10, 0, 0, 1);
		glm::vec3 side0_l = glm::vec3(side_h0_l) / side_h0_l.w;
		glm::vec3 side1_l = glm::vec3(side_h1_l) / side_h1_l.w;
		glm::vec3 floor_l = key_tl;

		side0_l.y = glm::clamp(heightmap->sample(glm::vec2(side0_l.x, side0_l.z)) + ik->toe_height, heel_l.y - heel_max_bend_s, heel_l.y + heel_max_bend_s);
		side1_l.y = glm::clamp(heightmap->sample(glm::vec2(side1_l.x, side1_l.z)) + ik->toe_height, heel_l.y - heel_max_bend_s, heel_l.y + heel_max_bend_s);
		floor_l.y = glm::clamp(floor_l.y, heel_l.y - heel_max_bend_d, heel_l.y + heel_max_bend_u);

		glm::vec3 targ_z_l = glm::normalize(floor_l - heel_l);
		glm::vec3 targ_x_l = glm::normalize(side0_l - side1_l);
		glm::vec3 targ_y_l = glm::normalize(glm::cross(targ_x_l, targ_z_l));
		targ_x_l = glm::cross(targ_z_l, targ_y_l);

		joint_anim_xform[JOINT_HEEL_L] = mix_transforms(
			joint_anim_xform[JOINT_HEEL_L],
			glm::inverse(joint_global_anim_xform[JOINT_KNEE_L]) * glm::mat4(
																	  glm::vec4(targ_x_l, 0),
																	  glm::vec4(-targ_y_l, 0),
																	  glm::vec4(targ_z_l, 0),
																	  glm::vec4(heel_l, 1)),
			ik_toe_pos_blend.y);

		glm::vec3 heel_r = glm::vec3(joint_global_anim_xform[JOINT_HEEL_R][3]);
		glm::vec4 side_h0_r = joint_global_anim_xform[JOINT_HEEL_R] * glm::vec4(10, 0, 0, 1);
		glm::vec4 side_h1_r = joint_global_anim_xform[JOINT_HEEL_R] * glm::vec4(-10, 0, 0, 1);
		glm::vec3 side0_r = glm::vec3(side_h0_r) / side_h0_r.w;
		glm::vec3 side1_r = glm::vec3(side_h1_r) / side_h1_r.w;
		glm::vec3 floor_r = key_tr;

		side0_r.y = glm::clamp(heightmap->sample(glm::vec2(side0_r.x, side0_r.z)) + ik->toe_height, heel_r.y - heel_max_bend_s, heel_r.y + heel_max_bend_s);
		side1_r.y = glm::clamp(heightmap->sample(glm::vec2(side1_r.x, side1_r.z)) + ik->toe_height, heel_r.y - heel_max_bend_s, heel_r.y + heel_max_bend_s);
		floor_r.y = glm::clamp(floor_r.y, heel_r.y - heel_max_bend_d, heel_r.y + heel_max_bend_u);

		glm::vec3 targ_z_r = glm::normalize(floor_r - heel_r);
		glm::vec3 targ_x_r = glm::normalize(side0_r - side1_r);
		glm::vec3 targ_y_r = glm::normalize(glm::cross(targ_z_r, targ_x_r));
		targ_x_r = glm::cross(targ_z_r, targ_y_r);

		joint_anim_xform[JOINT_HEEL_R] = mix_transforms(
			joint_anim_xform[JOINT_HEEL_R],
			glm::inverse(joint_global_anim_xform[JOINT_KNEE_R]) * glm::mat4(
																	  glm::vec4(-targ_x_r, 0),
																	  glm::vec4(targ_y_r, 0),
																	  glm::vec4(targ_z_r, 0),
																	  glm::vec4(heel_r, 1)),
			ik_toe_pos_blend.w);

		forward_kinematics();
	}

	void rotate_toe(Heightmap *heightmap, glm::vec4 ik_weight)
	{
		const float toe_max_bend_d = 0;
		const float toe_max_bend_u = 10;

		glm::vec4 ik_toe_rot_blend = glm::clamp(ik_weight * 2.5f, 0.0f, 1.0f);

		glm::vec3 toe_l = glm::vec3(joint_global_anim_xform[JOINT_TOE_L][3]);
		glm::vec4 fwrd_h_l = joint_global_anim_xform[JOINT_TOE_L] * glm::vec4(0, 0, 10, 1);
		glm::vec4 side_h0_l = joint_global_anim_xform[JOINT_TOE_L] * glm::vec4(10, 0, 0, 1);
		glm::vec4 side_h1_l = joint_global_anim_xform[JOINT_TOE_L] * glm::vec4(-10, 0, 0, 1);
		glm::vec3 fwrd_l = glm::vec3(fwrd_h_l) / fwrd_h_l.w;
		glm::vec3 side0_l = glm::vec3(side_h0_l) / side_h0_l.w;
		glm::vec3 side1_l = glm::vec3(side_h1_l) / side_h1_l.w;

		fwrd_l.y = glm::clamp(heightmap->sample(glm::vec2(fwrd_l.x, fwrd_l.z)) + ik->toe_height, toe_l.y - toe_max_bend_d, toe_l.y + toe_max_bend_u);
		side0_l.y = glm::clamp(heightmap->sample(glm::vec2(side0_l.x, side0_l.z)) + ik->toe_height, toe_l.y - toe_max_bend_d, toe_l.y + toe_max_bend_u);
		side1_l.y = glm::clamp(heightmap->sample(glm::vec2(side0_l.x, side1_l.z)) + ik->toe_height, toe_l.y - toe_max_bend_d, toe_l.y + toe_max_bend_u);

		glm::vec3 side_l = glm::normalize(side0_l - side1_l);
		fwrd_l = glm::normalize(fwrd_l - toe_l);
		glm::vec3 upwr_l = glm::normalize(glm::cross(side_l, fwrd_l));
		side_l = glm::cross(fwrd_l, upwr_l);

		joint_anim_xform[JOINT_TOE_L] = mix_transforms(
			joint_anim_xform[JOINT_TOE_L],
			glm::inverse(joint_global_anim_xform[JOINT_HEEL_L]) * glm::mat4(
																	  glm::vec4(side_l, 0),
																	  glm::vec4(-upwr_l, 0),
																	  glm::vec4(fwrd_l, 0),
																	  glm::vec4(toe_l, 1)),
			ik_toe_rot_blend.y);

		glm::vec3 toe_r = glm::vec3(joint_global_anim_xform[JOINT_TOE_R][3]);
		glm::vec4 fwrd_h_r = joint_global_anim_xform[JOINT_TOE_R] * glm::vec4(0, 0, 10, 1);
		glm::vec4 side_h0_r = joint_global_anim_xform[JOINT_TOE_R] * glm::vec4(10, 0, 0, 1);
		glm::vec4 side_h1_r = joint_global_anim_xform[JOINT_TOE_R] * glm::vec4(-10, 0, 0, 1);
		glm::vec3 fwrd_r = glm::vec3(fwrd_h_r) / fwrd_h_r.w;
		glm::vec3 side0_r = glm::vec3(side_h0_r) / side_h0_r.w;
		glm::vec3 side1_r = glm::vec3(side_h1_r) / side_h1_r.w;

		fwrd_r.y = glm::clamp(heightmap->sample(glm::vec2(fwrd_r.x, fwrd_r.z)) + ik->toe_height, toe_r.y - toe_max_bend_d, toe_r.y + toe_max_bend_u);
		side0_r.y = glm::clamp(heightmap->sample(glm::vec2(side0_r.x, side0_r.z)) + ik->toe_height, toe_r.y - toe_max_bend_d, toe_r.y + toe_max_bend_u);
		side1_r.y = glm::clamp(heightmap->sample(glm::vec2(side1_r.x, side1_r.z)) + ik->toe_height, toe_r.y - toe_max_bend_d, toe_r.y + toe_max_bend_u);

		glm::vec3 side_r = glm::normalize(side0_r - side1_r);
		fwrd_r = glm::normalize(fwrd_r - toe_r);
		glm::vec3 upwr_r = glm::normalize(glm::cross(side_r, fwrd_r));
		side_r = glm::cross(fwrd_r, upwr_r);

		joint_anim_xform[JOINT_TOE_R] = mix_transforms(
			joint_anim_xform[JOINT_TOE_R],
			glm::inverse(joint_global_anim_xform[JOINT_HEEL_R]) * glm::mat4(
																	  glm::vec4(side_r, 0),
																	  glm::vec4(-upwr_r, 0),
																	  glm::vec4(fwrd_r, 0),
																	  glm::vec4(toe_r, 1)),
			ik_toe_rot_blend.w);

		forward_kinematics();
	}

	void update_locks(glm::vec4 ik_weight)
	{
		if ((ik->lock[IK::HL] == 0.0) && (ik_weight.y >= ik->threshold))
		{
			ik->lock[IK::HL] = 1.0;
			ik->position[IK::HL] = glm::vec3(joint_global_anim_xform[JOINT_HEEL_L][3]);
			ik->lock[IK::TL] = 1.0;
			ik->position[IK::TL] = glm::vec3(joint_global_anim_xform[JOINT_TOE_L][3]);
		}

		if ((ik->lock[IK::HR] == 0.0) && (ik_weight.w >= ik->threshold))
		{
			ik->lock[IK::HR] = 1.0;
			ik->position[IK::HR] = glm::vec3(joint_global_anim_xform[JOINT_HEEL_R][3]);
			ik->lock[IK::TR] = 1.0;
			ik->position[IK::TR] = glm::vec3(joint_global_anim_xform[JOINT_TOE_R][3]);
		}

		if ((ik->lock[IK::HL] > 0.0) && (ik_weight.y < ik->threshold))
		{
			ik->lock[IK::HL] = glm::clamp(ik->lock[IK::HL] - ik->fade, 0.0f, 1.0f);
			ik->lock[IK::TL] = glm::clamp(ik->lock[IK::TL] - ik->fade, 0.0f, 1.0f);
		}

		if ((ik->lock[IK::HR] > 0.0) && (ik_weight.w < ik->threshold))
		{
			ik->lock[IK::HR] = glm::clamp(ik->lock[IK::HR] - ik->fade, 0.0f, 1.0f);
			ik->lock[IK::TR] = glm::clamp(ik->lock[IK::TR] - ik->fade, 0.0f, 1.0f);
		}
	}

	void check_collide(Areas *areas)
	{
		for (int j = 0; j < areas->num_walls(); j++)
		{
			glm::vec2 trjpoint = glm::vec2(trajectory->positions[Trajectory::LENGTH / 2].x, trajectory->positions[Trajectory::LENGTH / 2].z);
			glm::vec2 segpoint = segment_nearest(areas->wall_start[j], areas->wall_stop[j], trjpoint);
			float segdist = glm::length(segpoint - trjpoint);
			if (segdist < areas->wall_width[j] + 100.0)
			{
				glm::vec2 prjpoint0 = (areas->wall_width[j] + 0.0f) * glm::normalize(trjpoint - segpoint) + segpoint;
				glm::vec2 prjpoint1 = (areas->wall_width[j] + 100.0f) * glm::normalize(trjpoint - segpoint) + segpoint;
				glm::vec2 prjpoint = glm::mix(prjpoint0, prjpoint1, glm::clamp((segdist - areas->wall_width[j]) / 100.0f, 0.0f, 1.0f));
				trajectory->positions[Trajectory::LENGTH / 2].x = prjpoint.x;
				trajectory->positions[Trajectory::LENGTH / 2].z = prjpoint.y;
			}
		}
	}
};
