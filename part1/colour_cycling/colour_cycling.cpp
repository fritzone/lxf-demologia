#include <SDL2/SDL.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

// The size of the screen, for this situation it will be 512x512
const int SCREENSIZE_X = 512;
const int SCREENSIZE_Y = 512;

/**
 * Will generate the palette for the color cycling. The palette will be
 * emulating some stripes, of red, green, blue colours
 **/
void generateColorCyclePalette(SDL_Color* colours)
{
  for(int i = 0; i < 256; i++) {
    colours[i] = SDL_Color{0, 0, 0, 0};
  }
  for(int i = 0; i < 32; i++) {
    colours[i + 32] = SDL_Color{static_cast<Uint8>(i * 8), 0, 0, 0};
  }
  for(int i = 0; i < 32; i++) {
    colours[i + 64] = SDL_Color{static_cast<Uint8>(255 - i * 8), 0, 0, 0};
  }

  for(int i = 0; i < 32; i++) {
    colours[i + 128] = SDL_Color{0, 0, static_cast<Uint8>(i * 8), 0};
  }
  for(int i = 0; i < 32; i++) {
    colours[i + 160] = SDL_Color{0, 0, static_cast<Uint8>(255 - i * 8), 0};
  }
}


/**
 * Puts a pixel on the screen
 **/
void putPixel(int x, int y, Uint8 c, Uint8* screen) {
  screen[SCREENSIZE_X * y + x] = c;
}

/**
 * Will draw the initial screen. We aim for something that looks like a tunnel, or similar
 **/
void initializeScreen(Uint8* screen) {
  for (int x = 0; x < SCREENSIZE_X; x++) {
    for (int y = x; y < SCREENSIZE_Y - x ; y++) {
      putPixel(x, y, x % 255, screen);
      putPixel(y, x, x  % 255, screen);
      putPixel(SCREENSIZE_X - x, y, x % 255, screen);
    }
  }
}

/**
 * Will rotate the palette, in a way that it emulates the forward movement
 **/
void rotatePalette(SDL_Color arr[], int size) {
  SDL_Color c0 = arr[0];
  for (int i = 0; i< size - 1; i++)
  {
    arr[i] = arr[i + 1];
  }
  arr[size - 1] = c0;
}

int main() {

  // First step is to create our "virtual" screen
  Uint8* screen = new Uint8[SCREENSIZE_X * SCREENSIZE_Y];
  memset(screen, 0, SCREENSIZE_X * SCREENSIZE_Y);

  // We want to be sure that SDL_Quit is called if we normally leave the program
  atexit(SDL_Quit);

  // Initialize SDL
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Cannot create window:" <<  SDL_GetError() << std::endl;
    exit(1);
  }

  // Let's generate a palette
  SDL_Color colours[255] = {0};
  generateColorCyclePalette(colours);

  // Create a windows of the specific size
  SDL_Window* window = SDL_CreateWindow("Colour cycling", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREENSIZE_X, SCREENSIZE_Y, 0);
  if (!window) {
    std::cerr << "Cannot create window:" <<  SDL_GetError() << std::endl;
    exit(1);
  }

  // Create a renderer for the given window
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    std::cerr << "Cannot create renderer:"  <<  SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    exit(1);
  }

  // Create a surface to render to, which is closest to the original screen specs we need: 256 colours (8bit)
  // thus we have 8 for the "depth" parameters, and the special SDL_PIXELFORMAT_INDEX8 indicating a paletted surface,
  // so SDL will automatically allocate a palette
  SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, SCREENSIZE_X, SCREENSIZE_Y, 8, SDL_PIXELFORMAT_INDEX8);
  if (!surface) {
    std::cerr << "Cannot create surface" << std::endl;
    SDL_DestroyRenderer(renderer);    
    SDL_DestroyWindow(window);
    exit(1);
  }

  // Let's draw the initial screen
  initializeScreen(screen);
  // This will be the texture that will be created from the surface
  SDL_Texture* texture = nullptr;

  while (true) {
    // And set the colors of the surface to the one that we have created
    SDL_SetPaletteColors(surface->format->palette, colours, 0, 255);

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

    memcpy(surface->pixels, screen, SCREENSIZE_X * SCREENSIZE_Y);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);
    SDL_Delay(10);

    rotatePalette(colours + 1, 255);
  }
}
