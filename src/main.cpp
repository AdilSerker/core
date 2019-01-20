#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include "modules/Game.h"

static Game *game = NULL;

int main(void)
{
	game = new Game();
	game->init();

	game->loop();

	delete game;

	return 0;
}
