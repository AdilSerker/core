PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS = -L/usr/local/lib -lglfw -lGLEW -lGL #-lfreetype -lGLU -pthread -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11
	EXT = 
endif

MODULE = src/modules/*.cpp src/modules/**/*.cpp
WFLAGS = -Wall 
CFLAGS = -O3 -ffast-math

DIR_OBJ = obj

base: src/main.cpp 
	rm -f base && g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(MODULE) $(LFLAGS) -o $@

debug: src/main.cpp 
	g++ -std=gnu++11 $(WFLAGS) $< $(MODULE) -g -c

compile: src/main.cpp 
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(MODULE) -c

game: src/modules/Game.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< -c

scene: src/modules/Scene/Scene.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< -c

character: src/modules/Character/Character.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< -c

core: 
	rm -f core && g++ ./*.o $(LFLAGS) -o $@

