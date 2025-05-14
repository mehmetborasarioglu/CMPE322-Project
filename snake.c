#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>
void configure_terminal();
void initialize_game();
void game_loop();
void cleanup();
void reset_terminal();
void handle_sigint(int sig);
void move_snake(char pressed_button);
void make_move();
void render_grid();
#define GRID_SIZE 15

struct termios original_tio;

// Snake structure
typedef struct {
    int x, y;
} Point;

typedef struct {
    Point *body;
    int length;
} Snake;

int main() {
    configure_terminal();
    initialize_game();
    game_loop();

    cleanup();

    return 0;
}

Snake snake;
Point bait;
char grid[GRID_SIZE][GRID_SIZE];
int running = 1;

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}


void handle_sigint(int sig) {
    reset_terminal();
    printf("\nGame exited. Goodbye!\n");
    exit(0);
}

void configure_terminal() {
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &original_tio);
    signal(SIGINT, handle_sigint);
    new_tio = original_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO); 
    new_tio.c_cc[VMIN] = 1;             
    new_tio.c_cc[VTIME] = 0;            
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void initialize_game() {
    //fill the gird with dots
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '.';
        }
    }

    // initially snake has length 1
    snake.length = 1;
    snake.body = (Point *)malloc(sizeof(Point));
    snake.body[0].x = GRID_SIZE / 2;
    snake.body[0].y = GRID_SIZE / 2;
    grid[snake.body[0].x][snake.body[0].y] = 'O';

    
    srand(time(NULL)); 
    do {
        bait.x = rand() % GRID_SIZE;
        bait.y = rand() % GRID_SIZE;
    } while (bait.x == snake.body[0].x && bait.y == snake.body[0].y); // reseed if bait is inside the snake
    grid[bait.x][bait.y] = 'X';
}


void make_move() {
    // Clear the grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '.';
        }
    }

    // Place bait
    grid[bait.x][bait.y] = 'X';

    // Place snake
    for (int i = 0; i < snake.length; i++) {
        if (i == 0)
            grid[snake.body[i].x][snake.body[i].y] = 'O'; // Head
        else
            grid[snake.body[i].x][snake.body[i].y] = '#'; // Body
    }
}

// Move the snake
void move_snake(char pressed_button) {
    Point new_head = snake.body[0];
    if (pressed_button == 'w') new_head.x--;
    else if (pressed_button == 's') new_head.x++;
    else if (pressed_button == 'a') new_head.y--;
    else if (pressed_button == 'd') new_head.y++;

    if (new_head.x < 0) new_head.x = GRID_SIZE - 1;
    if (new_head.x >= GRID_SIZE) new_head.x = 0;
    if (new_head.y < 0) new_head.y = GRID_SIZE - 1;
    if (new_head.y >= GRID_SIZE) new_head.y = 0;

    

    // Grow the snake if it eats the bait
    if (new_head.x == bait.x && new_head.y == bait.y) {
        snake.length++;
        snake.body = (Point *)realloc(snake.body, snake.length * sizeof(Point));

        do {
            bait.x = rand() % GRID_SIZE;
            bait.y = rand() % GRID_SIZE;
        } while (grid[bait.x][bait.y] != '.');
    }

    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    snake.body[0] = new_head;

    
    make_move();
}

void render_grid() {
    printf("\033[H\033[J"); // Clear the screen
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            putchar(grid[i][j]);
        }
        putchar('\n');
    }
}

// Run the game loop
void game_loop() {
    char input;
    while (running) {
        // Render the grid
        render_grid();

        // Get user input
        input = getchar();

        // Exit on 'q'
        if (input == 'q') {
            printf("Exiting game...\n");
            running = 0;
            break;
        }

        // Process movement
        if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
            move_snake(input);
        }

        usleep(100000); // 100ms
    }
}

// Cleanup resources
void cleanup() {
    free(snake.body);
    reset_terminal();
}
