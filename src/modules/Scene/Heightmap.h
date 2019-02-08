#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <GL/glew.h>
#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

class Heightmap
{
public:
  float hscale;
  float vscale;
  float offset;
  std::vector<std::vector<float>> data;
  GLuint vbo;
  GLuint tbo;

  GLuint vao;

  GLuint getVao() const { return vao; }

  Heightmap();

  ~Heightmap();

  void load(const char *filename, float multiplier);

  void generate(int size, float multiplier);

  float sample(glm::vec2 pos);
};

#endif /* HEIGHTMAP_H */
