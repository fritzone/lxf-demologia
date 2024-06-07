#include <SDL2/SDL.h>
#include <SDL2/SDL.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

const int SCREENSIZE_X = 640;
const int SCREENSIZE_Y = 480;
const int XMIN = 0;
const int XMAX = SCREENSIZE_X - 1;
const int YMIN = 2;
const int YMAX = SCREENSIZE_Y - 1;

void putPixel(int x, int y, Uint8 c, Uint8* screen) 
{
  screen[SCREENSIZE_X * y + x] = c; 
}

Uint8 getPixel(int x, int y, Uint8* screen) 
{ 
  return screen[SCREENSIZE_X * y + x]; 
}

void generatePalette(SDL_Color* c, size_t cnt)
{
  // Add your palette generation logic here
}

void initializeScreen(Uint8* screen)
{
  memset(screen, 0, SCREENSIZE_X * SCREENSIZE_Y);
}

void updateScreen(Uint8* screen, const std::vector<int>& imageData, unsigned width, unsigned height) 
{
    // Copy the imageData into the screen array
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            int index = y * width + x;
            putPixel(x, y, static_cast<Uint8>(imageData[index]), screen);
        }
    }
}

// Function to split a string into a vector of integers
std::vector<int> split(const std::string& s, char delimiter) {
    std::vector<int> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(std::stoi(token));
    }
    return tokens;
}

bool loadCustomImage(const std::string& filename, std::vector<int>& palette, std::vector<int>& imageData, unsigned& width, unsigned& height) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return false;
    }

    std::string imageSize;
    std::getline(inFile, imageSize);
    std::vector<int> sizeValues = split(imageSize, 'x');
    width = sizeValues[0];
    height = sizeValues[1];

    palette.clear();
    std::string colorLine;
    while (std::getline(inFile, colorLine) && !colorLine.empty()) {
        std::vector<int> colorValues = split(colorLine, ' ');
        palette.insert(palette.end(), colorValues.begin(), colorValues.end());
    }

    imageData.clear();
    std::string rowLine;
    while (std::getline(inFile, rowLine)) {
        std::vector<int> rowValues = split(rowLine, ' ');
        imageData.insert(imageData.end(), rowValues.begin(), rowValues.end());
    }

    inFile.close();
    return true;
}

int main() 
{
    srand(static_cast<unsigned int>(time(nullptr)));
    bool exitRequest = false;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
                         "SDL2",
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
    initializeScreen(screen);

    std::vector<int> palette;
    std::vector<int> imageData;
    unsigned width, height;

    if (!loadCustomImage("output_image.custom", palette, imageData, width, height)) {
        SDL_Quit();
        return 1;
    }

    SDL_Color colours[256] = {0};
    for (size_t i = 0; i < palette.size(); i += 4) {
        colours[i / 4].r = palette[i];
        colours[i / 4].g = palette[i + 1];
        colours[i / 4].b = palette[i + 2];
        colours[i / 4].a = palette[i + 3];
    }

    SDL_SetPaletteColors(surface->format->palette, colours, 0, palette.size() / 4);

    SDL_Texture* texture = nullptr;

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

        updateScreen(screen, imageData, width, height);

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
