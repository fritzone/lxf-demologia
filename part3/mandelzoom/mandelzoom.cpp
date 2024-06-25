#include <SDL2/SDL.h>
#include <SDL2/SDL.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

const int SCREENSIZE_X = 320;
const int SCREENSIZE_Y = 200;

Uint8 MANDELBROT_MAX_ITERATIONS = 255; // The maximum value, after which we consider the point "escaped". Handy that there are 256 colours.
double MANDELBROT_THRESHOLD = 4.0; // This is 4.0, changing it has no real effect on the code.
double centerX = -0.743023954; // Center point on the real axis of the fractal, the middle of the Seahorse valley.
double centerY = -0.129123012;  // Center point on the imaginary axis

void putPixel(int x, int y, Uint8 c, Uint8* screen) 
{
  screen[SCREENSIZE_X * y + x] = c; 
}

Uint8 getPixel(int x, int y, Uint8* screen) 
{ 
  return screen[SCREENSIZE_X * y + x]; 
}

void initializeScreen(Uint8* screen)
{
  memset(screen, 0, SCREENSIZE_X * SCREENSIZE_Y);
}


void updateScreen(Uint8* screen, double zoomFactor, double centerX, double centerY) 
{
    for (int x = 0; x < SCREENSIZE_X; x++) {
        for (int y = 0; y < SCREENSIZE_Y; y++) {
            double zx = (static_cast<double>(x) - SCREENSIZE_X / 2) / (zoomFactor * SCREENSIZE_X) + centerX;
            double zy = (static_cast<double>(y) - SCREENSIZE_Y / 2) / (zoomFactor * SCREENSIZE_Y) + centerY;

            double cx = zx;
            double cy = zy;
            double zx2 = zx * zx;
            double zy2 = zy * zy;

            Uint8 colour = 0;
            while (zx2 + zy2 < MANDELBROT_THRESHOLD && colour < MANDELBROT_MAX_ITERATIONS) 
            {

                zy = 2.0 * zx * zy + cy;
                zx = zx2 - zy2 + cx;

                zx2 = zx * zx;
                zy2 = zy * zy;


                colour++;
            }
            
            putPixel(x, y, colour, screen);
        }
    }
}


/**
 * Will generate the palette for the fractal. If you want to obtain the same colours as from the article,
 * please use the colour cycle palette from episode 1.
 **/
void generatePalette(SDL_Color* colours) 
{
    for (int i = 0; i < 256; ++i) 
    {
        Uint8 red = static_cast<Uint8>((i * 2) % 256);
        Uint8 green = static_cast<Uint8>((i * 5) % 256);
        Uint8 blue = static_cast<Uint8>((i * 7) % 256);

        colours[i] = { red, green, blue, 255 };
    }
    colours[255] = { 0, 0, 0, 255 };
}

int main() 
{
    bool exitRequest = false;
  SDL_Color colours[256];
  generatePalette(colours);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
                         "Mandel Zoomer",
                         SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED,
                         SCREENSIZE_X, SCREENSIZE_Y,
                         SDL_WINDOW_ALLOW_HIGHDPI
                       );
    if (!window) 
    {
        std::cerr << "Cannot create window:" <<  SDL_GetError() << std::endl;
        exit(1);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
                             window,
                             -1,
                             0
                           );
    if (!renderer) 
    {
        std::cerr << "Cannot create renderer:"  <<  SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    SDL_Surface* surface = SDL_CreateRGBSurface(
                           0,
                           SCREENSIZE_X, SCREENSIZE_Y,
                           8,
                           0, 0, 0, 0
                         );
    if (!surface) {
        std::cerr << "Cannot create surface:" <<  SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    Uint8* screen = new Uint8[SCREENSIZE_X * SCREENSIZE_Y + 1];
      SDL_SetPaletteColors(surface->format->palette, colours, 0, 256);

    initializeScreen(screen);


    SDL_Texture* texture = nullptr;

    // Make the windows fullscreen if you wish
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    double zoom = 0.0;

    while (!exitRequest)
    {
        SDL_Event e;

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

        updateScreen(screen, zoom, centerX, centerY);

        zoom += 1;              // Experiments here, with various other values are welcome, such as to zoom in faster, more, move left/right in the fractal.
        centerY -= 0.00001;     // With these values we zoom into a slightly rotated baby mandel, see for yourself what you can discover.
        centerX -= 0.00000001;

        if(zoom >= 1024) 
        {
            exitRequest = true;
        }

        uint8_t* offscreen = (uint8_t*)surface->pixels;
        memcpy(offscreen, screen, SCREENSIZE_X * SCREENSIZE_Y);

        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);
    }

    delete[] screen;
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
