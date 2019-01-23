#ifndef SCENE_H
#define SCENE_H

#include "Heightmap.h"
#include "Areas.h"

#include "../Character/Character.h"

#include "../CameraOrbit.h"
#include "../LightDirectional.h"
#include "../Shader.h"

class Scene {
    public:
    Scene();
    ~Scene();

    void draw(LightDirectional *light, CameraOrbit *camera);
    void add_character(Character *character);
    void load_start_location();
    void load_location1();
    void generate();

    Heightmap *heightmap;
    private:
    Shader *shader;
    Character *character;
    Areas *areas;
};

#endif // !SCENE_H