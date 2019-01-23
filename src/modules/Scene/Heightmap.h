#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <GL/glew.h>
#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

class Heightmap {
  public:
  bool loading;

  float hscale;
  float vscale;
  float offset;
  std::vector<std::vector<float>> data;

  int range;

  GLuint vbo;
  GLuint tbo;
  
  Heightmap();
  
  ~Heightmap();
  
  void load(const char* filename, float multiplier);

  void generate(int size, float multiplier);
  
  float sample(glm::vec2 pos);

  private: 
  void diamondSquare(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int range, unsigned level);

  bool initialized;
  
};

#endif /* HEIGHTMAP_H */
