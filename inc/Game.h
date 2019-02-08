#ifndef GAME_H
#define GAME_H

#include <GL/glew.h>

#include "LightDirectional.h"
#include "CameraOrbit.h"

#include "scene/Scene.h"
#include "character/Character.h"

class Game
{
  public:
    ~Game();

    void init();

    void loop();

  private:
    GLuint VertexArrayID;

    LightDirectional *light;
    Scene *scene;
    CameraOrbit *camera;
    Character *character;

    void update();
    void render();
    void gl_init();
    void get_input_keyboard(glm::vec2 *direction_velocity, int *vel, int *strafe, bool *is_crouched);
};

#endif // !GAME_H