#include "Scene.h"

#include <iostream>

using namespace std;

Scene::Scene()
{
	this->shader = new Shader();
	shader->load("./shaders/phong.vert", "./shaders/phong.frag");

	this->heightmap = new Heightmap();
	this->areas = new Areas();

	glUniform1f(glGetUniformLocation(shader->program, "Fog.maxDist"), 70.0f);
	glUniform1f(glGetUniformLocation(shader->program, "Fog.minDist"), 1.0f);
	glUniform3f(glGetUniformLocation(shader->program, "Fog.color"), 0.5f, 0.5f, 0.5f);
}

Scene::~Scene()
{
	delete shader;
	delete heightmap;
	delete areas;
}

void Scene::draw(LightDirectional *light, CameraOrbit *camera)
{
	glUseProgram(shader->program);

	glm::mat4 proj = camera->proj_matrix();
	glm::mat4 view = camera->view_matrix();

	glUniformMatrix4fv(glGetUniformLocation(shader->program, "Light.position"), 1, GL_FALSE, glm::value_ptr(view * glm::vec4(0.0f, 100.0f, 100.0f, 0.0f)));
	glUniform3f(glGetUniformLocation(shader->program, "Light.intensity"), 0.8f, 0.8f, 0.8f);

	glUniform3f(glGetUniformLocation(shader->program, "Kd"), 0.8f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shader->program, "Ks"), 0.9f, 0.9f, 0.9f);
	glUniform3f(glGetUniformLocation(shader->program, "Ka"), 0.1f, 0.1f, 0.1f);
	glUniform1f(glGetUniformLocation(shader->program, "Shininess"), 180.0f);

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 mv = view * model;

	glUniformMatrix4fv(glGetUniformLocation(shader->program, "ModelViewMatrix"), 1, GL_FALSE, glm::value_ptr(mv));
	glUniformMatrix3fv(glGetUniformLocation(shader->program, "NormalMatrix"),
					   1, GL_FALSE, glm::value_ptr(glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]))));
	glUniformMatrix4fv(glGetUniformLocation(shader->program, "MVP"), 1, GL_FALSE, glm::value_ptr(proj * mv));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heightmap->tbo);
	// Position
	glBindBuffer(GL_ARRAY_BUFFER, heightmap->vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 0));
	glEnableVertexAttribArray(0); // Vertex position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glDrawElements(GL_TRIANGLES, ((heightmap->data.size() - 1) / 2) * ((heightmap->data[0].size() - 1) / 2) * 2 * 3, GL_UNSIGNED_INT, (void *)0);

	glUseProgram(0);
}

void Scene::add_character(Character *character)
{
	this->character = character;
}

void Scene::load_start_location()
{
	heightmap->load("./heightmaps/hmap_013_smooth.txt", 1.0);
	cout << "loaded hm"
		 << "\n";

	areas->clear();
	areas->add_wall(glm::vec2(1225, -1000), glm::vec2(1225, 1000), 20);
	areas->add_wall(glm::vec2(1225, 1000), glm::vec2(-1225, 1000), 20);
	areas->add_wall(glm::vec2(-1225, 1000), glm::vec2(-1225, -1000), 20);
	areas->add_wall(glm::vec2(-1225, -1000), glm::vec2(1225, -1000), 20);

	areas->add_jump(glm::vec3(237.64, 5, 452.98), 75, 100);
	areas->add_jump(glm::vec3(378.40, 5, 679.64), 75, 100);
	areas->add_jump(glm::vec3(227.17, 5, 866.28), 75, 100);
	areas->add_jump(glm::vec3(-43.93, 5, 609.78), 75, 100);
	areas->add_jump(glm::vec3(810.12, 5, 897.37), 75, 100);
	areas->add_jump(glm::vec3(945.85, 5, 493.90), 75, 100);
	areas->add_jump(glm::vec3(618.69, 5, 220.01), 75, 100);
	areas->add_jump(glm::vec3(950.29, 5, 246.37), 75, 100);
	areas->add_jump(glm::vec3(703.68, 5, -262.97), 75, 100);
	areas->add_jump(glm::vec3(798.17, 5, -579.91), 75, 100);
	areas->add_jump(glm::vec3(1137.51, 5, -636.69), 75, 100);
	areas->add_jump(glm::vec3(212.80, 5, -638.25), 75, 100);
	areas->add_jump(glm::vec3(79.65, 5, -909.37), 75, 100);
	areas->add_jump(glm::vec3(-286.95, 5, -771.64), 75, 100);
	areas->add_jump(glm::vec3(-994.98, 5, -547.12), 75, 100);
	areas->add_jump(glm::vec3(-384.53, 5, 245.73), 75, 100);
	areas->add_jump(glm::vec3(-559.39, 5, 672.81), 75, 100);
	areas->add_jump(glm::vec3(-701.95, 5, 902.13), 75, 100);

	character->reset_position(glm::vec2(300, 0), heightmap, areas);
}

void Scene::load_location1()
{

	heightmap->load("./heightmaps/hmap_007_smooth.txt", 1.0);

	areas->clear();
	areas->add_wall(glm::vec2(1137.99, -2583.42), glm::vec2(1154.53, 2604.02), 20);
	areas->add_wall(glm::vec2(1154.53, 2604.02), glm::vec2(644.10, 2602.73), 20);
	areas->add_wall(glm::vec2(644.10, 2602.73), glm::vec2(504.73, 2501.38), 20);
	areas->add_wall(glm::vec2(504.73, 2501.38), glm::vec2(12.73, 2522.49), 20);
	areas->add_wall(glm::vec2(12.73, 2522.49), glm::vec2(-84.41, 2497.15), 20);
	areas->add_wall(glm::vec2(-84.41, 2497.15), glm::vec2(-342.03, 2481.34), 20);
	areas->add_wall(glm::vec2(-342.03, 2481.34), glm::vec2(-436.74, 2453.81), 20);
	areas->add_wall(glm::vec2(-436.74, 2453.81), glm::vec2(-555.85, 2480.54), 20);
	areas->add_wall(glm::vec2(-555.85, 2480.54), glm::vec2(-776.98, 2500.82), 20);
	areas->add_wall(glm::vec2(-776.98, 2500.82), glm::vec2(-877.50, 2466.82), 20);
	areas->add_wall(glm::vec2(-877.50, 2466.82), glm::vec2(-975.67, 2488.11), 20);
	areas->add_wall(glm::vec2(-975.67, 2488.11), glm::vec2(-995.97, 2607.62), 20);
	areas->add_wall(glm::vec2(-995.97, 2607.62), glm::vec2(-1142.54, 2612.13), 20);
	areas->add_wall(glm::vec2(-1142.54, 2612.13), glm::vec2(-1151.56, 2003.29), 20);
	areas->add_wall(glm::vec2(-1151.56, 2003.29), glm::vec2(-1133.52, 1953.68), 20);
	areas->add_wall(glm::vec2(-1133.52, 1953.68), glm::vec2(-1153.82, 1888.29), 20);
	areas->add_wall(glm::vec2(-1153.82, 1888.29), glm::vec2(-1151.56, -2608.12), 20);
	areas->add_wall(glm::vec2(-1151.56, -2608.12), glm::vec2(-1126.76, -2608.12), 20);
	areas->add_wall(glm::vec2(-1126.76, -2608.12), glm::vec2(-1133.52, -427.57), 20);
	areas->add_wall(glm::vec2(-1133.52, -427.57), glm::vec2(-1074.89, -184.03), 20);
	areas->add_wall(glm::vec2(-1074.89, -184.03), glm::vec2(-973.42, 48.23), 20);
	areas->add_wall(glm::vec2(-973.42, 48.23), glm::vec2(-928.32, 217.35), 20);
	areas->add_wall(glm::vec2(-928.32, 217.35), glm::vec2(-732.14, 535.30), 20);
	areas->add_wall(glm::vec2(-732.14, 535.30), glm::vec2(-734.39, 436.09), 20);
	areas->add_wall(glm::vec2(-734.39, 436.09), glm::vec2(-838.12, 167.75), 20);
	areas->add_wall(glm::vec2(-838.12, 167.75), glm::vec2(-937.34, -427.57), 20);
	areas->add_wall(glm::vec2(-937.34, -427.57), glm::vec2(-930.57, -1164.94), 20);
	areas->add_wall(glm::vec2(-930.57, -1164.94), glm::vec2(-844.88, -1478.38), 20);
	areas->add_wall(glm::vec2(-844.88, -1478.38), glm::vec2(-691.55, -2166.15), 20);
	areas->add_wall(glm::vec2(-691.55, -2166.15), glm::vec2(-648.70, -2610.37), 20);
	areas->add_wall(glm::vec2(-648.70, -2610.37), glm::vec2(1139.49, -2581.06), 20);
	areas->add_wall(glm::vec2(-314.97, -2472.82), glm::vec2(-258.59, -2508.90), 20);
	areas->add_wall(glm::vec2(-258.59, -2508.90), glm::vec2(-195.45, -2504.39), 20);
	areas->add_wall(glm::vec2(-195.45, -2504.39), glm::vec2(-199.96, -2477.33), 20);
	areas->add_wall(glm::vec2(-199.96, -2477.33), glm::vec2(-238.30, -2450.27), 20);
	areas->add_wall(glm::vec2(-238.30, -2450.27), glm::vec2(-281.14, -2441.25), 20);
	areas->add_wall(glm::vec2(-281.14, -2441.25), glm::vec2(-310.46, -2466.06), 20);

	character->reset_position(glm::vec2(300, 0), heightmap, areas);
}

void Scene::load_test_location()
{

	heightmap->load("./heightmaps/test_ds.txt", 0.33);

	areas->clear();

	character->reset_position(glm::vec2(0, 0), heightmap, areas);
}
