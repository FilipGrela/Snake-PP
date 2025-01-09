extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

class Snake
{
public:
	enum Directions
	{
		Up,
		Down,
		Left,
		Right
	};

	struct Node;
	Snake(int length, int x, int y, int unitSize, int boardOffsetX, int boardOffsetY, int* pointsVerivle); // Constructor declaration
	~Snake(); // Destructor declaration
	int getLength() const;
	Directions getDirection() const; // Getter for direction
	Directions getNextDirection() const; // Getter for direction

	void move();
	void grow();
	void shorten(int units);
	bool checkCollision() const;
	bool checkIfFreeCell(int x, int y);
	void drawSnake(SDL_Surface* screen, Uint32 color);
	void changeDirection(Directions newDirection);

	Node* getHead() const;

private:
	int* pointsVerivle;
	int unitSize;
	int length;
	int boardOffsetX;
	int boardOffsetY;
	Node* tail;
	Node* head;
	Directions direction;
	Directions nextDirection;
	void addSegment(int x, int y);
	void initSnake();
};

struct Snake::Node {
	int x, y; // Position of the segment
	Node* nextNode; // Pointer to the next segment

	Node(int x, int y) : x(x), y(y), nextNode(nullptr) {}
};

/**
 * @brief Constructor for the Snake class.
 * @param length The initial length of the snake.
 * @param x The initial x-coordinate of the snake's head.
 * @param y The initial y-coordinate of the snake's head.
 * @param unitSize The size of each unit segment of the snake.
 * @param boardOffsetX The x-offset of the board.
 * @param boardOffsetY The y-offset of the board.
 * @param pointsVerivle A pointer to the score variable.
 */
Snake::Snake(int length, int x, int y, int unitSize, int boardOffsetX, int boardOffsetY, int* pointsVerivle) :
	length(length), head(nullptr), unitSize(unitSize), boardOffsetX(boardOffsetX), boardOffsetY(boardOffsetY), pointsVerivle(pointsVerivle) {
	// Initialize the snake with a single segment
	int len = length;
	head = new Node(x, y);
	tail = head;
	head->nextNode = tail;
	*pointsVerivle = 0;

	printf("%d", *pointsVerivle);

	direction = Right;
	initSnake();
	this->length = len;
}

// Destructor definition
Snake::~Snake() {
	Node* current = head;
	while (current != nullptr) {
		Node* next = current->nextNode;
		delete current;
		current = next;
	}
}

void Snake::initSnake() {
	int length = getLength(); // Example length, replace with actual length if available
	this->nextDirection = this->direction;
	printf("Length: %d\n", getLength());
	for (size_t i = 0; i < length - 1; i++) {
		grow();
	}
}

/**
 * @brief Gets the head node of the snake.
 * @return A pointer to the head node of the snake.
 */
Snake::Node* Snake::getHead() const {
	return head;
}

/**
 * @brief Changes the direction of the snake.
 * @param newDirection The new direction to set for the snake.
 */
void Snake::changeDirection(Directions newDirection) {
	// Prevent the snake from reversing direction
	if ((direction == Up && newDirection != Down) ||
		(direction == Down && newDirection != Up) ||
		(direction == Left && newDirection != Right) ||
		(direction == Right && newDirection != Left)) {
		nextDirection = newDirection;
	}
}

/**
 * @brief Draws the snake on the screen.
 * @param screen The SDL_Surface to draw on.
 * @param color The color of the snake.
 */
void Snake::drawSnake(SDL_Surface* screen, Uint32 color) {
	Node* current = head;
	while (current != nullptr) {
		SDL_Rect rect;
		rect.x = current->x * unitSize + boardOffsetX;
		rect.y = current->y * unitSize + boardOffsetY;
		rect.w = unitSize;
		rect.h = unitSize;
		SDL_FillRect(screen, &rect, color);
		current = current->nextNode;
	}
}

/**
 * @brief Moves the snake in the current direction.
 */
void Snake::move() {
	double dx = 0;
	double dy = 0;
	direction = nextDirection;
	if (direction == Up) {
		dy = -1;
	}
	else if (direction == Down) {
		dy = 1;
	}
	else if (direction == Left) {
		dx = -1;
	}
	else if (direction == Right) {
		dx = 1;
	}


	int lastX = 0;
	int lastY = 0;
	int i = 0;
	for (Node* current = head; current != nullptr; current = current->nextNode) {
		int lastOldX = lastX;
		int lastOldY = lastY;
		if (current == head) {
			lastX = current->x;
			lastY = current->y;

			current->x += dx;
			current->y += dy;
		}
		else {

			lastX = current->x;
			lastY = current->y;
			current->x = lastOldX;
			current->y = lastOldY;

		}
	}
}

/**
 * @brief Shortens the snake by the specified number of units.
 * @param units The number of units to shorten the snake by.
 */
void Snake::shorten(int units) {
	for (int i = 0; i < units; i++) {
		if (head == tail) {
			// If the snake has only one segment, do nothing
			return;
		}

		Node* current = head;
		// Traverse to the second-to-last node
		while (current->nextNode != tail) {
			current = current->nextNode;
		}

		// Delete the last node
		delete tail;
		tail = current;
		tail->nextNode = nullptr;
		length--;

		printf("Length: %d\n", getLength());
	}
}

void Snake::grow() {
	int newX = tail->x;
	int newY = tail->y;
	addSegment(newX, newY);

	printf("Length: %d\n", getLength());
}

/**
 * @brief Adds a new segment to the snake at the specified position.
 * @param x The x-coordinate of the new segment.
 * @param y The y-coordinate of the new segment.
 */
void Snake::addSegment(int x, int y) {
	Node* newSegment = new Node(x, y);
	if (tail != nullptr) {
		tail->nextNode = newSegment;
	}
	tail = newSegment;
	length++;
}

/**
 * @brief Checks if the snake has collided with itself.
 * @return True if the snake has collided with itself, false otherwise.
 */
bool Snake::checkCollision() const {

	Node* current = head->nextNode;
	while (current != nullptr) {
		if (head->x == current->x && head->y == current->y) {
			return true;
		}
		current = current->nextNode;
	}

	return false;
}

/**
 * @brief Checks if the specified cell is free (not occupied by the snake).
 * @param x The x-coordinate of the cell.
 * @param y The y-coordinate of the cell.
 * @return True if the cell is free, false otherwise.
 */
bool Snake::checkIfFreeCell(int x, int y) {
	bool isFree = true;
	for (Node* current = head; current != nullptr; current = current->nextNode) {
		if (current->x == x && current->y == y) {
			isFree = false;
			break;
		}
	}
	return isFree;
}

int Snake::getLength() const {
	return length;
}

Snake::Directions Snake::getDirection() const {
	return direction;
}

Snake::Directions Snake::getNextDirection() const {
	return nextDirection;
}

