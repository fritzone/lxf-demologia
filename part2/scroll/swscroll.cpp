#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>

const int SCREENSIZE_X = 640;  // Adjust accordingly to your screen
const int SCREENSIZE_Y = 400;
const int XMIN = 0;
const int XMAX = SCREENSIZE_X - 1;
const int YMIN = 2;
const int YMAX = SCREENSIZE_Y - 1;
const double RANDMONESS = 1.7;  // Play with this for more fun. The higher the value, the more pixelated the cloud is
const double MAXIMUM_RANDOM = static_cast<double>(RAND_MAX);

struct Star {
    int x;
    int y;
};


// Function to split a string into a vector of integers
template<class T=uint8_t>
std::vector<T> split(const std::string& s, char delimiter) {
    std::vector<T> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(std::stoi(token));
    }
    return tokens;
}

std::vector<Star> generateRandomStars(int numStars, int screenWidth, int screenHeight) {
    std::vector<Star> stars;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> xDistribution(0, screenWidth - 1);
    std::uniform_int_distribution<int> yDistribution(0, screenHeight - 1);

    for (int i = 0; i < numStars; ++i) {
        Star star;
        star.x = xDistribution(gen);
        star.y = yDistribution(gen);
        stars.push_back(star);
    }

    return stars;
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
 **/
void starfield(Uint8* screen, const std::vector<Star> stars)
{
  for(const auto& s : stars)
  {
    if(getPixel(s.x, s.y, screen) == 0 || getPixel(s.x, s.y, screen) == 153)
    {
      putPixel(s.x, s.y, 255, screen);
    }
  }
}


bool loadCustomImage(const std::string& filename, std::vector<uint8_t>& palette, std::vector<uint8_t>& imageData, unsigned& width, unsigned& height) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return false;
    }

    std::string imageSize;
    std::getline(inFile, imageSize);
    std::vector<int> sizeValues = split<int>(imageSize, 'x');
    width = sizeValues[0];
    height = sizeValues[1];

    palette.clear();
    std::string colorLine;
    while (std::getline(inFile, colorLine) && !colorLine.empty()) {
        std::vector<uint8_t> colorValues = split(colorLine, ' ');
        palette.insert(palette.end(), colorValues.begin(), colorValues.end());
    }

    imageData.clear();
    std::string rowLine;
    while (std::getline(inFile, rowLine)) {
        std::vector<uint8_t> rowValues = split(rowLine, ' ');
        imageData.insert(imageData.end(), rowValues.begin(), rowValues.end());
    }

    inFile.close();
    return true;
}


// Function to scale array length and interpolate content based on a percentage
std::vector<uint8_t> scaleArray(const uint8_t* inputArray, size_t originalLength, double percentage) {
    // Check for valid percentage
    if (percentage < 0.0 || percentage > 100.0) {
        return std::vector<uint8_t>(inputArray, inputArray + originalLength);
    }

    // Calculate the new length based on the percentage
    size_t newLength = static_cast<size_t>(originalLength * (percentage / 100.0));

    // Create a new vector to store the scaled values
    std::vector<uint8_t> scaledArray(newLength);

    // Calculate interpolation step
    double step = static_cast<double>(originalLength - 1) / static_cast<double>(newLength - 1);

    // Interpolate values for the elements in the new array
    for (size_t i = 0; i < newLength; ++i) {
        double index = i * step;
        size_t lowIndex = static_cast<size_t>(index);
        if(lowIndex > originalLength) lowIndex = 0;
        size_t highIndex = std::min(lowIndex + 1, originalLength - 1);
        double fraction = index - lowIndex;

        // Linear interpolation
        scaledArray[i] = static_cast<uint8_t>((1.0 - fraction) * inputArray[lowIndex] + fraction * inputArray[highIndex]);
    }

    return scaledArray;
}


int main() {
  srand(static_cast<unsigned>(time(nullptr)));

  int w = SCREENSIZE_X;
  int h = SCREENSIZE_Y;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow("Star Wars Scroll", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, w, h, 0);



  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_Surface* surface =
      SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
  if (surface == NULL) {
    std::cerr << "Cannot create surface" << std::endl;
    exit(1);
  }

    std::vector<uint8_t> palette;
    std::vector<uint8_t> imageData;
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
    colours[0] = {0, 0, 0, 0};
    colours[255] = {255, 255, 255, 0};

    SDL_SetPaletteColors(surface->format->palette, colours, 0, palette.size() / 4);


  const int screenWidth = SCREENSIZE_X;
  const int screenHeight = SCREENSIZE_Y;

  Uint8* screen = new Uint8[screenWidth * screenHeight + 1];
  memset(screen, 0, screenWidth * screenHeight + 1);

  SDL_SetPaletteColors(surface->format->palette, colours, 0, 256);
  srand(static_cast<unsigned int>(time(nullptr)));  

  Uint8* textBuffer = new Uint8[screenWidth * screenHeight + 1];
  memcpy(textBuffer, imageData.data(), SCREENSIZE_X*SCREENSIZE_Y);

  // generate the starfield
  std::vector<Star> stars = generateRandomStars(1024, screenWidth, screenHeight);

  //SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  SDL_Texture* texture = nullptr;
  int currentRow = YMAX - 1;
  int textureEndRow = 1;
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

    static Uint8 row[SCREENSIZE_X] = {0}; 
    double beginScale = 100.0 - static_cast<double>(textureEndRow)/4.0  + 1.0;
    for(int cr=0; cr<=textureEndRow; cr++)
    {
      memset(row, 0, SCREENSIZE_X);
      if(beginScale < 0) beginScale = 0;
      auto t = scaleArray(textBuffer + screenWidth * cr, screenWidth, beginScale);
      if(cr % 4 == 0) beginScale += 1.0;
      for(size_t j=0; j<t.size(); j++) row[SCREENSIZE_X / 2 - t.size()/2 + j] = t[j];
      memcpy(screen + currentRow * screenWidth +  screenWidth * cr, row, SCREENSIZE_X);
    }

    textureEndRow ++;
    currentRow --;

    // starfield
    starfield(screen, stars);


    if(textureEndRow == SCREENSIZE_Y)
    {
          delete[] screen;
          SDL_FreeSurface(surface);
          SDL_DestroyRenderer(renderer);
          SDL_DestroyWindow(window);
          SDL_Quit();
          return EXIT_SUCCESS;

    }

    uint8_t* offscreen = (uint8_t*)surface->pixels;
    memcpy(offscreen, screen, screenWidth * screenHeight);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(texture);

    SDL_Delay(100);
  }
}
