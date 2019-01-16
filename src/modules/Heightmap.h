#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <GL/glew.h>
// #include <SDL2/SDL.h>
#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

class Heightmap {
  public:
  float hscale;
  float vscale;
  float offset;
  std::vector<std::vector<float>> data;
  GLuint vbo;
  GLuint tbo;
  
  Heightmap();
  
  ~Heightmap();
  
  void load(const char* filename, float multiplier);
  
  float sample(glm::vec2 pos);
  
};

#endif /* HEIGHTMAP_H */
