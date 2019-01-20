#include "Scene.h"

Scene::Scene() {
    this->shader = new Shader();
	shader->load("./shaders/terrain.vs", "./shaders/terrain_low.fs");

	this->heightmap = new Heightmap();
	this->areas = new Areas();
}

Scene::~Scene() {
    delete shader;
	delete heightmap;
	delete areas;
}

void Scene::draw(LightDirectional *light, CameraOrbit *camera) {
	glm::mat4 light_view = glm::lookAt(camera->target + light->position, camera->target, glm::vec3(0, 1, 0));
	glm::mat4 light_proj = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 10.0f, 10000.0f);

	glm::vec3 light_direction = glm::normalize(light->target - light->position);

	glUseProgram(shader->program);

	glUniformMatrix4fv(glGetUniformLocation(shader->program, "view"), 1, GL_FALSE, glm::value_ptr(camera->view_matrix()));
	glUniformMatrix4fv(glGetUniformLocation(shader->program, "proj"), 1, GL_FALSE, glm::value_ptr(camera->proj_matrix()));
	glUniform3f(glGetUniformLocation(shader->program, "light_dir"), light_direction.x, light_direction.y, light_direction.z);

	glUniformMatrix4fv(glGetUniformLocation(shader->program, "light_view"), 1, GL_FALSE, glm::value_ptr(light_view));
	glUniformMatrix4fv(glGetUniformLocation(shader->program, "light_proj"), 1, GL_FALSE, glm::value_ptr(light_proj));

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, light->tex);
	glUniform1i(glGetUniformLocation(shader->program, "shadows"), 0);

	glBindBuffer(GL_ARRAY_BUFFER, heightmap->vbo);

	glEnableVertexAttribArray(glGetAttribLocation(shader->program, "vPosition"));
	glEnableVertexAttribArray(glGetAttribLocation(shader->program, "vNormal"));
	glEnableVertexAttribArray(glGetAttribLocation(shader->program, "vAO"));

	glVertexAttribPointer(glGetAttribLocation(shader->program, "vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 0));
	glVertexAttribPointer(glGetAttribLocation(shader->program, "vNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 3));
	glVertexAttribPointer(glGetAttribLocation(shader->program, "vAO"), 1, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 6));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heightmap->tbo);

	glDrawElements(GL_TRIANGLES, ((heightmap->data.size() - 1) / 2) * ((heightmap->data[0].size() - 1) / 2) * 2 * 3, GL_UNSIGNED_INT, (void *)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(glGetAttribLocation(shader->program, "vPosition"));
	glDisableVertexAttribArray(glGetAttribLocation(shader->program, "vNormal"));
	glDisableVertexAttribArray(glGetAttribLocation(shader->program, "vAO"));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);

}

void Scene::add_character(Character *character) {
    this->character = character;
}

void Scene::load_start_location() {

	heightmap->load("./heightmaps/hmap_013_smooth.txt", 1.0);
	
	areas->clear();
	areas->add_wall(glm::vec2( 1225, -1000), glm::vec2( 1225, 1000), 20);
	areas->add_wall(glm::vec2( 1225,  1000), glm::vec2(-1225, 1000), 20);
	areas->add_wall(glm::vec2(-1225,  1000), glm::vec2(-1225,-1000), 20);
	areas->add_wall(glm::vec2(-1225, -1000), glm::vec2( 1225,-1000), 20);
	
	areas->add_jump(glm::vec3( 237.64, 5,  452.98), 75, 100);
	areas->add_jump(glm::vec3( 378.40, 5,  679.64), 75, 100);
	areas->add_jump(glm::vec3( 227.17, 5,  866.28), 75, 100);
	areas->add_jump(glm::vec3( -43.93, 5,  609.78), 75, 100);
	areas->add_jump(glm::vec3( 810.12, 5,  897.37), 75, 100);
	areas->add_jump(glm::vec3( 945.85, 5,  493.90), 75, 100);
	areas->add_jump(glm::vec3( 618.69, 5,  220.01), 75, 100);
	areas->add_jump(glm::vec3( 950.29, 5,  246.37), 75, 100);
	areas->add_jump(glm::vec3( 703.68, 5, -262.97), 75, 100);
	areas->add_jump(glm::vec3( 798.17, 5, -579.91), 75, 100);
	areas->add_jump(glm::vec3(1137.51, 5, -636.69), 75, 100);
	areas->add_jump(glm::vec3( 212.80, 5, -638.25), 75, 100);
	areas->add_jump(glm::vec3(  79.65, 5, -909.37), 75, 100);
	areas->add_jump(glm::vec3(-286.95, 5, -771.64), 75, 100);
	areas->add_jump(glm::vec3(-994.98, 5, -547.12), 75, 100);
	areas->add_jump(glm::vec3(-384.53, 5,  245.73), 75, 100);
	areas->add_jump(glm::vec3(-559.39, 5,  672.81), 75, 100);
	areas->add_jump(glm::vec3(-701.95, 5,  902.13), 75, 100);

	character->reset_position(glm::vec2(300, 0), heightmap, areas);

}

