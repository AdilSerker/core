#include "Game.h"

#include <GLFW/glfw3.h>
extern GLFWwindow* window;

Game::~Game() {
    glDeleteVertexArrays(1, &VertexArrayID);

	delete camera;
	delete light;
	delete character;
	delete scene;

	glfwTerminate();
}

void Game::init() {
    gl_init();

	this->camera = new CameraOrbit();
	this->light = new LightDirectional();
	this->character = new Character();
	this->scene = new Scene();

    scene->add_character(character);
	scene->load_start_location();
	
}

void Game::gl_init() {

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		exit(-1);
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
		exit(-1);
	}
	glfwMakeContextCurrent(window);


	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		exit(-1);
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glDepthFunc(GL_LESS);

    glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

}

void Game::update() {
    double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

	camera->update(xpos, ypos);

	glm::vec2 direction_velocity = glm::vec2();
	int vel = -32768;
	int strafe = -32768;
	bool is_crouched = false;

	get_input_keyboard(&direction_velocity, &vel, &strafe, &is_crouched);

	character->update_move(direction_velocity, camera->direction(), vel, strafe, is_crouched);
}

void Game::get_input_keyboard(glm::vec2 *direction_velocity, int *vel, int *strafe, bool *is_crouched) {

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		direction_velocity->y += 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		direction_velocity->x += 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		direction_velocity->y -= 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		direction_velocity->x -= 32768;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		*vel += 65535;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
	{
		*strafe += 65535;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		*is_crouched = true;
	}

}

void Game::render() {
    float distance = glm::length(camera->target - character->getPosition());
	
	if(distance > 100) {
		glm::vec3 new_target = glm::normalize(character->getPosition() - camera->target) * (distance - 100);

		camera->target += new_target;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearDepth(1.0);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene->draw(light, camera);
	character->draw(light, camera);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void Game::loop() {

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0)
	{
		printf("\033c");
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){
			printf("FPS: %i\n", nbFrames);
			nbFrames = 0;
			lastTime += 1.0;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update();
		render();

		if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			scene->load_location1();
		}
		if(glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
			scene->load_start_location();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

	}
}