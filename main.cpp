#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>

#include"Snake.h"
#include"Board.h"


extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
} 

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	540
#define INFO_SCREN_HEIGHT 36

#define BOARD_WIDTH_UNITS 20
#define BOARD_HEIGHT_USNITS 20

#define UNIT_SIZE 20

#define SNAKE_SPEED 8 // units per second

struct GameData {

	int t1, t2, quit, frames, rc, points;
	double delta, worldTime, fpsTimer, fps;
	bool snakeAlive;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	
};


// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
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


// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void moveSnake(Snake* snake) {
	int headX = snake->getHead()->x;
	int headY = snake->getHead()->y;
	Snake::Directions direction = snake->getDirection();

	if (headX <= 0 && direction == Snake::Left) {
		snake->changeDirection(
			(!snake->checkIfFreeCell(headX, headY - 1) && headY != 0) ? Snake::Up : Snake::Down);
	}
	else if (headX >= BOARD_WIDTH_UNITS - 1 && direction == Snake::Right) {
		snake->changeDirection(
			(!snake->checkIfFreeCell(headX, headY + 1) && headY != BOARD_HEIGHT_USNITS - 1) ? Snake::Down : Snake::Up);
	}
	else if (headY <= 0 && direction == Snake::Up) {
		snake->changeDirection(
			(!snake->checkIfFreeCell(headX + 1, headY) && headX != BOARD_WIDTH_UNITS - 1) ? Snake::Right : Snake::Left);
	}
	else if (headY >= BOARD_HEIGHT_USNITS - 1 && direction == Snake::Down) {
		snake->changeDirection(
			(!snake->checkIfFreeCell(headX - 1, headY) && headX != 0) ? Snake::Left : Snake::Right);
	}


	snake->move();
}

void drawInfo(SDL_Surface* screen, Uint32 color1, Uint32 color2, double worldTime, double fps, SDL_Surface* charset, SDL_Texture* scrtex, SDL_Renderer* renderer) {
	// info text
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, INFO_SCREN_HEIGHT, color1, color2);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	char text[128];
	sprintf(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf(text, "Esc - wyjscie, \030 - przyspieszenie, \031 - zwolnienie");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

Snake* getNewSnake(int* pointsVerible) {
	int snakeOffsetX = (SCREEN_WIDTH - BOARD_WIDTH_UNITS * UNIT_SIZE) / 2;
	int snakeOffsetY = (SCREEN_HEIGHT - BOARD_HEIGHT_USNITS * UNIT_SIZE) / 2;
	
	return new Snake(2, BOARD_WIDTH_UNITS / 2, BOARD_HEIGHT_USNITS / 2, UNIT_SIZE, snakeOffsetX, snakeOffsetY, pointsVerible);
}

void draw(Snake* snake, Board* board,GameData* gameData) {
	int czarny = SDL_MapRGB(gameData->screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(gameData->screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(gameData->screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(gameData->screen->format, 0x11, 0x11, 0xCC);
	

	SDL_FillRect(gameData->screen, NULL, czarny);
	board->render(gameData->screen);
	snake->draw_snake(gameData->screen, niebieski);


	if (!gameData->snakeAlive) {
		DrawString(gameData->screen, SCREEN_WIDTH / 2 - 8 * 4, SCREEN_HEIGHT / 2 - 8, "GAME OVER", gameData->charset);
	}


	drawInfo(gameData->screen, niebieski, czerwony, gameData->worldTime, gameData->fps, gameData->charset, gameData->scrtex, gameData->renderer);
}

void restartGame(Snake** snake, double& snakeSpeedUnitsPerSeconnd, int& points, bool& snakeAlive) {
	delete *snake;
	*snake = getNewSnake(&points);
	snakeSpeedUnitsPerSeconnd = SNAKE_SPEED;
	snakeAlive = true;
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {

	GameData gameData;



	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	// to remove must be changed to "Native"

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	gameData.rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &gameData.window, &gameData.renderer);

	if(gameData.rc != 0) {
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
	if(gameData.charset == NULL) {
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

	gameData.frames = 0;
	gameData.fpsTimer = 60;
	gameData.fps = 0;
	gameData.quit = 0;
	gameData.worldTime = 0;

	double snakeSpeedUnitsPerSeconnd = SNAKE_SPEED;
	double lastSnakeUpdate = 0;
	gameData.snakeAlive = true;


	Board board = Board(SCREEN_WIDTH, SCREEN_HEIGHT, BOARD_WIDTH_UNITS, BOARD_HEIGHT_USNITS, UNIT_SIZE);
	Snake* snake = getNewSnake(&gameData.points);

	while (!gameData.quit) {
		gameData.t2 = SDL_GetTicks();

		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		gameData.delta = (gameData.t2 - gameData.t1) * 0.001;
		gameData.t1 = gameData.t2;

		gameData.worldTime += gameData.delta;


		gameData.fpsTimer += gameData.delta;
		if (gameData.fpsTimer > 0.5) {
			gameData.fps = gameData.frames * 2;
			gameData.frames = 0;
			gameData.fpsTimer -= 0.5;
		};


		
		lastSnakeUpdate += gameData.delta;
		if (lastSnakeUpdate >= 1 / snakeSpeedUnitsPerSeconnd)
		{
			lastSnakeUpdate = 0;
			moveSnake(snake);
			if (snake->checkCollision()) {
				gameData.snakeAlive = false;
				snakeSpeedUnitsPerSeconnd = 0;
			}
		}
		
		draw(snake, &board, &gameData);
		// handling of events (if there were any)
		while (SDL_PollEvent(&gameData.event)) {
			switch (gameData.event.type) {
			case SDL_KEYDOWN:
				switch (gameData.event.key.keysym.sym) {
				case SDLK_ESCAPE:
					gameData.quit = 1;
					break;
				case SDLK_UP:
					snake->changeDirection(Snake::Up);
					break;
				case SDLK_DOWN:
					snake->changeDirection(Snake::Down);
					break;
				case SDLK_LEFT:
					snake->changeDirection(Snake::Left);
					break;
				case SDLK_RIGHT:
					snake->changeDirection(Snake::Right);
					break;
				case SDLK_w:
					snake->grow();
					break;
				case SDLK_n:
					if (!gameData.snakeAlive) {
						restartGame(&snake, snakeSpeedUnitsPerSeconnd, gameData.points, gameData.snakeAlive);
					}
					break;
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				gameData.quit = 1;
				break;
			}

			gameData.frames++;
		};
	}
	// freeing all surfaces
	SDL_FreeSurface(gameData.charset);
	SDL_FreeSurface(gameData.screen);
	SDL_DestroyTexture(gameData.scrtex);
	SDL_DestroyRenderer(gameData.renderer);
	SDL_DestroyWindow(gameData.window);

	SDL_Quit();
	return 0;
	};
