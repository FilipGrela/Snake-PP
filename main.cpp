#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

#include"Snake.h"
#include"Board.h"


extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	820
#define SCREEN_HEIGHT	640
#define INFO_SCREN_HEIGHT 36

#define SCOREBOARD_PATH "scoreboard.txt"
#define SCOREBOARD_SIZE 10

#define BOARD_WIDTH_UNITS 30
#define BOARD_HEIGHT_USNITS 20

#define UNIT_SIZE 20

#define SNAKE_SPEED 8 // units per second
#define SNAKE_INITIAL_LENGTH 3 // initial length of the snake
#define SNAKE_MINIMUM_SPEED 6 // minimum speed of the snake
#define SNAKE_SPEEDUP 10 // time after which the snake speeds up in seconds
#define SNAKE_SPEEDUP_FACTOR 1.2 // factor by which the snake speeds up

#define POWER_UP_TIME 5 // time after which the power up disappears in seconds
#define POWER_UP_PROBABILITY 50 // probability of a power up appearing
#define POWER_UP_SLOWDOWN_FACTOR 0.1 // factor by which the snake slows down when power up is eaten
#define POWER_UP_SHORTEN_UNITS 5 // number of units by which the snake shortens when power up is eaten



struct Food {
	int x, y;
	bool powerUp;
	int color;
};

struct GameData {
	int t1, t2, quit, rc, points;
	double delta, worldTime, snakeSpeedUnitsPerSeconnd;
	double powerUpActivationTime = NULL;
	bool snakeAlive;
	bool powerUpActive;

	int* scoresTabele = nullptr;

	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	Food food;
	Food foodPowerUp;
};



/**
 * @brief Returns a random number in the range (min to max).
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return A random number between min and max.
 */
int getRandomNumber(int min, int max) {
	return rand() % (max + 1 - min) + min;
}

/**
 * @brief Draws a text string on the screen at the specified position.
 * @param screen The SDL_Surface to draw on.
 * @param x The x-coordinate of the starting position.
 * @param y The y-coordinate of the starting position.
 * @param text The text string to draw.
 * @param charset The SDL_Surface containing the character set.
 */
void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


/**
 * @brief Draws a surface sprite on the screen at the specified position.
 * @param screen The SDL_Surface to draw on.
 * @param sprite The SDL_Surface to draw.
 * @param x The x-coordinate of the center of the sprite.
 * @param y The y-coordinate of the center of the sprite.
 */
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


/**
 * @brief Draws a single pixel on the surface at the specified position.
 * @param surface The SDL_Surface to draw on.
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The color of the pixel.
 */
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


/**
 * @brief Draws a vertical or horizontal line on the screen.
 * @param screen The SDL_Surface to draw on.
 * @param x The starting x-coordinate of the line.
 * @param y The starting y-coordinate of the line.
 * @param l The length of the line.
 * @param dx The x-direction (0 for vertical, 1 for horizontal).
 * @param dy The y-direction (1 for vertical, 0 for horizontal).
 * @param color The color of the line.
 */
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

/**
 * @brief Draws a rectangle of the specified size on the screen.
 * @param screen The SDL_Surface to draw on.
 * @param x The x-coordinate of the top-left corner of the rectangle.
 * @param y The y-coordinate of the top-left corner of the rectangle.
 * @param l The width of the rectangle.
 * @param k The height of the rectangle.
 * @param outlineColor The color of the rectangle outline.
 * @param fillColor The color of the rectangle fill.
 */
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

/**
 * @brief Places food at a random position on the board.
 * @param food The Food object to place.
 * @param powerUp Whether the food is a power-up.
 * @param format The SDL_PixelFormat for color mapping.
 * @param snake The Snake object to avoid placing food on.
 */
void placeFood(Food& food, bool powerUp, SDL_PixelFormat* format, Snake* snake) {
	int x, y;
	do {
		x = getRandomNumber(0, BOARD_WIDTH_UNITS - 1);
		y = getRandomNumber(0, BOARD_HEIGHT_USNITS - 1);
	} while (!snake->checkIfFreeCell(x, y));

	food = Food();
	food.x = x;
	food.y = y;
	food.powerUp = powerUp;
	food.color = powerUp ? SDL_MapRGB(format, 0xFF, 0x00, 0xFF) : SDL_MapRGB(format, 0xFF, 0x00, 0x00);
}

/**
 * @brief Handles the effects of eating a power-up.
 * @param gameData The GameData object containing game state.
 * @param snake The Snake object to apply effects to.
 */
void handlePowerUp(GameData* gameData, Snake* snake) {
	if (getRandomNumber(0, 1) == 0) {
		gameData->snakeSpeedUnitsPerSeconnd *= POWER_UP_SLOWDOWN_FACTOR;
		if (gameData->snakeSpeedUnitsPerSeconnd < SNAKE_MINIMUM_SPEED) {
			gameData->snakeSpeedUnitsPerSeconnd = SNAKE_MINIMUM_SPEED;
		}
	}
	else {
		if (SNAKE_INITIAL_LENGTH < snake->getLength() - POWER_UP_SHORTEN_UNITS) {
			snake->shorten(POWER_UP_SHORTEN_UNITS);
		}
		else {
			snake->shorten(snake->getLength() - SNAKE_INITIAL_LENGTH);
		}
	}
}

/**
 * @brief Checks for collisions between the snake and food.
 * @param snake The Snake object to check for collisions.
 * @param gameData The GameData object containing game state.
 */
void checkFoodCollision(Snake* snake, GameData* gameData) {
	Food foods[] = { gameData->food, gameData->foodPowerUp };
	for (Food& food : foods) {
		if (snake->getHead()->x == food.x && snake->getHead()->y == food.y) {
			if (!food.powerUp) {
				snake->grow();
				gameData->points += 10;
				placeFood(gameData->food, false, gameData->screen->format, snake);
				int probability = getRandomNumber(0, 100);
				if (probability <= POWER_UP_PROBABILITY && !gameData->powerUpActive) {
					gameData->powerUpActive = true;
					gameData->powerUpActivationTime = gameData->worldTime;
					placeFood(gameData->foodPowerUp, true, gameData->screen->format, snake);
				}
			}
			else if (gameData->powerUpActive) {
				gameData->points += 20;
				handlePowerUp(gameData, snake);
				gameData->powerUpActive = false;

			}
		}
	}
}

/**
 * @brief Moves the snake and checks for collisions with food.
 * @param snake The Snake object to move.
 * @param gameData The GameData object containing game state.
 */
void moveSnake(Snake* snake, GameData* gameData) {
	int headX = snake->getHead()->x;
	int headY = snake->getHead()->y;
	Snake::Directions nextDirection = snake->getNextDirection();

	// Check if the snake's head is at the left edge and moving left
	if (headX <= 0 && nextDirection == Snake::Left) {
		snake->changeDirection(
			(snake->checkIfFreeCell(headX, headY - 1) && headY != 0) ? Snake::Up : Snake::Down);
	}
	// Check if the snake's head is at the right edge and moving right
	else if (headX >= BOARD_WIDTH_UNITS - 1 && nextDirection == Snake::Right) {
		snake->changeDirection(
			(snake->checkIfFreeCell(headX, headY + 1) && headY != BOARD_HEIGHT_USNITS - 1) ? Snake::Down : Snake::Up);
	}
	// Check if the snake's head is at the top edge and moving up
	else if (headY <= 0 && nextDirection == Snake::Up) {
		snake->changeDirection(
			(snake->checkIfFreeCell(headX + 1, headY) && headX != BOARD_WIDTH_UNITS - 1) ? Snake::Right : Snake::Left);
	}
	// Check if the snake's head is at the bottom edge and moving down
	else if (headY >= BOARD_HEIGHT_USNITS - 1 && nextDirection == Snake::Down) {
		snake->changeDirection(
			(snake->checkIfFreeCell(headX - 1, headY) && headX != 0) ? Snake::Left : Snake::Right);
	}

	// Move the snake in the current direction
	snake->move();

	// Check for collisions with food
	checkFoodCollision(snake, gameData);
}



/**
 * @brief Draws game information on the screen.
 * @param screen The SDL_Surface to draw on.
 * @param color1 The color of the outline.
 * @param color2 The color of the fill.
 * @param points The current score.
 * @param worldTime The elapsed game time.
 * @param charset The SDL_Surface containing the character set.
 * @param scrtex The SDL_Texture to update.
 * @param renderer The SDL_Renderer to use.
 */
void drawInfo(SDL_Surface* screen, Uint32 color1, Uint32 color2, int points, double worldTime, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer) {
	// info text
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, INFO_SCREN_HEIGHT, color1, color2);
	char text[128];
	sprintf(text, "Czas trwania = %.1lf s, Punkty: %d", worldTime, points);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 8, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Esc - wyjscie, n - nowa gra, \032 \030 \033 \031 - sterowanie");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);

	sprintf(text, "Wymagania: 1-4, A, B, C, D, F");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 30, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

/**
 * @brief Creates a new Snake object.
 * @param pointsVerible A pointer to the score variable.
 * @return A pointer to the new Snake object.
 */
Snake* getNewSnake(int* pointsVerible) {
	int snakeOffsetX = (SCREEN_WIDTH - BOARD_WIDTH_UNITS * UNIT_SIZE) / 2;
	int snakeOffsetY = (SCREEN_HEIGHT - BOARD_HEIGHT_USNITS * UNIT_SIZE) / 2;

	return new Snake(SNAKE_INITIAL_LENGTH, BOARD_WIDTH_UNITS / 2, BOARD_HEIGHT_USNITS / 2, UNIT_SIZE, snakeOffsetX, snakeOffsetY, pointsVerible);
}

/**
 * @brief Draws the progress bar for the power-up duration.
 * @param gameData The GameData object containing game state.
 */
void drawProgrsssBar(GameData* gameData) {
	SDL_Surface* screen = gameData->screen;
	int boardPaddingX = (SCREEN_WIDTH - BOARD_WIDTH_UNITS * UNIT_SIZE) / 2;
	int boardPaddingY = (SCREEN_HEIGHT - BOARD_HEIGHT_USNITS * UNIT_SIZE) / 2;
	int barWidth = SCREEN_WIDTH - 2 * boardPaddingX;
	int barHeight = 10;
	int barX = boardPaddingX;
	int barY = SCREEN_HEIGHT - INFO_SCREN_HEIGHT - barHeight - 4;
	double progress = (gameData->worldTime - gameData->powerUpActivationTime) / POWER_UP_TIME;
	if (progress > 1) {
		progress = 1;
	}
	DrawRectangle(screen, barX, barY, barWidth, barHeight, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00), SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00));
	DrawRectangle(screen, barX, barY, barWidth * progress, barHeight, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0x00), SDL_MapRGB(screen->format, 0xFF, 0xFF, 0x00));
}

/**
 * @brief Draws the food on the screen.
 * @param screen The SDL_Surface to draw on.
 * @param food The Food object to draw.
 */
void drawFood(SDL_Surface* screen, Food food) {
	// Calculate board padding
	int boardPaddingX = (SCREEN_WIDTH - BOARD_WIDTH_UNITS * UNIT_SIZE) / 2;
	int boardPaddingY = (SCREEN_HEIGHT - BOARD_HEIGHT_USNITS * UNIT_SIZE) / 2;

	// Calculate the center of the food
	int centerX = food.x * UNIT_SIZE + boardPaddingX + UNIT_SIZE / 2;
	int centerY = food.y * UNIT_SIZE + boardPaddingY + UNIT_SIZE / 2;
	int radius = UNIT_SIZE * 0.8 / 2;

	// Draw the food as a circle
	for (int w = 0; w < UNIT_SIZE; w++) {
		for (int h = 0; h < UNIT_SIZE; h++) {
			int pixelX = food.x * UNIT_SIZE + boardPaddingX + w;
			int pixelY = food.y * UNIT_SIZE + boardPaddingY + h;
			int dx = centerX - pixelX;
			int dy = centerY - pixelY;
			if ((dx * dx + dy * dy) <= (radius * radius)) {
				DrawPixel(screen, pixelX, pixelY, food.color);
			}
		}
	}
}

/**
 * @brief Draws the scoreboard on the screen.
 * @param gameData The GameData object containing game state.
 */
void drawScoreboard(GameData* gameData) {
	SDL_Surface* screen = gameData->screen;
	int x = 10; // X position of the scoreboard
	int y = 50; // Y position of the scoreboard
	int lineHeight = 20; // Height of each line
	int padding = 5; // Padding between lines
	int headerHeight = 30; // Height of the header
	int width = 200; // Width of the scoreboard


	// Draw the column headers
	DrawString(screen, x + padding, y, "Rank", gameData->charset);
	DrawString(screen, x + padding + 50, y, "Score", gameData->charset);

	// Draw the scores
	y += lineHeight + padding;
	for (int i = 0; i < SCOREBOARD_SIZE; i++) {
		if (gameData->scoresTabele[i] >= 0) {
			char rankText[8];
			char scoreText[32];
			sprintf(rankText, "%d.", i + 1);
			sprintf(scoreText, "%d", gameData->scoresTabele[i]);
			DrawString(screen, x + padding, y, rankText, gameData->charset);
			DrawString(screen, x + padding + 50, y, scoreText, gameData->charset);
			y += lineHeight + padding;
		}
	}
}


/**
 * @brief Draws the game elements on the screen.
 * @param snake The Snake object to draw.
 * @param board The Board object to draw.
 * @param gameData The GameData object containing game state.
 */
void draw(Snake* snake, Board* board, GameData* gameData) {
	int czarny = SDL_MapRGB(gameData->screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gameData->screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gameData->screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gameData->screen->format, 0x11, 0x11, 0xCC);

	SDL_FillRect(gameData->screen, NULL, czarny);
	board->render(gameData->screen);
	snake->drawSnake(gameData->screen, niebieski);

	drawFood(gameData->screen, gameData->food);
	if (gameData->powerUpActive) {
		drawFood(gameData->screen, gameData->foodPowerUp);
		drawProgrsssBar(gameData);
	}


	if (!gameData->snakeAlive) {
		DrawString(gameData->screen, SCREEN_WIDTH / 2 - 8 * 4, SCREEN_HEIGHT / 2 - 8, "GAME OVER", gameData->charset);
	}

	drawScoreboard(gameData);

	drawInfo(gameData->screen, niebieski, czerwony, gameData->points, gameData->worldTime, gameData->charset, gameData->scrtex, gameData->renderer);
}

/**
 * @brief Restarts the game by creating a new snake and resetting game variables.
 * @param snake A reference to the pointer of the Snake object to be reset.
 * @param snakeSpeedUnitsPerSecond The speed of the snake in units per second.
 * @param points The current score.
 * @param snakeAlive The state of the snake (alive or dead).
 * @param timeElapsed The elapsed game time.
 */
void restartGame(Snake*& snake, double& snakeSpeedUnitsPerSecond, int& points, bool& snakeAlive, double& timeElapsed) {
    delete snake;
    snake = getNewSnake(&points);
    timeElapsed = 0;
    snakeSpeedUnitsPerSecond = SNAKE_SPEED;
    snakeAlive = true;
}



/**
 * @brief Function sorts an array in ascending order, treating negative numbers as the largest value.
 * @param array A pointer to the integer array to be sorted.
 * @param n The number of elements in the array.
 */
void bubbleSort(int* array, int n) {
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			// Check if both elements are different from -1
			if (array[j] != -1 && array[j + 1] != -1 && array[j] < array[j + 1]) {
				// Swap elements
				int temp = array[j];
				array[j] = array[j + 1];
				array[j + 1] = temp;
			}
			// If array[j] is -1, do not swap with array[j+1], even if array[j+1] > array[j]
			else if (array[j] == -1 && array[j + 1] != -1) {
				// Move -1 to the end of the array
				int temp = array[j];
				array[j] = array[j + 1];
				array[j + 1] = temp;
			}
		}
	}
}



/**
 * @brief Sorts an array and writes the sorted values to a file.
 * @param file_name The name of the file where the sorted array will be written.
 * @param array A pointer to the array to be sorted.
 * @param n The number of elements in the array.
 */
void sortedArrayToFile(const char* fileName, int* array, int n) {
	bubbleSort(array, n + 1);

	FILE* file = fopen(fileName, "w");
	if (!file) {
		return;
	}

	// Put sorted array to file
	for (int i = 0; i < n; i++) {
		fprintf(file, "%d\n", array[i]);
	}

	fclose(file);
}


/**
 * @brief Reads an array of integers from a file or creates a new file if it doesn't exist.
 * @param file_name The name of the file to read from or write to.
 * @param n The number of integers to read into the array.
 * @return A pointer to the array of integers read from the file or filled with -1 if the file is not found.
 */
int* readScores(const char* fileName, int n) {
	FILE* file = fopen(fileName, "r");
	int* array = (int*)malloc((n + 1) * sizeof(int));

	if (!array) {
		return nullptr;
	}

	if (file) {
		// File exists, read data
		int i = 0;
		while (i < n && fscanf(file, "%d", &array[i]) == 1) {
			i++;
		}

		// If not enough data read, fill remaining spaces -1
		for (int j = i; j < n; j++) {
			array[j] = -1;
		}

		// If it doesn't read all the data, save the missing data to a file
		if (i < n) {
			file = freopen(fileName, "w", file); // Reset file to write mode
			if (file) {
				for (int j = 0; j < n; j++) {
					fprintf(file, "%d\n", array[j]);
				}
			}
		}

		fclose(file);
		bubbleSort(array, n);
	}
	else {
		// File does not exist, create new one and fill array -1
		file = fopen(fileName, "w");
		if (!file) {
			free(array);
			return nullptr;
		}

		// Fill file with -1
		for (int i = 0; i < n; i++) {
			array[i] = -1;
			fprintf(file, "%d\n", array[i]);
		}

		fclose(file);
	}

	return array;
}

/**
 * @brief Initializes SDL and creates the main window and renderer.
 * @param gameData The GameData object containing game state.
 * @return 0 on success, 1 on failure.
 */
int initSDL(GameData& gameData) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	gameData.rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&gameData.window, &gameData.renderer);

	if (gameData.rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(gameData.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(gameData.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(gameData.window, "Snake Filip Grela 203850");


	gameData.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	gameData.scrtex = SDL_CreateTexture(gameData.renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// disabling mouse cursor visibility
	SDL_ShowCursor(SDL_DISABLE);

	// load image cs8x8.bmp
	gameData.charset = SDL_LoadBMP("./cs8x8.bmp");
	if (gameData.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(gameData.screen);
		SDL_DestroyTexture(gameData.scrtex);
		SDL_DestroyWindow(gameData.window);
		SDL_DestroyRenderer(gameData.renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(gameData.charset, true, 0x000000);

	gameData.t1 = SDL_GetTicks();

	gameData.quit = 0;
	gameData.worldTime = 0;
	return 0;
}

/**
 * @brief Handles user input and controls the game state.
 * @param gameData The GameData object containing game state.
 * @param snake The Snake object to control.
 */
void handleControls(GameData& gameData, Snake*& snake) {
	while (SDL_PollEvent(&gameData.event)) {
		switch (gameData.event.type) {
		case SDL_KEYDOWN:
			switch (gameData.event.key.keysym.sym) {
			case SDLK_ESCAPE:
				gameData.quit = 1;
				break;
			case SDLK_UP:
				if (snake->getHead()->y > 0) { // Check if not at the top edge
					snake->changeDirection(Snake::Up);
				}
				break;
			case SDLK_DOWN:
				if (snake->getHead()->y < BOARD_HEIGHT_USNITS - 1) { // Check if not at the bottom edge
					snake->changeDirection(Snake::Down);
				}
				break;
			case SDLK_LEFT:
				if (snake->getHead()->x > 0) { // Check if not at the left edge
					snake->changeDirection(Snake::Left);
				}
				break;
			case SDLK_RIGHT:
				if (snake->getHead()->x < BOARD_WIDTH_UNITS - 1) { // Check if not at the right edge
					snake->changeDirection(Snake::Right);
				}
				break;
			case SDLK_n:
				if (!gameData.snakeAlive) {
					gameData.scoresTabele = readScores(SCOREBOARD_PATH, SCOREBOARD_SIZE);
					restartGame(snake, gameData.snakeSpeedUnitsPerSeconnd, gameData.points, gameData.snakeAlive, gameData.worldTime);
				}
				break;
			//case SDLK_w:
			//	snake->grow();
			}
			break;
		case SDL_KEYUP:
			break;
		case SDL_QUIT:
			gameData.quit = 1;
			break;
		}
	};
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {

	GameData gameData;

	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	// to remove must be changed to "Native"

	if (initSDL(gameData))
		return 1;

	double lastSnakeUpdate = 0;
	gameData.snakeSpeedUnitsPerSeconnd = SNAKE_SPEED;
	gameData.snakeAlive = true;


	Board board = Board(SCREEN_WIDTH, SCREEN_HEIGHT, BOARD_WIDTH_UNITS, BOARD_HEIGHT_USNITS, UNIT_SIZE);
	Snake* snake = getNewSnake(&gameData.points);

	placeFood(gameData.food, false, gameData.screen->format, snake);

	bool speedUpUsed = false;

	gameData.powerUpActive = false;
	gameData.scoresTabele = readScores(SCOREBOARD_PATH, SCOREBOARD_SIZE);


	for (int i = 0; i < SCOREBOARD_SIZE; i++) {
		printf("Score %d: %d\n", i + 1, gameData.scoresTabele[i]);
	}

	while (!gameData.quit) {
		gameData.t2 = SDL_GetTicks();


		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		gameData.delta = (gameData.t2 - gameData.t1) * 0.001;
		gameData.t1 = gameData.t2;

		if (gameData.snakeAlive) {
			gameData.worldTime += gameData.delta;
		}


		// Speed up the snake after a certain time
		if (!speedUpUsed && gameData.worldTime >= SNAKE_SPEEDUP) {
			speedUpUsed = true;
			gameData.snakeSpeedUnitsPerSeconnd *= SNAKE_SPEEDUP_FACTOR;
		}

		// Update the snake position, match sppeed with the game speed
		lastSnakeUpdate += gameData.delta;
		if (lastSnakeUpdate >= 1 / gameData.snakeSpeedUnitsPerSeconnd)
		{
			lastSnakeUpdate = 0;
			moveSnake(snake, &gameData);

			// Check if the snake has collided with itself
			if (snake->checkCollision()) {
				gameData.snakeAlive = false;
				gameData.snakeSpeedUnitsPerSeconnd = 0;
				gameData.scoresTabele[SCOREBOARD_SIZE] = gameData.points;
				sortedArrayToFile(SCOREBOARD_PATH, gameData.scoresTabele, SCOREBOARD_SIZE);
			}
		}

		// Check if the power up has expired
		if (gameData.powerUpActive && gameData.worldTime - POWER_UP_TIME > gameData.powerUpActivationTime && gameData.powerUpActivationTime != 0) {
			gameData.powerUpActive = false;
		}

		draw(snake, &board, &gameData);
		// handling of events (if there were any)
		handleControls(gameData, snake);
	}

	delete snake;
	// freeing all surfaces
	SDL_FreeSurface(gameData.charset);
	SDL_FreeSurface(gameData.screen);
	SDL_DestroyTexture(gameData.scrtex);
	SDL_DestroyRenderer(gameData.renderer);
	SDL_DestroyWindow(gameData.window);

	SDL_Quit();
	return 0;
};
