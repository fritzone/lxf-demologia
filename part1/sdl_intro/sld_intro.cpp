#include <SDL2/SDL.h>


int main(int argc, char* args[]) {

  atexit(SDL_Quit);

  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create a window
  SDL_Window* window =
      SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Create a renderer
  SDL_Renderer* renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // Just for fun, make the application to be fullscreen
  SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

  // Initial position of the rectangle
  int xPos = 100;
  int yPos = 100;

  // Main event loop
  bool quit = false;
  SDL_Event e;

  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    // Update position (move diagonally)
    xPos += 2;
    yPos += 2;

    // Clear the renderer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Set color to black
    SDL_RenderClear(renderer);

    // Draw the moving rectangle
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Set color to red
    SDL_Rect pixelRect = {xPos, yPos, 10, 10};  // Rectangle representing the moving pixel
    SDL_RenderFillRect(renderer, &pixelRect);

    // Present the renderer
    SDL_RenderPresent(renderer);

    // Add a delay to control the speed of the movement
    SDL_Delay(16);  // Adjust as needed for desired speed
  }

  // Clean up and exit
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  return 0;
}
