PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS = -L/usr/local/lib -lglfw -lGLEW -lGL #-lfreetype -lGLU -pthread -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11
	EXT = 
endif

MODULE = src/modules/*.cpp src/modules/**/*.cpp
WFLAGS = -Wall 
CFLAGS = -O3 -ffast-math

base: src/main.cpp 
	rm -f game && g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(MODULE) $(LFLAGS) -o game

debug: src/main.cpp 
	g++ -std=gnu++11 $(WFLAGS) $< $(MODULE) -g -c

all: src/main.cpp 
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(MODULE) -c

game: src/modules/Game.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< -c

scene: src/modules/Scene/Scene.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< -c

character: src/modules/Character/Character.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< -c

build: 
	rm -f core && g++ ./*.o $(LFLAGS) -o game

clean:
	rm -f core game ./*.o
