#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>

#define GRID_SIZE 5

// Global variables
struct termios original_tio;
char player1_grid[GRID_SIZE][GRID_SIZE];
char player2_grid[GRID_SIZE][GRID_SIZE];
char player1_display[GRID_SIZE][GRID_SIZE];
char player2_display[GRID_SIZE][GRID_SIZE];
int cursor_x = 0, cursor_y = 0;

// Function prototypes
void configure_terminal();
void reset_terminal();
void handle_sigint(int sig);
void initialize_grids();
void render_grid(char grid[GRID_SIZE][GRID_SIZE], int is_placing);
int place_ship(char grid[GRID_SIZE][GRID_SIZE], int length, int is_vertical);
int fire_at(char grid[GRID_SIZE][GRID_SIZE], char display[GRID_SIZE][GRID_SIZE]);
void game_loop();

// Terminal configuration functions
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

// Initialize grids
void initialize_grids() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            player1_grid[i][j] = '.';
            player2_grid[i][j] = '.';
            player1_display[i][j] = '.';
            player2_display[i][j] = '.';
        }
    }
}

// Render a grid
void render_grid(char grid[GRID_SIZE][GRID_SIZE], int is_placing) {
    printf("\033[H\033[J");
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (is_placing && i == cursor_x && j == cursor_y) {
                putchar('+'); // Cursor
            } else {
                putchar(grid[i][j]);
            }
        }
        putchar('\n');
    }
}

// Place a ship on the grid
int place_ship(char grid[GRID_SIZE][GRID_SIZE], int length, int is_vertical) {
    // Check if the ship fits and does not overlap
    for (int i = 0; i < length; i++) {
        int x = cursor_x + (is_vertical ? i : 0);
        int y = cursor_y + (is_vertical ? 0 : i);
        if (x >= GRID_SIZE || y >= GRID_SIZE || grid[x][y] != '.') {
            return 0; // Invalid placement
        }
    }

    // Place the ship
    for (int i = 0; i < length; i++) {
        int x = cursor_x + (is_vertical ? i : 0);
        int y = cursor_y + (is_vertical ? 0 : i);
        grid[x][y] = 'S';
    }
    return 1; // Successfully placed
}

// Fire at a grid
int fire_at(char grid[GRID_SIZE][GRID_SIZE], char display[GRID_SIZE][GRID_SIZE]) {
    if (grid[cursor_x][cursor_y] == 'S') {
        grid[cursor_x][cursor_y] = 'X';
        display[cursor_x][cursor_y] = 'X';
        return 1; // Hit
    } else if (grid[cursor_x][cursor_y] == '.') {
        display[cursor_x][cursor_y] = '.';
        return 0; // Miss
    }
    return -1; // Already hit
}

// Game loop
void game_loop() {
    char input;
    int is_placing_phase = 1;

    // Placing phase
    for (int player = 1; player <= 2; player++) {
        for (int ship = 0; ship < 2; ship++) {
            int length = (ship == 0) ? 3 : 1;
            int placed = 0;

            while (!placed) {
                render_grid(player == 1 ? player1_grid : player2_grid, 1);
                printf("Player %d: Place your ship of length %d (use 'v' for vertical, 'h' for horizontal): ", player, length);
                input = getchar();

                if (input == 'w' && cursor_x > 0) cursor_x--;
                if (input == 's' && cursor_x < GRID_SIZE - 1) cursor_x++;
                if (input == 'a' && cursor_y > 0) cursor_y--;
                if (input == 'd' && cursor_y < GRID_SIZE - 1) cursor_y++;

                if (input == 'v' || input == 'h') {
                    placed = place_ship(player == 1 ? player1_grid : player2_grid, length, input == 'v');
                }
            }
        }
    }

    // Shooting phase
    while (1) {
        render_grid(current_player == 1 ? player2_display : player1_display, 1);
        printf("Player %d: Take your shot!\n", current_player);
        input = getchar();

        if (input == 'w' && cursor_x > 0) cursor_x--;
        if (input == 's' && cursor_x < GRID_SIZE - 1) cursor_x++;
        if (input == 'a' && cursor_y > 0) cursor_y--;
        if (input == 'd' && cursor_y < GRID_SIZE - 1) cursor_y++;

        if (input == '\n') {
            int hit = fire_at(current_player == 1 ? player2_grid : player1_grid,
                              current_player == 1 ? player2_display : player1_display);
            if (hit >= 0) {
                current_player = (current_player == 1) ? 2 : 1;
            }
        }
    }
}

// Main function
int main() {
    configure_terminal();
    initialize_grids();
    game_loop();
    reset_terminal();
    return 0;
}
