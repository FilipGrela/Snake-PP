extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

class Board {
public:
    Board(int screenWidth, int screenHeight, int width, int height, int unitSize);
    void render(SDL_Surface* surface);

private:
	int unitSize;
    int width, height;
    SDL_Rect boardRect;
};

Board::Board( int screenWidth, int screenHeight, int width, int height, int unitSize) : width(width), height(height), unitSize(unitSize){
	int height_px = height * unitSize;
	int width_px = width * unitSize;
    boardRect = {(screenWidth - width_px) / 2, (screenHeight - height_px) / 2, width_px, height_px };
}

void Board::render(SDL_Surface* surface) {
    Uint32 green = SDL_MapRGB(surface->format, 0, 255, 0); // Green color
    SDL_FillRect(surface, &boardRect, green);
}
