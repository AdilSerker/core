// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include <chrono>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;
using namespace std::chrono;

#include "modules/CameraOrbit.h"
#include "modules/LightDirectional.h"
#include "modules/Heightmap.h"
#include "modules/Shader.h"
#include "modules/Areas.h"
#include "modules/Character.cpp"

static LightDirectional *light = NULL;

/* Heightmap */

static Heightmap *heightmap = NULL;
static Areas *areas = NULL;

/* Shader */

static Shader *shader_terrain = NULL;
static Shader *shader_character = NULL;

static CameraOrbit *camera = NULL;

static Character *character = NULL;
static CharacterOptions *char_options = NULL;
static PFNN *pfnn = NULL;
static Trajectory *trajectory = NULL;
static IK *ik = NULL;

static void pre_render()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

	camera->update(xpos, ypos);

	/* Update Target Direction / Velocity */

	int x_vel = 0;
	int y_vel = 0;
	int vel = -32768;
	int strafe = -32768;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		y_vel += 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		x_vel += 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		y_vel -= 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		x_vel -= 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		vel += 65535;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
	{
		strafe += 65535;
	}

	character->update_move(x_vel, y_vel, camera->direction(), vel, strafe);

	character->forecast(areas);

	character->update(heightmap);

	/* Perform Regression */

	character->predict_pfnn();

	character->build_local_transform();

	character->set_ik(heightmap);
	
	character->post_update_trajectory(areas);
}

void render()
{
	float distance = glm::length(camera->target - character->getPosition());
	
	if(distance > 100) {
		glm::vec3 new_target = glm::normalize(character->getPosition() - camera->target) * (distance - 100);

		camera->target += new_target;
	}

	glm::mat4 light_view = glm::lookAt(camera->target + light->position, camera->target, glm::vec3(0, 1, 0));
	glm::mat4 light_proj = glm::ortho(-500.0f, 500.0f, -500.0f, 500.0f, 10.0f, 10000.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearDepth(1.0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 light_direction = glm::normalize(light->target - light->position);

	glUseProgram(shader_terrain->program);

	glUniformMatrix4fv(glGetUniformLocation(shader_terrain->program, "view"), 1, GL_FALSE, glm::value_ptr(camera->view_matrix()));
	glUniformMatrix4fv(glGetUniformLocation(shader_terrain->program, "proj"), 1, GL_FALSE, glm::value_ptr(camera->proj_matrix()));
	glUniform3f(glGetUniformLocation(shader_terrain->program, "light_dir"), light_direction.x, light_direction.y, light_direction.z);

	glUniformMatrix4fv(glGetUniformLocation(shader_terrain->program, "light_view"), 1, GL_FALSE, glm::value_ptr(light_view));
	glUniformMatrix4fv(glGetUniformLocation(shader_terrain->program, "light_proj"), 1, GL_FALSE, glm::value_ptr(light_proj));

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, light->tex);
	glUniform1i(glGetUniformLocation(shader_terrain->program, "shadows"), 0);

	glBindBuffer(GL_ARRAY_BUFFER, heightmap->vbo);

	glEnableVertexAttribArray(glGetAttribLocation(shader_terrain->program, "vPosition"));
	glEnableVertexAttribArray(glGetAttribLocation(shader_terrain->program, "vNormal"));
	glEnableVertexAttribArray(glGetAttribLocation(shader_terrain->program, "vAO"));

	glVertexAttribPointer(glGetAttribLocation(shader_terrain->program, "vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 0));
	glVertexAttribPointer(glGetAttribLocation(shader_terrain->program, "vNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 3));
	glVertexAttribPointer(glGetAttribLocation(shader_terrain->program, "vAO"), 1, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void *)(sizeof(float) * 6));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heightmap->tbo);

	glDrawElements(GL_TRIANGLES, ((heightmap->data.size() - 1) / 2) * ((heightmap->data[0].size() - 1) / 2) * 2 * 3, GL_UNSIGNED_INT, (void *)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(glGetAttribLocation(shader_terrain->program, "vPosition"));
	glDisableVertexAttribArray(glGetAttribLocation(shader_terrain->program, "vNormal"));
	glDisableVertexAttribArray(glGetAttribLocation(shader_terrain->program, "vAO"));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glUseProgram(shader_character->program);

	glUniformMatrix4fv(glGetUniformLocation(shader_character->program, "view"), 1, GL_FALSE, glm::value_ptr(camera->view_matrix()));
	glUniformMatrix4fv(glGetUniformLocation(shader_character->program, "proj"), 1, GL_FALSE, glm::value_ptr(camera->proj_matrix()));
	glUniform3f(glGetUniformLocation(shader_character->program, "light_dir"), light_direction.x, light_direction.y, light_direction.z);

	glUniformMatrix4fv(glGetUniformLocation(shader_character->program, "light_view"), 1, GL_FALSE, glm::value_ptr(light_view));
	glUniformMatrix4fv(glGetUniformLocation(shader_character->program, "light_proj"), 1, GL_FALSE, glm::value_ptr(light_proj));

	glUniformMatrix4fv(glGetUniformLocation(shader_character->program, "joints"), Character::JOINT_NUM, GL_FALSE, (float *)character->joint_mesh_xform);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, light->tex);
	glUniform1i(glGetUniformLocation(shader_character->program, "shadows"), 0);

	glBindBuffer(GL_ARRAY_BUFFER, character->vbo);

	glEnableVertexAttribArray(glGetAttribLocation(shader_character->program, "vPosition"));
	glEnableVertexAttribArray(glGetAttribLocation(shader_character->program, "vNormal"));
	glEnableVertexAttribArray(glGetAttribLocation(shader_character->program, "vAO"));
	glEnableVertexAttribArray(glGetAttribLocation(shader_character->program, "vWeightVal"));
	glEnableVertexAttribArray(glGetAttribLocation(shader_character->program, "vWeightIds"));

	glVertexAttribPointer(glGetAttribLocation(shader_character->program, "vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(float) * 15, (void *)(sizeof(float) * 0));
	glVertexAttribPointer(glGetAttribLocation(shader_character->program, "vNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(float) * 15, (void *)(sizeof(float) * 3));
	glVertexAttribPointer(glGetAttribLocation(shader_character->program, "vAO"), 1, GL_FLOAT, GL_FALSE, sizeof(float) * 15, (void *)(sizeof(float) * 6));
	glVertexAttribPointer(glGetAttribLocation(shader_character->program, "vWeightVal"), 4, GL_FLOAT, GL_FALSE, sizeof(float) * 15, (void *)(sizeof(float) * 7));
	glVertexAttribPointer(glGetAttribLocation(shader_character->program, "vWeightIds"), 4, GL_FLOAT, GL_FALSE, sizeof(float) * 15, (void *)(sizeof(float) * 11));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, character->tbo);
	glDrawElements(GL_TRIANGLES, character->ntri, GL_UNSIGNED_INT, (void *)0);

	glDisableVertexAttribArray(glGetAttribLocation(shader_character->program, "vPosition"));
	glDisableVertexAttribArray(glGetAttribLocation(shader_character->program, "vNormal"));
	glDisableVertexAttribArray(glGetAttribLocation(shader_character->program, "vAO"));
	glDisableVertexAttribArray(glGetAttribLocation(shader_character->program, "vWeightVal"));
	glDisableVertexAttribArray(glGetAttribLocation(shader_character->program, "vWeightIds"));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);

	// glMatrixMode(GL_MODELVIEW);
	// glLoadMatrixf(glm::value_ptr(camera->view_matrix()));

	// glMatrixMode(GL_PROJECTION);
	// glLoadMatrixf(glm::value_ptr(camera->proj_matrix()));

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

int gl_init()
{

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BASE", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);


	return 0;
}

int main(void)
{
	gl_init();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	camera = new CameraOrbit();
	light = new LightDirectional();

	pfnn = new PFNN(MODE_CONSTANT);

	trajectory = new Trajectory(pfnn);
	char_options = new CharacterOptions();
	ik = new IK();
	character = new Character(trajectory, ik, char_options);
	character->load(
		"./network/character_vertices.bin",
		"./network/character_triangles.bin",
		"./network/character_parents.bin",
		"./network/character_xforms.bin");

	shader_terrain = new Shader();
	shader_terrain->load("./shaders/terrain.vs", "./shaders/terrain_low.fs");
	shader_character = new Shader();
	shader_character->load("./shaders/character.vs", "./shaders/character_low.fs");

	heightmap = new Heightmap();
	areas = new Areas();

	heightmap->load("./heightmaps/hmap_004_smooth.txt", 1.0);

	areas->clear();
	areas->add_wall(glm::vec2(1013.78, -1023.47), glm::vec2( 1013.78,  1037.65), 20);
	areas->add_wall(glm::vec2(1013.78,  1037.65), glm::vec2(-1005.93,  1032.48), 20);
	areas->add_wall(glm::vec2(-1005.93, 1032.48), glm::vec2( -1012.46, -985.26), 20);
	areas->add_wall(glm::vec2(-1012.46, -985.26), glm::vec2( -680.57, -1001.82), 20);
	areas->add_wall(glm::vec2(-680.57, -1001.82), glm::vec2( -571.86, -1008.58), 20);
	areas->add_wall(glm::vec2(-571.86, -1008.58), glm::vec2( -441.50, -1025.14), 20);
	areas->add_wall(glm::vec2(-441.50, -1025.14), glm::vec2( -205.33, -1023.47), 20);
	areas->add_wall(glm::vec2(-205.33, -1023.47), glm::vec2( 1018.95, -1023.47), 20);

	character->reset_position(glm::vec2(0, 0), heightmap);

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render();

		pre_render();


		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glDeleteVertexArrays(1, &VertexArrayID);

	delete camera;
	delete light;
	delete character;
	delete trajectory;
	delete ik;
	delete shader_terrain;
	delete shader_character;
	delete heightmap;
	delete areas;
	delete pfnn;

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
