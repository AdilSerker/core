#include <math.h>
#include "Heightmap.h"

#include <iostream>




  
Heightmap::Heightmap() {
  hscale = 3.937007874;
  vscale = 3.0;
	offset = 0.0;
	vbo = 0;
	tbo = 0;
	
  loading = false;
  initialized = false;


}

Heightmap::~Heightmap() {
	if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
	if (tbo != 0) { glDeleteBuffers(1, &tbo); tbo = 0; } 
}

double dRand(double dMin, double dMax)
{
	double d = (double)rand() / RAND_MAX;
	return dMin + d * (dMax - dMin);
}


int count = 0;


void Heightmap::diamondSquare(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int range, unsigned level) {
    if (level < 1) return;
    
	// diamonds
    for (unsigned i = x1 + level; i < x2; i += level) {
		
        for (unsigned j = y1 + level; j < y2; j += level) {
			
            float a = data[i - level][j - level];
			float b = data[i][j - level];
			float c = data[i - level][j];
			float d = data[i][j];
			float e = data[i - level / 2][j - level / 2] = (a + b + c + d) / 4 + dRand(0, range);

        }
	}
    // squares
    for (unsigned i = x1 + 2 * level; i < x2; i += level)
        for (unsigned j = y1 + 2 * level; j < y2; j += level) {
            float a = data[i - level][j - level];
            float b = data[i][j - level];
            float c = data[i - level][j];
            float d = data[i][j];
            float e = data[i - level / 2][j - level / 2];

            float f = data[i - level][j - level / 2] = (a + c + e + data[i - 3 * level / 2][j - level / 2]) / 4 + dRand(0, range);
            float g = data[i - level / 2][j - level] = (a + b + e + data[i - level / 2][j - 3 * level / 2]) / 4 + dRand(0, range);
        }

    diamondSquare(x1, y1, x2, y2, range / 2, level / 2);
}

void Heightmap::generate(int size, float multiplier) {
  
	vscale = multiplier * vscale;
  	hscale = multiplier * hscale;
	
	if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
	if (tbo != 0) { glDeleteBuffers(1, &tbo); tbo = 0; }
	
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &tbo);
	
	data.clear();
	
  	srand(time(NULL));
  
  	std::string line;
  	for(int x = 0; x < size; ++x) {
		std::vector<float> row;

      for(int y = 0; y < size; ++y) {
        float f = 0.0f;
        row.push_back(f);
      }
		data.push_back(row);
  	}

	int range = 100;

	data[0][0] = dRand(0, range);
	srand(time(NULL));
	data[0][size-1] = dRand(0, range);
	srand(time(NULL));
	data[size-1][0] = dRand(0, range);
	srand(time(NULL));
	data[size-1][size-1] = dRand(0, range);;
	srand(time(NULL));
	diamondSquare(0, 0, size-1, size-1, range, size-1);
	
  
  int w = data.size();
  int h = data[0].size();
  
  offset = 0.0;
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    	offset += data[x][y];
  }
  offset /= w * h;
  
  printf("Generate Heightmap (%i %i)\n", (int)w, (int)h);
  
  glm::vec3* posns = (glm::vec3*)malloc(sizeof(glm::vec3) * w * h);
  glm::vec3* norms = (glm::vec3*)malloc(sizeof(glm::vec3) * w * h);
  float* aos   = (float*)malloc(sizeof(float) * w * h);
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    float cx = hscale * x, cy = hscale * y, cw = hscale * w, ch = hscale * h;
    posns[x+y*w] = glm::vec3(cx - cw/2, sample(glm::vec2(cx-cw/2, cy-ch/2)), cy - ch/2);
  }
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    norms[x+y*w] = (x > 0 && x < w-1 && y > 0 && y < h-1) ?
      glm::normalize(glm::mix(
        glm::cross(
          posns[(x+0)+(y+1)*w] - posns[x+y*w],
          posns[(x+1)+(y+0)*w] - posns[x+y*w]),
        glm::cross(
          posns[(x+0)+(y-1)*w] - posns[x+y*w],
          posns[(x-1)+(y+0)*w] - posns[x+y*w]), 0.5)) : glm::vec3(0,1,0);
  }


  char ao_filename[512];
  memcpy(ao_filename, "./heightmaps/generated.txt", strlen("./heightmaps/generated.txt")-4);
  ao_filename[strlen("./heightmaps/generated.txt")-4] = '\0';
  strcat(ao_filename, "_ao.txt");
  
  srand(0);

  FILE* ao_file = fopen(ao_filename, "r");
  bool ao_generate = false;
  if (ao_file == NULL || ao_generate) {
    ao_file = fopen(ao_filename, "w");
    ao_generate = true;
  }
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    
    if (ao_generate) {
    
      float ao_amount = 0.0;
      float ao_radius = 50.0;
      int ao_samples = 1024;
      int ao_steps = 5;
      for (int i = 0; i < ao_samples; i++) {
        glm::vec3 off = glm::normalize(glm::vec3(rand() % 10000 - 5000, rand() % 10000 - 5000, rand() % 10000 - 5000));
        if (glm::dot(off, norms[x+y*w]) < 0.0f) { off = -off; }
        for (int j = 1; j <= ao_steps; j++) {
          glm::vec3 next = posns[x+y*w] + (((float)j) / ao_steps) * ao_radius * off;
          if (sample(glm::vec2(next.x, next.z)) > next.y) { ao_amount += 1.0; break; }
        }
      }
      
      aos[x+y*w] = 1.0 - (ao_amount / ao_samples);
      fprintf(ao_file, y == h-1 ? "%f\n" : "%f ", aos[x+y*w]);
    } else {
      fscanf(ao_file, y == h-1 ? "%f\n" : "%f ", &aos[x+y*w]);
    }
    
  }
  
  fclose(ao_file);

  float *vbo_data = (float*)malloc(sizeof(float) * 7 * w * h);

  uint32_t *tbo_data = (uint32_t*)malloc(sizeof(uint32_t) * 3 * 2 * ((w-1)/2) * ((h-1)/2));
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    vbo_data[x*7+y*7*w+0] = posns[x+y*w].x; 
    vbo_data[x*7+y*7*w+1] = posns[x+y*w].y;
    vbo_data[x*7+y*7*w+2] = posns[x+y*w].z;
    vbo_data[x*7+y*7*w+3] = norms[x+y*w].x;
    vbo_data[x*7+y*7*w+4] = norms[x+y*w].y;
    vbo_data[x*7+y*7*w+5] = norms[x+y*w].z; 
    vbo_data[x*7+y*7*w+6] = aos[x+y*w]; 
  }
  
  free(posns);
  free(norms);
  free(aos);

  for (int x = 0; x < (w-1)/2; x++)
  for (int y = 0; y < (h-1)/2; y++) {
    tbo_data[x*3*2+y*3*2*((w-1)/2)+0] = (x*2+0)+(y*2+0)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+1] = (x*2+0)+(y*2+2)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+2] = (x*2+2)+(y*2+0)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+3] = (x*2+2)+(y*2+2)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+4] = (x*2+2)+(y*2+0)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+5] = (x*2+0)+(y*2+2)*w;
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * w * h, vbo_data, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 3 * 2 * ((w-1)/2) * ((h-1)/2), tbo_data, GL_STATIC_DRAW);  
  
  free(vbo_data);
  free(tbo_data);
  
}

void Heightmap::load(const char* filename, float multiplier) {
  loading = true;

	vscale = multiplier * vscale;
  hscale = multiplier * hscale;
	
	if (vbo != 0) { glDeleteBuffers(1, &vbo); vbo = 0; }
	if (tbo != 0) { glDeleteBuffers(1, &tbo); tbo = 0; }
	
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &tbo);
	
	data.clear();
	
  	std::ifstream file(filename);
  
  	std::string line;
  	while (std::getline(file, line)) {
		std::vector<float> row;
		std::istringstream iss(line);
		while (iss) {
		float f;
		iss >> f;
		row.push_back(f);
		}
		data.push_back(row);
  	}
  
  int w = data.size();
  int h = data[0].size();
  
  offset = 0.0;
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    	offset += data[x][y];
  }
  offset /= w * h;


  
  printf("Loaded Heightmap '%s' (%i %i)\n", filename, (int)w, (int)h);
  
  glm::vec3* posns = (glm::vec3*)malloc(sizeof(glm::vec3) * w * h);
  glm::vec3* norms = (glm::vec3*)malloc(sizeof(glm::vec3) * w * h);
  float* aos   = (float*)malloc(sizeof(float) * w * h);
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    float cx = hscale * x, cy = hscale * y, cw = hscale * w, ch = hscale * h;

    posns[x+y*w] = glm::vec3(cx - cw/2, sample(glm::vec2(cx-cw/2, cy-ch/2)), cy - ch/2);
  }
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    norms[x+y*w] = (x > 0 && x < w-1 && y > 0 && y < h-1) ?
      glm::normalize(glm::mix(
        glm::cross(
          posns[(x+0)+(y+1)*w] - posns[x+y*w],
          posns[(x+1)+(y+0)*w] - posns[x+y*w]),
        glm::cross(
          posns[(x+0)+(y-1)*w] - posns[x+y*w],
          posns[(x-1)+(y+0)*w] - posns[x+y*w]), 0.5)) : glm::vec3(0,1,0);
  }


  char ao_filename[512];
  memcpy(ao_filename, filename, strlen(filename)-4);
  ao_filename[strlen(filename)-4] = '\0';
  strcat(ao_filename, "_ao.txt");
  
  srand(0);

  FILE* ao_file = fopen(ao_filename, "r");
  bool ao_generate = false;
  if (ao_file == NULL || ao_generate) {
    ao_file = fopen(ao_filename, "w");
    ao_generate = true;
  }
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    
    if (ao_generate) {
    
      float ao_amount = 0.0;
      float ao_radius = 50.0;
      int ao_samples = 1024;
      int ao_steps = 5;
      for (int i = 0; i < ao_samples; i++) {
        glm::vec3 off = glm::normalize(glm::vec3(rand() % 10000 - 5000, rand() % 10000 - 5000, rand() % 10000 - 5000));
        if (glm::dot(off, norms[x+y*w]) < 0.0f) { off = -off; }
        for (int j = 1; j <= ao_steps; j++) {
          glm::vec3 next = posns[x+y*w] + (((float)j) / ao_steps) * ao_radius * off;
          if (sample(glm::vec2(next.x, next.z)) > next.y) { ao_amount += 1.0; break; }
        }
      }
      
      aos[x+y*w] = 1.0 - (ao_amount / ao_samples);
      fprintf(ao_file, y == h-1 ? "%f\n" : "%f ", aos[x+y*w]);
    } else {
      fscanf(ao_file, y == h-1 ? "%f\n" : "%f ", &aos[x+y*w]);
    }
    
  }
  
  fclose(ao_file);

  float *vbo_data = (float*)malloc(sizeof(float) * 7 * w * h);

  uint32_t *tbo_data = (uint32_t*)malloc(sizeof(uint32_t) * 3 * 2 * ((w-1)/2) * ((h-1)/2));
  
  for (int x = 0; x < w; x++)
  for (int y = 0; y < h; y++) {
    vbo_data[x*7+y*7*w+0] = posns[x+y*w].x; 
    vbo_data[x*7+y*7*w+1] = posns[x+y*w].y;
    vbo_data[x*7+y*7*w+2] = posns[x+y*w].z;
    vbo_data[x*7+y*7*w+3] = norms[x+y*w].x;
    vbo_data[x*7+y*7*w+4] = norms[x+y*w].y;
    vbo_data[x*7+y*7*w+5] = norms[x+y*w].z; 
    vbo_data[x*7+y*7*w+6] = aos[x+y*w]; 
  }
  
  free(posns);
  free(norms);
  free(aos);

  for (int x = 0; x < (w-1)/2; x++)
  for (int y = 0; y < (h-1)/2; y++) {
    tbo_data[x*3*2+y*3*2*((w-1)/2)+0] = (x*2+0)+(y*2+0)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+1] = (x*2+0)+(y*2+2)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+2] = (x*2+2)+(y*2+0)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+3] = (x*2+2)+(y*2+2)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+4] = (x*2+2)+(y*2+0)*w;
    tbo_data[x*3*2+y*3*2*((w-1)/2)+5] = (x*2+0)+(y*2+2)*w;
  }
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 7 * w * h, vbo_data, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tbo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 3 * 2 * ((w-1)/2) * ((h-1)/2), tbo_data, GL_STATIC_DRAW);  
  
  free(vbo_data);
  free(tbo_data);
  
  loading = false;
}

float Heightmap::sample(glm::vec2 pos) {

  int w = data.size();
  int h = data[0].size();
  
  pos.x = (pos.x/hscale) + w/2;
  pos.y = (pos.y/hscale) + h/2;
  
  float a0 = fmod(pos.x, 1.0);
  float a1 = fmod(pos.y, 1.0);
  
  int x0 = (int)std::floor(pos.x), x1 = (int)std::ceil(pos.x);
  int y0 = (int)std::floor(pos.y), y1 = (int)std::ceil(pos.y);
  
  x0 = x0 < 0 ? 0 : x0; x0 = x0 >= w ? w-1 : x0;
  x1 = x1 < 0 ? 0 : x1; x1 = x1 >= w ? w-1 : x1;
  y0 = y0 < 0 ? 0 : y0; y0 = y0 >= h ? h-1 : y0;
  y1 = y1 < 0 ? 0 : y1; y1 = y1 >= h ? h-1 : y1;
  
  float s0 = vscale * (data[x0][y0] - offset);
  float s1 = vscale * (data[x1][y0] - offset);
  float s2 = vscale * (data[x0][y1] - offset);
  float s3 = vscale * (data[x1][y1] - offset);
  
  return (s0 * (1-a0) + s1 * a0) * (1-a1) + (s2 * (1-a0) + s3 * a0) * a1;

}
