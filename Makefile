PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS = -L/usr/local/lib -lglfw -lGLEW -lGL #-lfreetype -lGLU -pthread -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11
	EXT = 
endif

INC = -I inc

MODULE = src/modules/*.cpp src/modules/**/*.cpp
WFLAGS = -Wall 
CFLAGS = -O3 -ffast-math
DBG = -g -rdynamic

base: src/main.cpp 
	rm -f game && g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(MODULE) $(INC) $(LFLAGS) -o game

debug: src/main.cpp 
	g++ -std=gnu++11 $(WFLAGS) $(DBG) $< $(MODULE) $(INC) -c

all: src/main.cpp 
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(MODULE) $(INC) -c && make build

game: src/modules/Game.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(INC) -c && make build

scene: src/modules/Scene/Scene.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(INC) -c && make build

hm: src/modules/Scene/Heightmap.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(INC) -c && make build

character: src/modules/Character/Character.cpp
	g++ -std=gnu++11 $(CFLAGS) $(WFLAGS) $< $(INC) -c && make build

build: 
	rm -f core && g++ ./*.o $(LFLAGS) $(INC) -o game

clean:
	rm -f core game ./*.o
