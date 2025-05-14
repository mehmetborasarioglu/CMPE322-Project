#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

#define GRID_HEIGHT 10
#define GRID_WIDTH 7
#define BURGER_HEIGHT 10

// Global variables
struct termios original_tio;
char grid[GRID_HEIGHT][GRID_WIDTH];
int bun_position = GRID_WIDTH / 2;
int burger_height = 0;
int score = 0;
int game_speed = 200000; // Initial game speed in microseconds

// Ingredients
char ingredients[] = {'P', 'L', 'T', 'C', 'B'}; // Patty, Lettuce, Tomato, Cheese, Bun
char current_ingredient;

// Function prototypes
void configure_terminal();
void reset_terminal();
void handle_sigint(int sig);
void initialize_grid();
void render_grid();
void spawn_ingredient();
void update_ingredient();
int catch_ingredient();
void game_loop();

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

void reset_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
    printf("\nTerminal reset. Goodbye!\n");
}

void handle_sigint(int sig) {
    reset_terminal();
    exit(0);
}

void initialize_grid() {
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            grid[i][j] = '.';
        }
    }
    grid[GRID_HEIGHT - 1][bun_position] = '='; // Place the bun
}

void render_grid() {
    printf("\033[H\033[J"); // Clear the terminal
    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            putchar(grid[i][j]);
        }
        putchar('\n');
    }
    printf("Burger Height: %d / %d\n", burger_height, BURGER_HEIGHT);
    printf("Score: %d\n", score);
}

void spawn_ingredient() {
    int random_index = rand() % 100 < 5 ? 4 : rand() % 4; // 5% chance for bun, otherwise random ingredient
    current_ingredient = ingredients[random_index];
    grid[0][rand() % GRID_WIDTH] = current_ingredient;
}

void update_ingredient() {
    for (int i = GRID_HEIGHT - 2; i >= 0; i--) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (grid[i][j] != '.' && grid[i][j] != '=') {
                grid[i + 1][j] = grid[i][j];
                grid[i][j] = '.';
            }
        }
    }
}

int catch_ingredient() {
    for (int j = 0; j < GRID_WIDTH; j++) {
        if (grid[GRID_HEIGHT - 2][j] != '.' && j == bun_position) {
            char ingredient = grid[GRID_HEIGHT - 2][j];
            grid[GRID_HEIGHT - 2][j] = '.';

            if (ingredient == 'B') {
                // Top bun caught, end game with win
                return -1;
            } else {
                // Update score and burger height
                burger_height++;
                if (ingredient == 'P') score += 50;
                if (ingredient == 'C') score += 25;
                if (ingredient == 'T') score += 15;
                if (ingredient == 'L') score += 5;

                // Increase game speed for difficulty
                if (game_speed > 50000) game_speed -= 5000;
                return 1;
            }
        }
    }
    return 0; // Ingredient missed
}

void game_loop() {
    char input;
    srand(time(NULL));
    spawn_ingredient();

    while (1) {
        render_grid();

        // Check for user input
        if (read(STDIN_FILENO, &input, 1) == 1) {
            if (input == 'a' && bun_position > 0) {
                grid[GRID_HEIGHT - 1][bun_position] = '.';
                bun_position--;
                grid[GRID_HEIGHT - 1][bun_position] = '=';
            } else if (input == 'd' && bun_position < GRID_WIDTH - 1) {
                grid[GRID_HEIGHT - 1][bun_position] = '.';
                bun_position++;
                grid[GRID_HEIGHT - 1][bun_position] = '=';
            }
        }

        // Update falling ingredient
        update_ingredient();

        // Check if the ingredient is caught
        int result = catch_ingredient();
        if (result == -1) {
            printf("You caught the bun! Final Score: %d\n", score);
            break; // Game won
        } else if (result == 0) {
            printf("You missed an ingredient! Game Over. Final Score: 0\n");
            break; // Game lost
        }

        // Spawn a new ingredient if necessary
        if (rand() % 5 == 0) {
            spawn_ingredient();
        }

        // Check for win condition
        if (burger_height == BURGER_HEIGHT) {
            printf("You made the perfect burger! Final Score: %d\n", score);
            break;
        }

        usleep(game_speed); // Delay for game speed
    }
}

int main() {
    configure_terminal();
    initialize_grid();
    game_loop();
    reset_terminal();
    return 0;
}
