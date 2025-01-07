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
    Snake(int size, int length, int x, int y); // Constructor declaration
	~Snake(); // Destructor declaration
    int getLength() const; // Method to return a value

    void move();
    void grow();
    bool checkCollision(int width, int height) const;
    void draw_snake(SDL_Surface* screen, Uint32 color);
    void changeDirection(Directions newDirection); // New method declaration


	Node* getHead() const;

private:
    int size;
    int length;
    Node* tail;
    Node* head;
	Directions direction;
    void addSegment(int x, int y);
	void init_snake();
};


struct Snake::Node {
    int x, y; // Position of the segment
    int size; // Size of the segment
    Node* next_node; // Pointer to the next segment

    Node(int x, int y, int size) : x(x), y(y), size(size), next_node(nullptr) {}
};

// Constructor definition
Snake::Snake(int size, int length, int x, int y) : size(size), length(length), head(nullptr) {
    // Initialize the snake with a single segment
    head = new Node(x, y, size);
    tail = head;
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
	printf("Snake initialized with:\nlength: %d\n", length);
}

Snake::Node* Snake::getHead() const {
    return head;
}

void Snake::changeDirection(Directions newDirection) {
	printf("Changing direction\n");
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
	int i = 0;
    while (current != nullptr) {
        SDL_Rect rect;
		//printf("x: %d, y: %d %d\n", current->x, current->y);
        rect.x = current->x;
        rect.y = current->y;
        rect.w = current->size;
        rect.h = current->size;
        SDL_FillRect(screen, &rect, color);
        current = current->next_node;

        i++;
    }
}

/***
 * This function updates the position of the snake's head by adding a new segment
 * at the new position and removing the last segment to simulate movement.
 * @param dx: The change in the x-coordinate.
 * @param dy: The change in the y-coordinate.
 */
void Snake::move() {
	int dx = 0;
	int dy = 0;
	printf("Moving snake dir = %d\n", direction);
    if (direction == Up) {
        dy = -size;
    }
    else if (direction == Down) {
        dy = size;
    }
    else if (direction == Left) {
        dx = -size;
    }
    else if (direction == Right) {
        dx = size;
    }

    
    int newX = head->x + dx;
    int newY = head->y + dy;
    addSegment(newX, newY);

    Node* temp = head;
    head = head->next_node;
    delete temp;
}

void Snake::grow() {
    int newX = tail->x;
    int newY = tail->y;
    addSegment(newX, newY);
}

void Snake::addSegment(int x, int y) {
    Node* newSegment = new Node(x, y, size);
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


// Method definition
int Snake::getLength() const {
    return length;
}

