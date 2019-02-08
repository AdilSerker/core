#ifndef SCENE_H
#define SCENE_H

#include "Heightmap.h"
#include "Areas.h"

#include "character/Character.h"

#include "CameraOrbit.h"
#include "LightDirectional.h"
#include "Shader.h"

class Scene
{
public:
	Scene();
	~Scene();

	void draw(LightDirectional *light, CameraOrbit *camera);
	void add_character(Character *character);
	void load_start_location();
	void load_location1();
	void load_test_location();

private:
	Shader *shader;
	Character *character;
	Heightmap *heightmap;
	Areas *areas;
};

#endif // !SCENE_H