#include <SDL2/SDL.h>

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

const int SCREENSIZE_X = 1024;
const int SCREENSIZE_Y = 768;
const int XMIN = 0;
const int XMAX = SCREENSIZE_X - 1;
const int YMIN = 2;
const int YMAX = SCREENSIZE_Y - 1;
const int SPEED = 1;
const int TEXTURE_SIZE = 256;
const int TUNNEL_END_SIZE = 100;

void putPixel(int x, int y, Uint8 c, Uint8* screen) {
  screen[SCREENSIZE_X * y + x] = c;
}

Uint8 getPixel(int x, int y, Uint8* screen) {
  return screen[SCREENSIZE_X * y + x];
}

void initializeScreen(Uint8* screen) {
  memset(screen, 0, SCREENSIZE_X * SCREENSIZE_Y);
}

bool isPointInsideCircle(int pointX, int pointY, int circleCenterX,
                         int circleCenterY, int circleRadius) {
  // Calculate the distance between the point and the center of the circle
  int distance =
      sqrt(pow(pointX - circleCenterX, 2) + pow(pointY - circleCenterY, 2));

  // Check if the distance is less than the radius of the circle
  return distance < circleRadius;
}

void updateScreen(Uint8* screen, const std::vector<int>& imageData) {
  static double animation_rotation = 0;
  static double animation_zoom = 0;

  static const int TUNNEL_CENTRE_X = SCREENSIZE_X / 2;
  static const int TUNNEL_CENTRE_Y = SCREENSIZE_Y / 2;
  static const int DISTORTION = 64;
  static const double MULTIPLICATOR = 2.5;

  animation_rotation += 0.01;
  animation_zoom += 0.01;

  for (int y = 0; y < SCREENSIZE_Y; y++) {
    for (int x = 0; x < SCREENSIZE_X; x++) {
      if (!isPointInsideCircle(x, y, TUNNEL_CENTRE_X, TUNNEL_CENTRE_Y, TUNNEL_END_SIZE))
      {
        int distance = static_cast<int>(DISTORTION * TEXTURE_SIZE / log(pow(x - TUNNEL_CENTRE_X, 2) + pow(y - TUNNEL_CENTRE_Y, 2)) );
        int angle = static_cast<int>(MULTIPLICATOR * TEXTURE_SIZE * atan2(x - TUNNEL_CENTRE_X, y - TUNNEL_CENTRE_Y) / M_PI );

        unsigned u = static_cast<unsigned>(distance + TEXTURE_SIZE * animation_zoom) % TEXTURE_SIZE;
        unsigned v = static_cast<unsigned>(angle    + TEXTURE_SIZE * animation_rotation) % TEXTURE_SIZE;

        Uint8 color = static_cast<Uint8>(imageData[u * TEXTURE_SIZE + v]);
        putPixel(x, y, color, screen);
      } 
      else 
      {
        putPixel(x, y, 0, screen);
      }
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

bool loadCustomImage(const std::string& filename, std::vector<int>& palette,
                     std::vector<int>& imageData, unsigned& width,
                     unsigned& height) {
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

int main() {
  bool exitRequest = false;

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* window =
      SDL_CreateWindow("Tunnel", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       SCREENSIZE_X, SCREENSIZE_Y, SDL_WINDOW_ALLOW_HIGHDPI);
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
  unsigned width, height;

  if (!loadCustomImage("output_image.custom", palette, imageData, width,
                       height)) {
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

  SDL_SetPaletteColors(surface->format->palette, colours, 0,
                       palette.size() / 4);

  SDL_Texture* texture = nullptr;

  //    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

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

    SDL_Delay(10);
  }

  delete[] screen;
  SDL_FreeSurface(surface);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
