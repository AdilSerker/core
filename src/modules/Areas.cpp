#include <glm/glm.hpp>
#include <vector>

#include "Areas.h"

void Areas::clear() {
  crouch_pos.clear();
  crouch_size.clear();
  jump_pos.clear();
  jump_size.clear();
  jump_falloff.clear();
  wall_start.clear();
  wall_stop.clear();
  wall_width.clear();
}
  
void Areas::add_wall(glm::vec2 start, glm::vec2 stop, float width) {
  wall_start.push_back(start);
  wall_stop.push_back(stop);
  wall_width.push_back(width);
}
  
void Areas::add_crouch(glm::vec3 pos, glm::vec2 size) {
  crouch_pos.push_back(pos);
  crouch_size.push_back(size);
}

void Areas::add_jump(glm::vec3 pos, float size, float falloff) {
  jump_pos.push_back(pos);
  jump_size.push_back(size);
  jump_falloff.push_back(falloff);
}
  
int Areas::num_walls() { return wall_start.size(); }
int Areas::num_crouches() { return crouch_pos.size(); }
int Areas::num_jumps() { return jump_pos.size(); }
