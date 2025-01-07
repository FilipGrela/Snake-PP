extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

class Board {
public:
    Board(int screenWidth, int screenHeight, int width, int height);
    void render(SDL_Surface* surface);

private:
    int width, height;
    SDL_Rect boardRect;
};

Board::Board( int screenWidth, int screenHeight, int width, int height) : width(width), height(height) {
    boardRect = {(screenWidth - width) / 2, (screenHeight - height) / 2, width, height};
}

void Board::render(SDL_Surface* surface) {
    Uint32 green = SDL_MapRGB(surface->format, 0, 255, 0); // Green color
    SDL_FillRect(surface, &boardRect, green);
}
