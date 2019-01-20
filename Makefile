PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)),Linux)
	LFLAGS = -L/usr/local/lib -lglfw -lGLEW -lGL #-lfreetype -lGLU -pthread -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11
	EXT = 
endif

MODULE = src/modules/*.cpp src/modules/**/*.cpp
CFLAGS = -O3 -Wall -ffast-math

base: src/main.cpp 
	g++ -std=gnu++11 $(CFLAGS) $< $(MODULE) $(LFLAGS) -o $@
  
clean:
	rm base$(EXT)
