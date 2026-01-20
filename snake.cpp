#include <iostream>
#include <deque>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <random>
#include <cstdlib>

// Platform-specific headers for non-blocking input
#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

using namespace std;

// Represents a 2D coordinate
struct Point {
    int x, y;
};

// Represents the possible directions the snake can move
enum class Direction {
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

#ifndef _WIN32
struct termios old_tio;

void init_termios() {
    tcgetattr(STDIN_FILENO, &old_tio);
    struct termios new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void reset_termios() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

int kbhit() {
    char ch;
    int oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    int n = read(STDIN_FILENO, &ch, 1);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (n > 0) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}
#endif

class Game {
private:
    // Game state
    const int width;
    const int height;
    atomic<bool> gameOver; // atomic for thread-safe access
    int score;

    // Snake properties
    Point snakeHead;
    deque<Point> snakeBody;
    Direction dir;

    // Food properties
    Point food;
    
    // Threading and synchronization
    mutex inputMutex; // Protects access to 'dir' variable


    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    
    void generateFood() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> distribX(1, width - 2);
        uniform_int_distribution<> distribY(1, height - 2);

        food.x = distribX(gen);
        food.y = distribY(gen);
    }

public:
    
    Game(int w, int h) : width(w), height(h), gameOver(false), score(0), dir(Direction::STOP) {}

    
    void setup() {
        // Center the snake initially
        snakeHead.x = width / 2;
        snakeHead.y = height / 2;
        snakeBody.clear();
        score = 0;
        dir = Direction::STOP;

        // Place the first food item
        generateFood();
    }

    
    void draw() {
        clearScreen();
        // Top border
        for (int i = 0; i < width; ++i) cout << "#";
        cout << endl;

        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (j == 0 || j == width - 1) {
                    cout << "#"; // Side borders
                } else if (i == snakeHead.y && j == snakeHead.x) {
                    cout << "O"; // Snake head
                } else if (i == food.y && j == food.x) {
                    cout << "F"; // Food
                } else {
                    bool isBodyPart = false;
                    for (const auto& part : snakeBody) {
                        if (part.x == j && part.y == i) {
                            cout << "o"; // Snake body
                            isBodyPart = true;
                            break;
                        }
                    }
                    if (!isBodyPart) {
                        cout << " "; // Empty space
                    }
                }
            }
            cout << endl;
        }

        // Bottom border
        for (int i = 0; i < width; ++i) cout << "#";
        cout << endl;
        cout << "Score: " << score << endl;
        cout << "Use w/a/s/d to move. Press 'x' to quit." << endl;
    }

    
    void input() {
        while (!gameOver) {
            if (kbhit()) {
                char move = 0;
#ifdef _WIN32
                move = _getch();
#else
                move = getchar();
#endif
                
                // Lock the mutex before modifying the shared 'dir' variable
                lock_guard<mutex> lock(inputMutex);
                
                // Prevent the snake from reversing on itself
                switch (move) {
                    case 'a':
                        if (dir != Direction::RIGHT) dir = Direction::LEFT;
                        break;
                    case 'd':
                        if (dir != Direction::LEFT) dir = Direction::RIGHT;
                        break;
                    case 'w':
                        if (dir != Direction::DOWN) dir = Direction::UP;
                        break;
                    case 's':
                        if (dir != Direction::UP) dir = Direction::DOWN;
                        break;
                    case 'x':
                        gameOver = true;
                        break;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(20)); // Small sleep to prevent high CPU usage
        }
    }

    
    void logic() {
        // Add current head to the body
        if (dir != Direction::STOP) {
            snakeBody.push_front(snakeHead);
        }

        // Move the head based on the current direction
        {
            // Lock the mutex to safely read the direction
            lock_guard<mutex> lock(inputMutex);
            switch (dir) {
                case Direction::LEFT:  snakeHead.x--; break;
                case Direction::RIGHT: snakeHead.x++; break;
                case Direction::UP:    snakeHead.y--; break;
                case Direction::DOWN:  snakeHead.y++; break;
                default: break;
            }
        }

        // Check for collision with walls
        if (snakeHead.x >= width-1 || snakeHead.x <= 0 || snakeHead.y >= height || snakeHead.y < 0) {
            gameOver = true;
            return;
        }

        // Check for collision with self
        for (const auto& part : snakeBody) {
            if (part.x == snakeHead.x && part.y == snakeHead.y) {
                gameOver = true;
                return;
            }
        }

        // Check if snake eats food
        if (snakeHead.x == food.x && snakeHead.y == food.y) {
            score += 10;
            generateFood();
        } else {
            // If no food is eaten, remove the tail to simulate movement
            if (dir != Direction::STOP) {
                snakeBody.pop_back();
            }
        }
    }

    /**
     * @brief The main entry point to start and run the game.
     */
    void run() {
        setup();
        
        // Start the input handling in a separate thread
        thread inputThread(&Game::input, this);

        while (!gameOver) {
            draw();
            logic();
            // Control game speed
            this_thread::sleep_for(chrono::milliseconds(150));
        }

        // Wait for the input thread to finish before exiting
        inputThread.join();

        clearScreen();
        cout << "GAME OVER " << endl;
        cout << "Your final score: " << score << endl;
        
    }
};

int main() {
#ifndef _WIN32
    init_termios(); // Set up terminal for non-blocking input on Unix
#endif

    Game snakeGame(40, 20); // Create a game with a 40x20 board
    snakeGame.run();

#ifndef _WIN32
    reset_termios(); // Restore terminal settings on Unix
#endif
    return 0;
}