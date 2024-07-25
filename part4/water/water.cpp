#include <SDL2/SDL.h>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

const int SCREENSIZE_X = 800;
const int SCREENSIZE_Y = 600;
const int XMIN = 0;
const int XMAX = SCREENSIZE_X - 1;
const int YMIN = 2;
const int YMAX = SCREENSIZE_Y - 1;

const int RIPPLE_DENSITY = 128;
const float RIPPLE_HEIGHT = 2.0;
const bool LIGHT = false;
const int WATER_WOBBLITY = 8;

int heightMap[2][SCREENSIZE_X * SCREENSIZE_Y] = {0};

void putPixel(int x, int y, Uint8 c, Uint8* screen) {
  screen[SCREENSIZE_X * y + x] = c;
}

Uint8 getPixel(int x, int y, Uint8* screen) {
  return screen[SCREENSIZE_X * y + x];
}

void generatePalette(SDL_Color* c, size_t cnt) {
  // Add your palette generation logic here
}

int heightSum(int* currentMap, int index) {
  return currentMap[index + SCREENSIZE_X] + 
         currentMap[index - SCREENSIZE_X] +
         currentMap[index + 1] + 
         currentMap[index - 1] +
         currentMap[index - SCREENSIZE_X - 1] +
         currentMap[index - SCREENSIZE_X + 1] +
         currentMap[index + SCREENSIZE_X - 1] +
         currentMap[index + SCREENSIZE_X + 1];
}

void calculateWater(int currentPage, int density) {
  int count = SCREENSIZE_X + 1;

  int* newptr = &heightMap[currentPage][0];
  int* oldptr = &heightMap[currentPage ^ 1][0];
  int y = (SCREENSIZE_Y - 1) * SCREENSIZE_X;

  while (count < y) 
  {
    int x = count + SCREENSIZE_X - 2;
    while(count < x) 
    {
      int newHeight = ((heightSum(oldptr, count)) / 8) - newptr[count];
      newptr[count] = newHeight - (newHeight / density);
      count++;
    }
    count += 2;
  }
}

void smoothenWater(int currentPage) {
  int count = SCREENSIZE_X + 1;

  int* newptr = &heightMap[currentPage][0];
  int* oldptr = &heightMap[currentPage ^ 1][0];

  for (int y = 1; y < SCREENSIZE_Y - 1; y++) {
    for (int x = 1; x < SCREENSIZE_X - 1; x++) {
      int newHeight = ((heightSum(oldptr, count)) / 8) + newptr[count];
      newptr[count] = newHeight >> 1;
      count++;
    }
    count += 2;
  }
}

void drawWater(int page, const std::vector<int>& imageData, Uint8* screen) 
{
  int* ptr = &heightMap[page][0];
  int offset = SCREENSIZE_X;
  int y = (SCREENSIZE_Y - 1) * SCREENSIZE_X;
  while(offset < y) 
  {
    int x = offset + SCREENSIZE_X - 2;
    while(offset < x) 
    {
      int dx = ptr[offset] - ptr[offset - 1];
      int dy = ptr[offset] - ptr[offset + SCREENSIZE_X];
      size_t idx = (offset + (LIGHT ? 2 : 1) * SCREENSIZE_X * dx + dy) % (SCREENSIZE_X * SCREENSIZE_Y);
      int c = imageData[idx];
      screen[offset] = (c < 0) ? 0 : (c > 254) ? 254 + (LIGHT ? 1 : 0) : c;
      offset++;
    }
    offset+=2;
  }
}

void waterDroplet(int x, int y, int radius, int height, int page) 
{
  int radsquare = pow(radius, 2);
  float length = RIPPLE_HEIGHT / pow(radius, 2);

  height *= pow(RIPPLE_HEIGHT, 3);

  int left = -radius;
  int right = radius;
  int top = -radius;
  int bottom = radius;

  if (x - radius < 1) left -= (x - radius - 1);
  if (y - radius < 1) top -= (y - radius - 1);
  if (x + radius > SCREENSIZE_X - 1) right -= (x + radius - SCREENSIZE_X + 1);
  if (y + radius > SCREENSIZE_Y - 1) bottom -= (y + radius - SCREENSIZE_Y + 1);

  for (int cy = top; cy < bottom; cy++) {
    for (int cx = left; cx < right; cx++) {
      int square = cy * cy + cx * cx;
      if (square < radsquare) {
        int dist = sqrt(square * length + square * length);
        heightMap[page][SCREENSIZE_X * (cy + y) + cx + x] += (int)(((dist) * RIPPLE_DENSITY) * (height)) / ( pow(RIPPLE_HEIGHT, 2));
      }
    }
  }
}

void initializeScreen(Uint8* screen) {
  memset(screen, 0, SCREENSIZE_X * SCREENSIZE_Y);
}

void updateScreen(Uint8* screen, const std::vector<int>& imageData) 
{
  static int dropletRadius = 5, currentHeightMapIndex = 0, dropletCounter = 0;

  dropletCounter++;
  
  calculateWater(currentHeightMapIndex ^ 1, WATER_WOBBLITY);

  for (int cc = 0; cc < dropletCounter; cc++) 
  {
    waterDroplet(SCREENSIZE_X / 2, SCREENSIZE_Y / 2, cc * dropletRadius, dropletRadius * 10, currentHeightMapIndex);
    smoothenWater(currentHeightMapIndex);
    dropletRadius+=4;
  }

  if (dropletCounter == 15) { 
    dropletRadius = 0;
    dropletCounter = 0;
  }

  drawWater(currentHeightMapIndex, imageData, screen);

  currentHeightMapIndex ^= 1;
}

int main() {
  srand(static_cast<unsigned int>(time(nullptr)));
  bool exitRequest = false;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* window = SDL_CreateWindow("Water ripples", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, SCREENSIZE_X,
                                        SCREENSIZE_Y, SDL_WINDOW_ALLOW_HIGHDPI);
  if (!window) {
    std::cerr << "Cannot create window:" << SDL_GetError() << std::endl;
    exit(1);
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    std::cerr << "Cannot create renderer:" << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  SDL_Surface* surface =
      SDL_CreateRGBSurface(0, SCREENSIZE_X, SCREENSIZE_Y, 8, 0, 0, 0, 0);
  if (!surface) {
    std::cerr << "Cannot create surface:" << SDL_GetError() << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  Uint8* screen = new Uint8[SCREENSIZE_X * SCREENSIZE_Y + 1];
  initializeScreen(screen);

  std::vector<int> palette;
  std::vector<int> imageData;
  for (int i = 0; i < SCREENSIZE_X * SCREENSIZE_Y; i++) imageData.push_back(0);

  float a = 0;
  for (int x = 0; x < SCREENSIZE_X; x++)
    for (int y = 0; y < SCREENSIZE_Y;
         y++) { /*std::cout << x << "," << y << std::endl;*/
      a += 0.1;
      if (a > 3.1415 * 2) a = 0;
      imageData[y * SCREENSIZE_X + x] = (int)( sin ((float)x / SCREENSIZE_Y)  * cos( (float)y / SCREENSIZE_Y) * 255);
    }

  SDL_Color colours[256] = {0};
  for (size_t i = 0; i < 256 * 4; i += 4) {
    colours[i / 4].r = 0;
    colours[i / 4].g = 0;
    colours[i / 4].b = i/2;
    colours[i / 4].a = 255;
  }

  colours[255] = {255,255,255,255};

  SDL_SetPaletteColors(surface->format->palette, colours, 0, 255);
//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
  SDL_Texture* texture = nullptr;

  while (!exitRequest) {
    SDL_Event e;

    while (SDL_PollEvent(&e) > 0) {
      switch (e.type) {
        case SDL_QUIT:
          exitRequest = true;
      }
    }
    if (exitRequest) {
      break;
    }

    updateScreen(screen, imageData);

    uint8_t* offscreen = (uint8_t*)surface->pixels;
    memcpy(offscreen, screen, SCREENSIZE_X * SCREENSIZE_Y);

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);

    SDL_Delay(100);
  }

  delete[] screen;
  SDL_FreeSurface(surface);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
