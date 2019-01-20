PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS = -L/usr/local/lib -lglfw -lGLEW -lGL # -lGLU -pthread -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11
	EXT = 
endif

MODULE = src/modules/*.cpp src/modules/**/*.cpp

base: src/main.cpp 
	g++ -std=gnu++11 -Wall -O3 -ffast-math $< $(MODULE) $(LFLAGS) -o $@
  
clean:
	rm base$(EXT)
