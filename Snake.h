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
    Snake(int length, int x, int y, int unitSize, int boardOffsetX, int boardOffsetY); // Constructor declaration
    ~Snake(); // Destructor declaration
    int getLength() const;
    Directions getDirection() const; // Getter for direction

    void move();
    void grow();
    bool checkCollision(int width, int height) const;
	bool checkIfFreeCell(int x, int y);
    void draw_snake(SDL_Surface* screen, Uint32 color);
    void changeDirection(Directions newDirection);

    Node* getHead() const;

private:
    int unitSize;
    int length;
    int boardOffsetX;
    int boardOffsetY;
    Node* tail;
    Node* head;
    Directions direction;
    void addSegment(int x, int y);
    void init_snake();
};

struct Snake::Node {
    int x, y; // Position of the segment
    Node* next_node; // Pointer to the next segment

    Node(int x, int y) : x(x), y(y), next_node(nullptr) {}
};

// Constructor definition
Snake::Snake(int length, int x, int y, int unitSize, int boardOffsetX, int boardOffsetY) :
    length(length), head(nullptr), unitSize(unitSize), boardOffsetX(boardOffsetX), boardOffsetY(boardOffsetY) {
    // Initialize the snake with a single segment
    head = new Node(x, y);
	tail = new Node(x, y - 1);
    head->next_node = tail;

	direction = Right;
	init_snake();
}

// Destructor definition
Snake::~Snake() {
    Node* current = head;
    while (current != nullptr) {
        Node* next = current->next_node;
        delete current;
        current = next;
    }
}

void Snake::init_snake() {
	int length = getLength(); // Example length, replace with actual length if available
    printf("%d",getLength());
    for (size_t i = 0; i < length; i++)
    {
		grow();
    }
}

Snake::Node* Snake::getHead() const {
    return head;
}

void Snake::changeDirection(Directions newDirection) {
    // Prevent the snake from reversing direction
    if ((direction == Up && newDirection != Down) ||
        (direction == Down && newDirection != Up) ||
        (direction == Left && newDirection != Right) ||
        (direction == Right && newDirection != Left)) {
        direction = newDirection;
    }
}

void Snake::draw_snake(SDL_Surface* screen, Uint32 color) {
    Node* current = head;
    while (current != nullptr) {
        SDL_Rect rect;
        rect.x = current->x * unitSize + boardOffsetX;
        rect.y = current->y * unitSize + boardOffsetY;
        rect.w = unitSize;
        rect.h = unitSize;
        SDL_FillRect(screen, &rect, color);
        current = current->next_node;
    }
}

/***
 * This function updates the position of the snake's head by adding a new segment
 * at the new position and removing the last segment to simulate movement.
 * @param dx: The change in the x-coordinate.
 * @param dy: The change in the y-coordinate.
 */
void Snake::move() {
	double dx = 0;
	double dy = 0;
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


    int last_x = 0;
    int last_y = 0;
    int i = 0;
	for (Node* current = head; current != nullptr; current = current->next_node) {
        int last_old_x = last_x;
		int last_old_y = last_y;
		if (current == head) {
            last_x = current->x;
            last_y = current->y;

			current->x += dx;
			current->y += dy;
		}
		else {

            last_x = current->x;
            last_y = current->y;
            current->x = last_old_x;
            current->y = last_old_y;

		}
	}
}

void Snake::grow() {
    int newX = tail->x;
    int newY = tail->y-1;
    addSegment(newX, newY);
}

void Snake::addSegment(int x, int y) {
    Node* newSegment = new Node(x, y);
    if (tail != nullptr) {
        tail->next_node = newSegment;
    }
    tail = newSegment;
}

bool Snake::checkCollision(int width, int height) const {
    if (head->x < 0 || head->x >= width || head->y < 0 || head->y >= height) {
        return true;
    }

    Node* current = head->next_node;
    while (current != nullptr) {
        if (head->x == current->x && head->y == current->y) {
            return true;
        }
        current = current->next_node;
    }

    return false;
}

bool Snake::checkIfFreeCell(int x, int y) {
    bool isCollision = false;
	for (Node* current = head; current != nullptr; current = current->next_node) {
		if (current->x == x && current->y == y) {
			isCollision = true;
			break;
		}
	}
	return isCollision;
}

// Method definition
int Snake::getLength() const {
    return length;
}

Snake::Directions Snake::getDirection() const {
    return direction;
}

