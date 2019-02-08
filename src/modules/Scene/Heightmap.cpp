#include "scene/Heightmap.h"

void Heightmap::deleteBuffers()
{
	if (buffers.size() > 0)
	{
		glDeleteBuffers((GLsizei)buffers.size(), buffers.data());
		buffers.clear();
	}

	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
}

Heightmap::~Heightmap()
{
	deleteBuffers();
}

Heightmap::Heightmap(const char *filename, float multiplier)
{
	hscale = 3.937007874;
	vscale = 3.0;

	vscale = multiplier * vscale;
	hscale = multiplier * hscale;

	std::ifstream file(filename);

	std::string line;
	while (std::getline(file, line))
	{
		std::vector<float> row;
		std::istringstream iss(line);
		while (iss)
		{
			float f;
			iss >> f;
			row.push_back(f);
		}
		data.push_back(row);
	}

	int w = data.size();
	int h = data[0].size();

	offset = 0.0;
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			offset += data[x][y];
		}
	offset /= w * h;

	printf("Loaded Heightmap '%s' (%i %i)\normals", filename, (int)w, (int)h);

	glm::vec3 *posns = (glm::vec3 *)malloc(sizeof(glm::vec3) * w * h);
	glm::vec3 *norms = (glm::vec3 *)malloc(sizeof(glm::vec3) * w * h);

	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			float cx = hscale * x, cy = hscale * y, cw = hscale * w, ch = hscale * h;
			posns[x + y * w] = glm::vec3(cx - cw / 2, sample(glm::vec2(cx - cw / 2, cy - ch / 2)), cy - ch / 2);
		}

	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			norms[x + y * w] =
				(x > 0 && x < w - 1 && y > 0 && y < h - 1)
					? glm::normalize(glm::mix(
						  glm::cross(
							  posns[(x + 0) + (y + 1) * w] - posns[x + y * w],
							  posns[(x + 1) + (y + 0) * w] - posns[x + y * w]),
						  glm::cross(
							  posns[(x + 0) + (y - 1) * w] - posns[x + y * w],
							  posns[(x - 1) + (y + 0) * w] - posns[x + y * w]),
						  0.5))
					: glm::vec3(0, 1, 0);
		}

	std::vector<GLfloat> points(3 * w * h);
	std::vector<GLfloat> normals(3 * w * h);
	std::vector<GLuint> indices(3 * 2 * ((w - 1) / 2) * ((h - 1) / 2));

	// float *vbo_data = (float *)malloc(sizeof(float) * 3 * w * h);
	// float *normals = (float *)malloc(sizeof(float) * 3 * w * h);
	// uint32_t *tbo_data = (uint32_t *)malloc(sizeof(uint32_t) * 3 * 2 * ((w - 1) / 2) * ((h - 1) / 2));

	int vidx = 0;
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			points[vidx] = posns[x + y * w].x;
			points[vidx + 1] = posns[x + y * w].y;
			points[vidx + 2] = posns[x + y * w].z;
			normals[vidx] = norms[x + y * w].x;
			normals[vidx + 1] = norms[x + y * w].y;
			normals[vidx + 2] = norms[x + y * w].z;

			vidx += 3;
		}

	free(posns);
	free(norms);

	for (int x = 0; x < (w - 1) / 2; x++)
		for (int y = 0; y < (h - 1) / 2; y++)
		{
			indices[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 0] = (x * 2 + 0) + (y * 2 + 0) * w;
			indices[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 1] = (x * 2 + 0) + (y * 2 + 2) * w;
			indices[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 2] = (x * 2 + 2) + (y * 2 + 0) * w;
			indices[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 3] = (x * 2 + 2) + (y * 2 + 2) * w;
			indices[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 4] = (x * 2 + 2) + (y * 2 + 0) * w;
			indices[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 5] = (x * 2 + 0) + (y * 2 + 2) * w;
		}

	initBuffers(&indices, &points, &normals);
}

void Heightmap::initBuffers(
	std::vector<GLuint> *indices,
	std::vector<GLfloat> *points,
	std::vector<GLfloat> *normals)
{
	// Must have data for indices, points, and normals
	if (indices == nullptr || points == nullptr || normals == nullptr)
		return;

	nVerts = (GLuint)indices->size();

	GLuint indexBuf = 0, posBuf = 0, normBuf = 0, tcBuf = 0, tangentBuf = 0;
	glGenBuffers(1, &indexBuf);
	buffers.push_back(indexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), indices->data(), GL_STATIC_DRAW);

	glGenBuffers(1, &posBuf);
	buffers.push_back(posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glBufferData(GL_ARRAY_BUFFER, points->size() * sizeof(GLfloat), points->data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normBuf);
	buffers.push_back(normBuf);
	glBindBuffer(GL_ARRAY_BUFFER, normBuf);
	glBufferData(GL_ARRAY_BUFFER, normals->size() * sizeof(GLfloat), normals->data(), GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	// Position
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0); // Vertex position

	// Normal
	glBindBuffer(GL_ARRAY_BUFFER, normBuf);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1); // Normal

	glBindVertexArray(0);
}

float Heightmap::sample(glm::vec2 pos)
{

	int w = data.size();
	int h = data[0].size();

	pos.x = (pos.x / hscale) + w / 2;
	pos.y = (pos.y / hscale) + h / 2;

	float a0 = fmod(pos.x, 1.0);
	float a1 = fmod(pos.y, 1.0);

	int x0 = (int)std::floor(pos.x), x1 = (int)std::ceil(pos.x);
	int y0 = (int)std::floor(pos.y), y1 = (int)std::ceil(pos.y);

	x0 = x0 < 0 ? 0 : x0;
	x0 = x0 >= w ? w - 1 : x0;
	x1 = x1 < 0 ? 0 : x1;
	x1 = x1 >= w ? w - 1 : x1;
	y0 = y0 < 0 ? 0 : y0;
	y0 = y0 >= h ? h - 1 : y0;
	y1 = y1 < 0 ? 0 : y1;
	y1 = y1 >= h ? h - 1 : y1;

	float s0 = vscale * (data[x0][y0] - offset);
	float s1 = vscale * (data[x1][y0] - offset);
	float s2 = vscale * (data[x0][y1] - offset);
	float s3 = vscale * (data[x1][y1] - offset);

	return (s0 * (1 - a0) + s1 * a0) * (1 - a1) + (s2 * (1 - a0) + s3 * a0) * a1;
}

void Heightmap::render(Shader *shader, glm::mat4 view, glm::mat4 proj)
{
	if (vao == 0)
		return;

	glBindVertexArray(vao);

	shader->setUniform("Kd", 0.2f, 0.2f, 0.2f);
	shader->setUniform("Ks", 0.9f, 0.9f, 0.9f);
	shader->setUniform("Ka", 0.1f, 0.1f, 0.1f);
	shader->setUniform("Shininess", 180.0f);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 mv = view * model;

	shader->setUniform("ModelViewMatrix", mv);
	shader->setUniform("NormalMatrix",
					   glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	shader->setUniform("MVP", proj * mv);

	glDrawElements(GL_TRIANGLES, nVerts, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
