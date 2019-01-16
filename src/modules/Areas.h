#ifndef AREAS_H
#define AREAS_H

#include <glm/glm.hpp>
#include <vector>

class Areas {
  public:
  static constexpr float CROUCH_WAVE = 50;

  std::vector<glm::vec3> crouch_pos;
  std::vector<glm::vec2> crouch_size;
  
  std::vector<glm::vec3> jump_pos;
  std::vector<float> jump_size;
  std::vector<float> jump_falloff;
  
  std::vector<glm::vec2> wall_start;
  std::vector<glm::vec2> wall_stop;
  std::vector<float> wall_width;
  
  void clear();
  
  void add_wall(glm::vec2 start, glm::vec2 stop, float width);
  
  void add_crouch(glm::vec3 pos, glm::vec2 size);
  
  void add_jump(glm::vec3 pos, float size, float falloff);
  
  int num_walls();
  int num_crouches();
  int num_jumps();
  
};

#endif /* AREAS_H */
