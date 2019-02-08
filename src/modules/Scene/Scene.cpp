#include "scene/Scene.h"

Scene::Scene()
{
	this->shader = new Shader();
	shader->load("./shaders/phong.vert", "./shaders/phong.frag");
	shader->use();

	this->heightmap = new Heightmap("./heightmaps/test_ds.txt", 0.33);
	this->areas = new Areas();

	shader->setUniform("Fog.maxDist", 70.0f);
	shader->setUniform("Fog.minDist", 1.0f);
	shader->setUniform("Fog.color", vec3(0.5f, 0.5f, 0.5f));
}

Scene::~Scene()
{
	delete shader;
	delete heightmap;
	delete areas;
}

void Scene::draw(LightDirectional *light, CameraOrbit *camera)
{
	shader->setUniform("Light.position", camera->view_matrix() * glm::vec4(0.0f, 1.0f, 1.0f, 0.0f));
	shader->setUniform("Light.intensity", vec3(0.8f, 0.8f, 0.8f));

	heightmap->render(shader, camera->view_matrix(), camera->proj_matrix());
}

void Scene::add_character(Character *character)
{
	this->character = character;

	character->reset_position(glm::vec2(0, 0), heightmap, areas);
}
