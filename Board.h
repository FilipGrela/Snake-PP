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

/**
 * @brief Constructor for the Board class.
 * @param screenWidth The width of the screen.
 * @param screenHeight The height of the screen.
 * @param width The width of the board in units.
 * @param height The height of the board in units.
 * @param unitSize The size of each unit on the board.
 */
Board::Board(int screenWidth, int screenHeight, int width, int height, int unitSize) : width(width), height(height), unitSize(unitSize) {
	int heightPx = height * unitSize;
	int widthPx = width * unitSize;
	boardRect = { (screenWidth - widthPx) / 2, (screenHeight - heightPx) / 2, widthPx, heightPx };
}

/**
 * @brief Renders the board on the given surface.
 * @param surface The SDL_Surface to render the board on.
 */
void Board::render(SDL_Surface* surface) {
	Uint32 green = SDL_MapRGB(surface->format, 0, 255, 0); // Green color
	Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255); // White color for outline

	// Draw the outline with white color
	SDL_Rect outlineRect = { boardRect.x - 1, boardRect.y - 1, boardRect.w + 2, boardRect.h + 2 };
	SDL_FillRect(surface, &outlineRect, white);
	// Fill the board with green color
	SDL_FillRect(surface, &boardRect, green);

	

}
