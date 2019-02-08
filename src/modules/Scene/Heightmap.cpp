#include "Heightmap.h"

#include <iostream>

using namespace std;

Heightmap::Heightmap()
{
	hscale = 3.937007874;
	vscale = 3.0;
	offset = 0.0;
	vbo = 0;
	tbo = 0;
}

Heightmap::~Heightmap()
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

void Heightmap::load(const char *filename, float multiplier)
{

	hscale = 3.937007874;
	vscale = 3.0;

	vscale = multiplier * vscale;
	hscale = multiplier * hscale;

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

	data.clear();

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

	printf("Loaded Heightmap '%s' (%i %i)\n", filename, (int)w, (int)h);

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
	cout << "points and normal created"
		 << "\n";

	float *vbo_data = (float *)malloc(sizeof(float) * 7 * w * h);

	uint32_t *tbo_data = (uint32_t *)malloc(sizeof(uint32_t) * 3 * 2 * ((w - 1) / 2) * ((h - 1) / 2));

	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			vbo_data[x * 7 + y * 7 * w + 0] = posns[x + y * w].x;
			vbo_data[x * 7 + y * 7 * w + 1] = posns[x + y * w].y;
			vbo_data[x * 7 + y * 7 * w + 2] = posns[x + y * w].z;
			vbo_data[x * 7 + y * 7 * w + 3] = norms[x + y * w].x;
			vbo_data[x * 7 + y * 7 * w + 4] = norms[x + y * w].y;
			vbo_data[x * 7 + y * 7 * w + 5] = norms[x + y * w].z;
			vbo_data[x * 7 + y * 7 * w + 6] = 0.0f;
		}

	free(posns);
	free(norms);

	for (int x = 0; x < (w - 1) / 2; x++)
		for (int y = 0; y < (h - 1) / 2; y++)
		{
			tbo_data[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 0] = (x * 2 + 0) + (y * 2 + 0) * w;
			tbo_data[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 1] = (x * 2 + 0) + (y * 2 + 2) * w;
			tbo_data[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 2] = (x * 2 + 2) + (y * 2 + 0) * w;
			tbo_data[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 3] = (x * 2 + 2) + (y * 2 + 2) * w;
			tbo_data[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 4] = (x * 2 + 2) + (y * 2 + 0) * w;
			tbo_data[x * 3 * 2 + y * 3 * 2 * ((w - 1) / 2) + 5] = (x * 2 + 0) + (y * 2 + 2) * w;
		}

	cout << "vbo and tbo created"
		 << "\n";

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * w * h, vbo_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 3 * 2 * ((w - 1) / 2) * ((h - 1) / 2), tbo_data, GL_STATIC_DRAW);

	free(vbo_data);
	free(tbo_data);
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
