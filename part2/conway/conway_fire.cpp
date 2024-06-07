#include <SDL2/SDL.h>

#include <algorithm>  // for std::swap
#include <cstdlib>
#include <ctime>
#include <iostream>

const int SCREENSIZE_X = 640;
const int SCREENSIZE_Y = 480;
const int XMIN = 0;
const int XMAX = SCREENSIZE_X - 1;
const int YMIN = 2;
const int YMAX = SCREENSIZE_Y - 1;
const int FIRE_HEIGHT = 2;
const int CONWAY_DIFFERENTIATOR = 128;

/**
 * Will generate the red components of the fire palette
 **/
std::vector<uint8_t> generateReds() {
  std::vector<uint8_t> result;

  for (int i = 0; i < 18; ++i) {
    result.push_back(0);
  }

  for (int i = 1; i <= 16; ++i) {
    result.push_back(i * 8);
  }
  int count = 128;

  for (int i = 0; i <= 7; ++i) {
    result.push_back(count);
    if (count == 176) {
      count += 4;
      result.push_back(count);
      continue;
    }
    for (int j = 0; j < 4; ++j) {
      count += 4;
      result.push_back(count);
    }
  }
  count = 252;
  while (result.size() <= 256) result.push_back(count);

  return result;
}

/**
 * Will generate the green components of the fire palette
 **/
std::vector<uint8_t> generateGreens() {
  std::vector<uint8_t> result;

  // Add 20 zeros
  for (int i = 0; i < 40; ++i) {
    result.push_back(0);
  }

  // Add numbers in the pattern 4, 8, 12, ..., 252
  for (int i = 4; i <= 252; i += 4) {
    result.push_back(i);
    result.push_back(i);
  }

  int count = 252;
  while (result.size() <= 256) result.push_back(count);

  return result;
}

/**
 * Will generate the blue components of the fire palette
 **/
std::vector<uint8_t> generateBlues() {
  std::vector<uint8_t> result;

  result.push_back(0);

  for (int i = 0; i < 36; ++i) {
    result.push_back(i * 2);
  }

  for (int i = 18; i >= 0; i--) {
    result.push_back(i);
    result.push_back(i);
  }

  int i = 0;

  while (result.size() < 144) result.push_back(i++);

  for (int i = 0; i < 54; ++i) {
    result.push_back(i);
    result.push_back(i);
  }

  int count = 252;
  while (result.size() <= 256) result.push_back(count);

  return result;
}

/**
 * Will generate the fire palette
 **/
void generateFirePalette(SDL_Color* colours, int count) {
  auto reds = generateReds();
  auto greens = generateGreens();
  auto blues = generateBlues();

  for (int i = 0; i < count; i++) {
    colours[i] = SDL_Color{reds[i], greens[i], blues[i]};
  }
}

/**
 * This routine will be called when the application initializes the screen for the effects
 **/
void initializeScreen(Uint8* screen)
{
  // reset the screen to 0
  memset(screen, 0, SCREENSIZE_X * SCREENSIZE_Y);

  // Initialize the last row of the screen with random values (0 or 255).
  for (int x = XMIN; x <= XMAX; ++x) 
  {
    screen[YMAX * SCREENSIZE_X + x] = rand() % 255;
  }
}

/**
 * Update screen is called for every frame that will be presented.
 **/
void updateScreen(Uint8* screen, int& cycles)
{
  // Adding another random row at the bottom of the screen
  for (int x = XMIN; x <= XMAX; ++x) 
  {
    switch (rand() % 10) 
    {
    case 0: case 2: case 4:
      screen[YMAX * SCREENSIZE_X + x] = 0;
      break;
    case 1: case 3: case 5: case 6: case 7:
      screen[YMAX * SCREENSIZE_X + x] = rand() % 255; 
      break;
    case 8: case 9:
      screen[YMAX * SCREENSIZE_X + x] = 255;
      break;
    }
  }

  ++cycles;
  if (cycles == FIRE_HEIGHT + 1)
  {
    // If we have reached the desired height we apply the Conway's Game of Life rules.
    cycles = 0;
    for (int x = XMIN; x < XMAX; ++x)
    {
      for (int y = YMIN + 1; y < YMAX; ++y)
      {
        int neighbours = (screen[(y - 1) * SCREENSIZE_X + x] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[(y + 1) * SCREENSIZE_X + x] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[y * SCREENSIZE_X + (x - 1)] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[y * SCREENSIZE_X + (x + 1)] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[(y - 1) * SCREENSIZE_X + (x - 1)] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[(y - 1) * SCREENSIZE_X + (x + 1)] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[(y + 1) * SCREENSIZE_X + (x - 1)] > CONWAY_DIFFERENTIATOR ? 0 : 1) +
          (screen[(y + 1) * SCREENSIZE_X + (x + 1)] > CONWAY_DIFFERENTIATOR ? 0 : 1);

        if (screen[y * SCREENSIZE_X + x] < CONWAY_DIFFERENTIATOR) 
        {
          // Cell is alive
          if (neighbours < 2 || neighbours > 3) 
          {
            int total = 0;
            int tdivctr = 1;
            total += screen[(y + 1) * SCREENSIZE_X + (x - 1)];
            if (rand() % 10 < 2) 
            {
              total += screen[(y + 1) * SCREENSIZE_X + x];
              tdivctr++;
            }
            if (rand() % 10 < 8) 
            {
              total += screen[(y + 1) * SCREENSIZE_X + (x + 1)];
              tdivctr++;
            }
            if (rand() % 10 < 5) 
            {
              total += screen[y * SCREENSIZE_X + (x - 1)];
              tdivctr++;
            }
            if (rand() % 10 < 7) 
            {
              total += screen[y * SCREENSIZE_X + x];
              tdivctr++;
            }
            if (rand() % 10 < 5) 
            {
              total += screen[y * SCREENSIZE_X + (x + 1)];
              tdivctr++;
            }
            Uint8 a = static_cast<Uint8>( total / (tdivctr + (rand() % 10 < 2 ? 1 : 0)));
            screen[y * SCREENSIZE_X + x] = a;  // Cell dies
          }
        } 
        else 
        {
          // Cell is dead
          if (neighbours == 3) 
          {
            screen[y * SCREENSIZE_X + x] = 255;  // Cell becomes alive
          }
        }
      }
    }
  }

  // And here let's do a heavily randomized fire routine
  for (int x = XMIN; x <= XMAX; x++) 
  {
    for (int y = YMIN; y < YMAX; y++) 
    {
      int total = 0;
      int tdivctr = 1;
      total += screen[(y + 1) * SCREENSIZE_X + (x - 1)];
      if (rand() % 10 < 2) 
      {
        total += screen[(y + 1) * SCREENSIZE_X + x];
        tdivctr++;
      }
      if (rand() % 10 < 8) 
      {
        total += screen[(y + 1) * SCREENSIZE_X + (x + 1)];
        tdivctr++;
      }
      if (rand() % 10 < 5) 
      {
        total += screen[y * SCREENSIZE_X + (x - 1)];
        tdivctr++;
      }
      if (rand() % 10 < 7) 
      {
        total += screen[y * SCREENSIZE_X + x];
        tdivctr++;
      }
      if (rand() % 10 < 5) 
      {
        total += screen[y * SCREENSIZE_X + (x + 1)];
        tdivctr++;
      }
      Uint8 a = static_cast<Uint8>( total / tdivctr );

      screen[y * SCREENSIZE_X + x] = a;
      if (rand() % 10 < 5) screen[y * SCREENSIZE_X + (x - 1)] = a;
      if (rand() % 10 < 5) screen[y * SCREENSIZE_X + (x + 1)] = a;
      if (rand() % 10 < 5) screen[(y - 1) * SCREENSIZE_X + x] = a;
      if (rand() % 10 < 5) screen[(y - 2) * SCREENSIZE_X + x] = a;

      if(rand() % 256 == 15 )
      {
          int rx = x - rand() % SCREENSIZE_X;
          int ry = y - rand() % SCREENSIZE_Y  ;
          Uint8 colAt = screen[ry * SCREENSIZE_X + rx];
          if(colAt >= 16)
          {
              screen[ry * SCREENSIZE_X + rx] = rand() % 255;
          }
      }

    }
  }
}

/**
 * Main entry point
 **/
int main() 
{

  // generic initialization
  srand(static_cast<unsigned int>(time(nullptr)));
  int cycles = 0;                               // The current iteration
  bool exitRequest = false;                     // Did we press the Close button on the window?

  // Initialize SDL, for now we use only the Video subsystem
  SDL_Init(SDL_INIT_VIDEO);

  // Create a window for the specified size
  SDL_Window* window = SDL_CreateWindow(
                         "Fire with Conway",                           // Title of the window
                         SDL_WINDOWPOS_CENTERED,                       // Center horizontally on current screen
                         SDL_WINDOWPOS_CENTERED,                       // Center vertically on current screen
                         SCREENSIZE_X, SCREENSIZE_Y,                   // The size of the screen
                         SDL_WINDOW_ALLOW_HIGHDPI                      // Not that necessary, use it if you have large screen
                       );
  if (!window) {
    std::cerr << "Cannot create window:" <<  SDL_GetError() << std::endl;
    exit(1);
  }

  // Create a renderer for the given window
  SDL_Renderer* renderer = SDL_CreateRenderer(
                             window,                                       // The window which will present the content of the renderer
                             -1,                                           // Use the first driver available
                             0                                             // No specific flags are needed
                           );
  if (!renderer) 
  {
    std::cerr << "Cannot create renderer:"  <<  SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  // Create a surface to render to, which is closest to the original screen specs we need: 256 colours (8b it)
  SDL_Surface* surface = SDL_CreateRGBSurface(
                           0,                                            // flag - not used
                           SCREENSIZE_X, SCREENSIZE_Y,                   // the size of the surface
                           8,                                            // 8 bit depth, we will get a palette allocated
                           0, 0, 0, 10                                   // the masks, they are not used for 8 bit surface
                         );
  if (!surface) 
  {
    std::cerr << "Cannot create surface:" <<  SDL_GetError() << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  // This will be the actual screen on which we perform the drawing. We use the SDL type Uint8, and initialize it to 0
  Uint8* screen = new Uint8[SCREENSIZE_X * SCREENSIZE_Y + 1];
  initializeScreen(screen);

  // The palette that will be used for this scene
  SDL_Color colours[256] = {0};
  generateFirePalette(colours, 255);
  SDL_SetPaletteColors(surface->format->palette, colours, 0, 255);
  // The texture that will be shown on the screen
  SDL_Texture* texture = nullptr;

  // The main loop of the application
  while (!exitRequest)
  {
    SDL_Event e;

    // check if we need to exit
    while (SDL_PollEvent(&e) > 0)
    {
      switch (e.type) {
      case SDL_QUIT:
        exitRequest = true;
      }
    }
    if (exitRequest)
    {
      break;
    }

    // let's calculate the next frame of the effect and draw it on the virtual screen
    updateScreen(screen, cycles);

    // fetching the pixel data of the surface
    uint8_t* offscreen = (uint8_t*)surface->pixels;

    // Copying the work screen over to the surface
    memcpy(offscreen, screen, SCREENSIZE_X * SCREENSIZE_Y);

    // Creating the texture from the given surface
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    // And drawing the texture to the given renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Showing the renderer on the screen
    SDL_RenderPresent(renderer);

    // And freeing the texture to not to have a memory leak
    SDL_DestroyTexture(texture);
  }

  // Releasing the allocated resources
  delete[] screen;
  SDL_FreeSurface(surface);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
