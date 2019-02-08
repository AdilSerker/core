#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <GL/glew.h>
#include <eigen3/Eigen/Dense>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>

#include "Shader.h"

class Heightmap
{
  protected:
	GLuint nVerts;
	GLuint vao;

	std::vector<GLuint> buffers;
	void initBuffers(
		std::vector<GLuint> *indices,
		std::vector<GLfloat> *points,
		std::vector<GLfloat> *normals);
	void deleteBuffers();

	float hscale;
	float vscale;
	float offset;
	std::vector<std::vector<float>> data;

  public:
	Heightmap(const char *filename, float multiplier);
	~Heightmap();
	void render(Shader *shader, glm::mat4 view, glm::mat4 proj);

	float sample(glm::vec2 pos);
};

#endif /* HEIGHTMAP_H */
