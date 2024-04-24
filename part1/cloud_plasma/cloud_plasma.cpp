#include <SDL2/SDL.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

const int SCREENSIZE_X = 640;  // Adjust accordingly to your screen
const int SCREENSIZE_Y = 480;
const int XMIN = 0;
const int XMAX = SCREENSIZE_X - 1;
const int YMIN = 2;
const int YMAX = SCREENSIZE_Y - 1;
const double RANDMONESS = 1.7;  // Play with this for more fun. The higher the value, the more pixelated the cloud is
const double MAXIMUM_RANDOM = static_cast<double>(RAND_MAX);

/**
 * Will rotate the palette of the specified size with the given number of rotations
 **/
void rotatePalette(SDL_Color arr[], int size) 
{
  SDL_Color c0 = arr[0];
  for (int i = 0; i< size - 1; i++)
  {
    arr[i] = arr[i + 1];
  }
  arr[size - 1] = c0;
}

/**
 * Places a pixel with the specified colour at the given coordinates.
 **/
void putPixel(int x, int y, Uint8 c, Uint8* screen) 
{
  screen[SCREENSIZE_X * y + x] = c; 
}

/**
 * Retrieves the value of the colour of the pixel found at the given coordinates 
 **/
Uint8 getPixel(int x, int y, Uint8* screen) 
{ 
  return screen[SCREENSIZE_X * y + x]; 
}

/**
 * This is the diamond step in the Diamond-Square algorithm.
 * This function is responsible for adjusting the midpoint of
 * a line (either horizontal or vertical) between two given points. 
 * It calculates the midpoint value by averaging the values of the 
 * The rf factor controls the degree of randomness in the displacement.
*/
void diamondStep(int x1, int y1, int x, int y, int x2, int y2, double rf, Uint8* screen) 
{
  if (getPixel(x, y, screen) != 0) 
  {
    return;
  }

  int d = abs(x1 - x2) + abs(y1 - y2);
  // calculate a new colour
  int v = static_cast<int>((getPixel(x1, y1, screen) + getPixel(x2, y2, screen)) / 2 + (rand() / MAXIMUM_RANDOM - 0.5) * d * rf);
  v = std::clamp(v, 1, 255); // Ensure v is within the valid range

  putPixel(x, y, static_cast<Uint8>(v), screen);
}

/**
 * This function recursively subdivides the terrain represented by the input square 
 * defined by (x1, y1) and (x2, y2). 
 * It then calls the diamondStep function for each edge of the square and calculates 
 * the midpoint value for the center of the square if it hasn't been set already. 
 * The terrain is further subdivided into smaller squares until a certain size 
 * is reached, at which point the recursion stops.
 **/
void squareStep(int x1, int y1, int x2, int y2, Uint8* screen) 
{
  if((x2 - x1 < 2) && (y2 - y1 < 2)) 
  {
    return;
  }

  int x = (x1 + x2) / 2;
  int y = (y1 + y2) / 2;

  diamondStep(x1, y1, x, y1, x2, y1, RANDMONESS, screen);
  diamondStep(x2, y1, x2, y, x2, y2, RANDMONESS, screen);
  diamondStep(x1, y2, x, y2, x2, y2, RANDMONESS, screen);
  diamondStep(x1, y1, x1, y, x1, y2, RANDMONESS, screen);

  if (getPixel(x, y, screen) == 0) 
  {
    double v = (getPixel(x1, y1, screen) + getPixel(x2, y1, screen) +
                getPixel(x2, y2, screen) + getPixel(x1, y2, screen)) /
               4.0;
    putPixel(x, y, static_cast<Uint8>(v), screen);
  }

  squareStep(x1, y1, x, y, screen);
  squareStep(x, y1, x2, y, screen);
  squareStep(x, y, x2, y2, screen);
  squareStep(x1, y, x, y2, screen);
}

void initializeScreen(Uint8* screen) {
  putPixel(0, 0, 1 + rand() % 255, screen);
  putPixel(XMAX, 0, 1 + rand() % 255, screen);
  putPixel(XMAX, YMAX, 1 + rand() % 255, screen);
  putPixel(0, YMAX, 1 + rand() % 255, screen);
  squareStep(0, 0, XMAX, YMAX, screen);
}

void generateColorCyclePalette(SDL_Color* colours) {
    // Set the main colors at specific indices
    colours[0].r = 0;
    colours[0].g = 0;
    colours[0].b = 0;
    colours[0].a = 255; // Black

    colours[85].r = 255;
    colours[85].g = 165; // RGB values for orange
    colours[85].b = 0;
    colours[85].a = 255; // Orange

    colours[170].r = 0;
    colours[170].g = 255;
    colours[170].b = 255;
    colours[170].a = 255; // Cyan

    // Generate transitional shades between the main colors
    for (int i = 1; i < 85; ++i) {
        float ratio = static_cast<float>(i) / 85.0f;

        colours[i].r = static_cast<Uint8>((1.0f - ratio) * colours[0].r + ratio * colours[85].r);
        colours[i].g = static_cast<Uint8>((1.0f - ratio) * colours[0].g + ratio * colours[85].g);
        colours[i].b = static_cast<Uint8>((1.0f - ratio) * colours[0].b + ratio * colours[85].b);
        colours[i].a = 255; // Alpha value, fully opaque
    }

    for (int i = 86; i < 170; ++i) {
        float ratio = static_cast<float>(i - 85) / 84.0f;

        colours[i].r = static_cast<Uint8>((1.0f - ratio) * colours[85].r + ratio * colours[170].r);
        colours[i].g = static_cast<Uint8>((1.0f - ratio) * colours[85].g + ratio * colours[170].g);
        colours[i].b = static_cast<Uint8>((1.0f - ratio) * colours[85].b + ratio * colours[170].b);
        colours[i].a = 255; // Alpha value, fully opaque
    }

    for (int i = 171; i < 256; ++i) {
        float ratio = static_cast<float>(i - 170) / 85.0f;

        colours[i].r = static_cast<Uint8>((1.0f - ratio) * colours[170].r + ratio * colours[0].r);
        colours[i].g = static_cast<Uint8>((1.0f - ratio) * colours[170].g + ratio * colours[0].g);
        colours[i].b = static_cast<Uint8>((1.0f - ratio) * colours[170].b + ratio * colours[0].b);
        colours[i].a = 255; // Alpha value, fully opaque
    }
}


int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  SDL_Color colours[256];
  generateColorCyclePalette(colours);

  int w = SCREENSIZE_X;
  int h = SCREENSIZE_Y;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Cloud Plasma", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, w, h, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_Surface* surface =
      SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
  if (surface == NULL) {
    std::cerr << "Cannot create surface" << std::endl;
    exit(1);
  }

  const int screenWidth = SCREENSIZE_X;
  const int screenHeight = SCREENSIZE_Y;

  Uint8* screen = new Uint8[screenWidth * screenHeight + 1];
  SDL_Texture* texture = nullptr;
  memset(screen, 0, screenWidth * screenHeight + 1);

  SDL_SetPaletteColors(surface->format->palette, colours, 0, 256);

  initializeScreen(screen);
    //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  while (true) {
    SDL_Event e;

    while (SDL_PollEvent(&e) > 0) {
      switch (e.type) {
        case SDL_QUIT:
          delete[] screen;
          SDL_FreeSurface(surface);
          SDL_DestroyRenderer(renderer);
          SDL_DestroyWindow(window);
          SDL_Quit();
          return EXIT_SUCCESS;
      }
    }

    rotatePalette(colours, 256);
    SDL_SetPaletteColors(surface->format->palette, colours, 0, 256);


    uint8_t* offscreen = (uint8_t*)surface->pixels;
    memcpy(offscreen, screen, screenWidth * screenHeight);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);

    SDL_Delay(10);
  }
}
