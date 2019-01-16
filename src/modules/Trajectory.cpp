#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "PFNN.h"

using namespace glm;

struct Trajectory {

  enum { LENGTH = 120 };
  
  PFNN* pfnn;
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
  
  vec3 target_dir, target_vel;
  
  Trajectory(PFNN* pfnn)
    : width(25)
    , target_dir(vec3(0,0,1))
    , target_vel(vec3(0))
    , pfnn(pfnn) {}

  void rotation() {
    for (int i = 0; i < LENGTH; i++) {
      rotations[i] = mat3(rotate(atan2f(
        directions[i].x,
        directions[i].z), vec3(0,1,0)));
    }
  }

  void input_position(vec3 pos, int w, int i) {
    pfnn->Xp((w*0)+i/10) = pos.x; pfnn->Xp((w*1)+i/10) = pos.z;
  }
  
  void input_direction(vec3 dir, int w, int i) {
    pfnn->Xp((w*2)+i/10) = dir.x; pfnn->Xp((w*3)+i/10) = dir.z;
  }

  void input_gaits(int w, int i) {
    pfnn->Xp((w*4)+i/10) = gait_stand[i];
    pfnn->Xp((w*5)+i/10) = gait_walk[i];
    pfnn->Xp((w*6)+i/10) = gait_jog[i];
    pfnn->Xp((w*7)+i/10) = gait_crouch[i];
    pfnn->Xp((w*8)+i/10) = gait_jump[i];
    pfnn->Xp((w*9)+i/10) = 0.0; // Unused.
  }

  void input_previous_state(vec3 pos, vec3 prv, int i, int JOINT_NUM) {
      pfnn->Xp(LENGTH + (JOINT_NUM*3*0)+i*3+0) = pos.x;
      pfnn->Xp(LENGTH + (JOINT_NUM*3*0)+i*3+1) = pos.y;
      pfnn->Xp(LENGTH + (JOINT_NUM*3*0)+i*3+2) = pos.z;
      pfnn->Xp(LENGTH + (JOINT_NUM*3*1)+i*3+0) = prv.x;
      pfnn->Xp(LENGTH + (JOINT_NUM*3*1)+i*3+1) = prv.y;
      pfnn->Xp(LENGTH + (JOINT_NUM*3*1)+i*3+2) = prv.z;
  }

  void input_heights(vec3 root_position, float position_r, float position_l, int o, int w, int i) {
    pfnn->Xp(o+(w*0)+(i/10)) = position_r - root_position.y;
    pfnn->Xp(o+(w*1)+(i/10)) = positions[i].y - root_position.y;
    pfnn->Xp(o+(w*2)+(i/10)) = position_l - root_position.y;
  }

  vec3 getPosition(int opos, int i) {
    return vec3(pfnn->Yp(opos+i*3+0), pfnn->Yp(opos+i*3+1), pfnn->Yp(opos+i*3+2));
  }

  vec3 getVelocity(int ovel, int i) {
    return vec3(pfnn->Yp(ovel+i*3+0), pfnn->Yp(ovel+i*3+1), pfnn->Yp(ovel+i*3+2));
  }

  vec3 getRotation(int orot, int i) {
    return vec3(pfnn->Yp(orot+i*3+0), pfnn->Yp(orot+i*3+1), pfnn->Yp(orot+i*3+2));
  }

  void predict(int i) {
			heights[i] = heights[LENGTH / 2];
			gait_stand[i] = gait_stand[LENGTH / 2];
			gait_walk[i] = gait_walk[LENGTH / 2];
			gait_jog[i] = gait_jog[LENGTH / 2];
			gait_crouch[i] = gait_crouch[LENGTH / 2];
			gait_jump[i] = gait_jump[LENGTH / 2];
			gait_bump[i] = gait_bump[LENGTH / 2];
  }

  void update_past() {
    for (int i = 0; i < LENGTH/2; i++) {
      positions[i]  = positions[i+1];
      directions[i] = directions[i+1];
      rotations[i] = rotations[i+1];
      heights[i] = heights[i+1];
      gait_stand[i] = gait_stand[i+1];
      gait_walk[i] = gait_walk[i+1];
      gait_jog[i] = gait_jog[i+1];
      gait_crouch[i] = gait_crouch[i+1];
      gait_jump[i] = gait_jump[i+1];
      gait_bump[i] = gait_bump[i+1];
    }
  }

  void update_current(float stand_amount) {
  
    glm::vec3 trajectory_update = (rotations[LENGTH/2] * glm::vec3(pfnn->Yp(0), 0, pfnn->Yp(1)));
    positions[LENGTH/2]  = positions[LENGTH/2] + stand_amount * trajectory_update;
    directions[LENGTH/2] = glm::mat3(glm::rotate(stand_amount * -pfnn->Yp(2), glm::vec3(0,1,0))) * directions[LENGTH/2];
    rotations[LENGTH/2] = glm::mat3(glm::rotate(atan2f(
        directions[LENGTH/2].x,
        directions[LENGTH/2].z), glm::vec3(0,1,0))
    );     
  }

  void update_future() {
    for (int i = LENGTH/2+1; i < LENGTH; i++) {
      int w = (LENGTH/2)/10;
      float m = fmod(((float)i - (LENGTH/2)) / 10.0, 1.0);
      positions[i].x  = (1-m) * pfnn->Yp(8+(w*0)+(i/10)-w) + m * pfnn->Yp(8+(w*0)+(i/10)-w+1);
      positions[i].z  = (1-m) * pfnn->Yp(8+(w*1)+(i/10)-w) + m * pfnn->Yp(8+(w*1)+(i/10)-w+1);
      directions[i].x = (1-m) * pfnn->Yp(8+(w*2)+(i/10)-w) + m * pfnn->Yp(8+(w*2)+(i/10)-w+1);
      directions[i].z = (1-m) * pfnn->Yp(8+(w*3)+(i/10)-w) + m * pfnn->Yp(8+(w*3)+(i/10)-w+1);
      positions[i]    = (rotations[LENGTH/2] * positions[i]) + positions[LENGTH/2];
      directions[i]   = glm::normalize((rotations[LENGTH/2] * directions[i]));
      rotations[i]    = glm::mat3(glm::rotate(atan2f(directions[i].x, directions[i].z), glm::vec3(0,1,0)));
    }
  }

  float get_stand_amount() {
    return powf(1.0f-gait_stand[LENGTH/2], 0.25f);
  }

  void update_gait(int vel, float crouched_amount, float extra_gait_smooth) {

    if (glm::length(target_vel) < 0.1)  {
      float stand_amount = 1.0f - glm::clamp(glm::length(target_vel) / 0.1f, 0.0f, 1.0f);
      gait_stand[LENGTH/2]  = glm::mix(gait_stand[LENGTH/2],  stand_amount, extra_gait_smooth);
      gait_walk[LENGTH/2]   = glm::mix(gait_walk[LENGTH/2],   0.0f, extra_gait_smooth);
      gait_jog[LENGTH/2]    = glm::mix(gait_jog[LENGTH/2],    0.0f, extra_gait_smooth);
      gait_crouch[LENGTH/2] = glm::mix(gait_crouch[LENGTH/2], 0.0f, extra_gait_smooth);
      gait_jump[LENGTH/2]   = glm::mix(gait_jump[LENGTH/2],   0.0f, extra_gait_smooth);
      gait_bump[LENGTH/2]   = glm::mix(gait_bump[LENGTH/2],   0.0f, extra_gait_smooth);
    } else if (crouched_amount > 0.1) {
      gait_stand[LENGTH/2]  = glm::mix(gait_stand[LENGTH/2],  0.0f, extra_gait_smooth);
      gait_walk[LENGTH/2]   = glm::mix(gait_walk[LENGTH/2],   0.0f, extra_gait_smooth);
      gait_jog[LENGTH/2]    = glm::mix(gait_jog[LENGTH/2],    0.0f, extra_gait_smooth);
      gait_crouch[LENGTH/2] = glm::mix(gait_crouch[LENGTH/2], crouched_amount, extra_gait_smooth);
      gait_jump[LENGTH/2]   = glm::mix(gait_jump[LENGTH/2],   0.0f, extra_gait_smooth);
      gait_bump[LENGTH/2]   = glm::mix(gait_bump[LENGTH/2],   0.0f, extra_gait_smooth);
    } else if ((vel / 32768.0) + 1.0) {
      gait_stand[LENGTH/2]  = glm::mix(gait_stand[LENGTH/2],  0.0f, extra_gait_smooth);
      gait_walk[LENGTH/2]   = glm::mix(gait_walk[LENGTH/2],   0.0f, extra_gait_smooth);
      gait_jog[LENGTH/2]    = glm::mix(gait_jog[LENGTH/2],    1.0f, extra_gait_smooth);
      gait_crouch[LENGTH/2] = glm::mix(gait_crouch[LENGTH/2], 0.0f, extra_gait_smooth);
      gait_jump[LENGTH/2]   = glm::mix(gait_jump[LENGTH/2],   0.0f, extra_gait_smooth);    
      gait_bump[LENGTH/2]   = glm::mix(gait_bump[LENGTH/2],   0.0f, extra_gait_smooth);    
    } else {
      gait_stand[LENGTH/2]  = glm::mix(gait_stand[LENGTH/2],  0.0f, extra_gait_smooth);
      gait_walk[LENGTH/2]   = glm::mix(gait_walk[LENGTH/2],   1.0f, extra_gait_smooth);
      gait_jog[LENGTH/2]    = glm::mix(gait_jog[LENGTH/2],    0.0f, extra_gait_smooth);
      gait_crouch[LENGTH/2] = glm::mix(gait_crouch[LENGTH/2], 0.0f, extra_gait_smooth);
      gait_jump[LENGTH/2]   = glm::mix(gait_jump[LENGTH/2],   0.0f, extra_gait_smooth);  
      gait_bump[LENGTH/2]   = glm::mix(gait_bump[LENGTH/2],   0.0f, extra_gait_smooth);  
    }

  }

  void reset(glm::vec3 root_position, glm::mat3 root_rotation) {
    for (int i = 0; i < LENGTH; i++) {
      positions[i] = root_position;
      rotations[i] = root_rotation;
      directions[i] = glm::vec3(0,0,1);
      heights[i] = root_position.y;
      gait_stand[i] = 0.0;
      gait_walk[i] = 0.0;
      gait_jog[i] = 0.0;
      gait_crouch[i] = 0.0;
      gait_jump[i] = 0.0;
      gait_bump[i] = 0.0;
    }
  }
};
