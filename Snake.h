class Snake
{
public:
    Snake(int length); // Constructor declaration
	~Snake(); // Destructor declaration
    int getLength() const; // Method to return a value

    void move(int dx, int dy);
    void grow();
    bool checkCollision(int width, int height) const;

    struct Node;
private:
    int length;
    Node* head;
    Node* tail;
    void addSegment(int x, int y);
	void init_snake();
};


struct Snake::Node {
    int x, y; // Position of the segment
    Node* next_node; // Pointer to the next segment

    Node(int x, int y) : x(x), y(y), next_node(nullptr) {}
};

// Constructor definition
Snake::Snake(int length) : length(length), head(nullptr) {
    // Initialize the snake with a single segment
    head = new Node(0, 0);
    tail = head;
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


/***
 * This function updates the position of the snake's head by adding a new segment
 * at the new position and removing the last segment to simulate movement.
 * @param dx: The change in the x-coordinate.
 * @param dy: The change in the y-coordinate.
 */
void move(int dx, int dy);
void Snake::move(int dx, int dy) {
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


// Method definition
int Snake::getLength() const {
    return length;
}

