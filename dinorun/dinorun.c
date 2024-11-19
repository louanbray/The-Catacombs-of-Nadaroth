#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

/// Alternative plus 'trans-plateforme' que 'system("clear")'
#define clear_screen() printf("\033[H\033[J")

const int DISPLAY_POS = 2;
const int SCORE_DIVISOR = 2;
const int COL = 100;
const int ROW = 24;

int TICKS, GAME_PHASE, SCORE_POS, PLAYER_POS, PLAYER_HEIGHT;
bool IS_GAME_OVER;

char USERNAME[20];
int MAX_PLAYER = 0;
int ID = -1;

int COLOR = 1;
int ANIMATION = 1;

int ULTIMATE_CHARGE, FIRE_COOLDOWN_CHARGE = 0;
int FIRE_COOLDOWN = 4;
bool IS_ULTIMATE_FIRED;

char** S_ENEMY1;
char** S_ENEMY2;
char** S_ENEMY3;
char** S_BIRD;

/// /!\ Ne pas toucher à cette fonction, je ne la comprend pas.
// Gets a char from STDIN
// Returns immediatly, even if STDIN is empty
// in which case it returns ?
int getch() {
    int acquisition_time = 50;
    int ch;
    struct termios oldattr, newattr;

    if (tcgetattr(STDIN_FILENO, &oldattr) != 0) {
        perror("tcgetattr");
    }

    newattr = oldattr;
    // Disable canonical mode and echo
    newattr.c_lflag &= ~(ICANON | ECHO);

    // Set VMIN and VTIME to control read behavior
    newattr.c_cc[VMIN] = 1;   // Minimum number of characters to read
    newattr.c_cc[VTIME] = 0;  // Timeout value (0 = no timeout)

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newattr) != 0) {
        perror("tcsetattr");
    }
    struct pollfd mypoll = {STDIN_FILENO, POLLIN | POLLPRI};

    // Poll the input
    if (poll(&mypoll, 1, acquisition_time) > 0) {
        ch = getchar();
    } else if (errno != 0) {
        perror("poll");
    }

    // Flush remaining input to avoid characters "remaining"
    tcflush(STDIN_FILENO, TCIFLUSH);

    // Restore the original terminal settings
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oldattr) != 0) {
        perror("tcsetattr");
    }
    return ch;
}
/// Fin /!\

typedef struct slot_s {
    char** val;
    int** type;
} slot;

typedef struct jump_s {
    bool isJumping;
    int jumpHeight;
    int jumpLength;
    int jumpPhase;
} jump_parameters;

enum Type {
    NONE,
    DISPLAY,
    OBSTACLE,
    FLOOR,
    PLAYER,
    AMMO,
    BIRD,
    GAME_OVER,
    MENU,
    LEADERBOARD,
    ULTIMATE
};

enum GamePhase {
    EASY,
    MEDIUM,
    HARD,
    HELL
};

typedef struct player_s {
    char pseudo[20];
    int bestScore;
} player;

typedef struct score_s {
    int* ids;
    int score;
    player* bestScore;
} score_tab;

// Generate main structures
slot generate_board() {
    char** val = malloc(sizeof(char*) * ROW);
    int** type = malloc(sizeof(int*) * ROW);
    for (int i = 0; i < ROW; i++) {
        char* col_val = malloc(sizeof(char) * COL);
        int* col_type = malloc(sizeof(int) * COL);
        for (int j = 0; j < COL; j++) {
            col_val[j] = ' ';
            col_type[j] = NONE;
        }
        val[i] = col_val;
        type[i] = col_type;
    }
    slot board = {val, type};
    return board;
}

score_tab init_scores() {
    FILE* file = fopen("score.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int max_id = 1;
    char c_temp = fgetc(file);
    while (c_temp != EOF) {
        max_id += c_temp == ',';
        c_temp = fgetc(file);
    }
    fclose(file);

    MAX_PLAYER = max_id - 1;

    score_tab score;
    int* id_list = malloc(max_id * sizeof(int));
    for (int i = 0; i < max_id; i++) {
        id_list[i] = i;
    }

    score.ids = id_list;
    score.score = 0;

    player* joueurs = malloc(max_id * sizeof(player));
    for (int i = 0; i < max_id; i++) {
        joueurs[i].pseudo[0] = '\0';
        joueurs[i].bestScore = 0;
    }

    score.bestScore = joueurs;
    return score;
}

/// Utils
// Power function for integers
int ipow(int base, int exp) {
    int result = 1;
    for (;;) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}
// Swap values at indexes i,j of array arr
void swap(int* arr, int i, int j) {
    int mem = arr[j];
    arr[j] = arr[i];
    arr[i] = mem;
}
// Returns the min value of i1, i2
int min(int i1, int i2) {
    return (i1 < i2) ? i1 : i2;
}
// Returns length of an integer < 1e11
int lenHelper(unsigned x) {
    if (x >= 1000000000) return 10;
    if (x >= 100000000) return 9;
    if (x >= 10000000) return 8;
    if (x >= 1000000) return 7;
    if (x >= 100000) return 6;
    if (x >= 10000) return 5;
    if (x >= 1000) return 4;
    if (x >= 100) return 3;
    if (x >= 10) return 2;
    return 1;
}

// Prints the game board.
void print_board(slot board) {
    clear_screen();
    if (COLOR) {
        for (int i = ROW - 1; i >= 0; i--) {
            for (int j = 0; j < COL; j++) {
                int type = board.type[i][j];
                char val = board.val[i][j];
                if (val != ' ') {
                    if (type == OBSTACLE) {
                        printf("\x1b[30;1m%c\033[0m", val);
                    } else if (type == FLOOR) {
                        printf("\033[1;32m%c\033[0m", val);
                    } else if (type == DISPLAY) {
                        printf("\033[0;36m%c\033[0m", val);
                    } else if (type == GAME_OVER) {
                        printf("\x1b[31;1m%c\033[0m", val);
                    } else if (type == PLAYER || type == LEADERBOARD) {
                        printf("\x1b[33;1m%c\033[0m", val);
                    } else if (type == MENU || type == BIRD) {
                        printf("\x1b[34;1m%c\033[0m", val);
                    } else if (type == ULTIMATE) {
                        printf("\x1b[35m%c\033[0m", val);
                    } else {
                        printf("%c", val);
                    }
                } else {
                    printf(" ");
                }
            }
            printf("\n");
        }
    } else {
        for (int i = ROW - 1; i >= 0; i--) {
            char buffer[101];
            sprintf(&buffer[0], "%100s", &board.val[i][0]);
            printf("%100s\n", buffer);
        }
    }
}

// Load a sprite from a file on the board at specific coordinates and return the last column modified
int load_sprite(int x, int y, slot board, int t, char* sprite) {
    FILE* file = fopen(sprite, "r");
    if (file == NULL) {
        perror("Error opening file");
        return x;
    }
    char c = fgetc(file);
    int height = 0;
    while (c != ',') {
        height = height * 10 + (c - 48);
        c = fgetc(file);
    }
    c = fgetc(file);
    int length = 0;
    while (c != ',') {
        length = length * 10 + (c - 48);
        c = fgetc(file);
    }

    c = fgetc(file);
    c = fgetc(file);
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < length; j++) {
            c = fgetc(file);
            if (board.type[i + y][j + x] == NONE && c != ' ') {
                board.val[i + y][j + x] = c;
                board.type[i + y][j + x] = t;
            }
        }
        c = fgetc(file);
        if (c != EOF)
            c = fgetc(file);
    }

    fclose(file);
    return x + length - 1;
}
// Place a hard loaded sprite
void place(int x, int y, slot board, int t, char** sprite, int height, int length) {
    for (register int i = height - 1; i >= 0; i--) {
        int row = i + y;
        for (int j = 0; j < length; j++) {
            int col = j + x;
            if (board.type[row][col] == NONE && sprite[i][j] != ' ') {
                board.val[row][col] = sprite[i][j];
                board.type[row][col] = t;
            }
        }
    }
}
// Hard Loading of a sprite to an array
char** load(char* sprite) {
    FILE* file = fopen(sprite, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }
    char c = fgetc(file);
    int height = 0;
    while (c != ',') {
        height = height * 10 + (c - 48);
        c = fgetc(file);
    }
    c = fgetc(file);
    int length = 0;
    while (c != ',') {
        length = length * 10 + (c - 48);
        c = fgetc(file);
    }

    c = fgetc(file);
    c = fgetc(file);

    char** sprite_array = malloc(sizeof(char*) * height);
    for (int i = height - 1; i >= 0; i--) {
        char* col_ = malloc(sizeof(char) * length);
        for (int j = 0; j < length; j++) {
            c = fgetc(file);
            col_[j] = c;
        }
        sprite_array[i] = col_;
        c = fgetc(file);
        if (c != EOF)
            c = fgetc(file);
    }

    fclose(file);
    return sprite_array;
}

// Save best score in the file
void saveBestScore(score_tab score) {
    FILE* file = fopen("score.txt", "w+");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    for (int i = 0; i < MAX_PLAYER; i++) {
        fprintf(file, "%s:%d,", score.bestScore[i].pseudo, score.bestScore[i].bestScore);
        if (i != MAX_PLAYER - 1) {
            fprintf(file, "\n");
        }
    }
    fclose(file);
}
// Parse best scores file
void getBestScore(score_tab scores) {
    FILE* file = fopen("score.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    int id = 0;
    char c = fgetc(file);
    int i = 0;
    while (c != EOF) {
        for (i = 0; c != ':'; i++) {
            scores.bestScore[id].pseudo[i] = c;
            c = fgetc(file);
        }
        scores.bestScore[id].pseudo[i] = '\0';
        if (strcmp(USERNAME, scores.bestScore[id].pseudo) == 0)
            ID = id;

        c = fgetc(file);
        while (c != ',') {
            scores.bestScore[id].bestScore = scores.bestScore[id].bestScore * 10 + (c - 48);
            c = fgetc(file);
        }
        c = fgetc(file);
        if (c != EOF)
            c = fgetc(file);

        id++;
    }
    if (ID == -1) {
        ID = id;
        MAX_PLAYER += 1;
        strcpy(scores.bestScore[id].pseudo, USERNAME);
    }
    fclose(file);
}

// Initialize the board.
void init_board(slot board) {
    for (int i = 0; i < ROW; i++ /*waw ... space*/) {
        for (int j = 0; j < COL; j++) {
            board.val[i][j] = ' ';
            board.type[i][j] = NONE;
        }
    }

    if (!IS_GAME_OVER) {
        for (int j = 0; j < COL; j++ /*build floor*/) {
            board.val[0][j] = j % 2 == 0 ? '-' : '_';
            board.type[0][j] = FLOOR;
        }
        load_sprite(DISPLAY_POS, ROW - 2, board, DISPLAY, "sprite_best_score.txt");
        load_sprite(DISPLAY_POS, ROW - 3, board, DISPLAY, "sprite_score.txt");
        load_sprite(COL / 2 - 9, ROW - 2, board, ULTIMATE, "sprite_ultimate_text.txt");
        board.val[ROW - 3][(COL / 2) - 6] = '[';
        board.val[ROW - 3][(COL / 2) + 5] = ']';

        for (int i = 0; i <= 11; i++)
            board.type[ROW - 3][(COL / 2) - 6 + i] = ULTIMATE;

        load_sprite(COL - 14, ROW - 2, board, MENU, "sprite_fire_text.txt");
        board.val[ROW - 3][COL - 11] = '[';
        board.val[ROW - 3][COL - 4] = ']';

        for (int i = 0; i <= 7; i++)
            board.type[ROW - 3][COL - 11 + i] = MENU;

        PLAYER_POS = load_sprite(PLAYER_POS, 1, board, PLAYER, "sprite_player.txt");
    }
}

// Graphic Update for the score
void score_update(slot board, score_tab* score) {
    int i = 0;
    int _score = (*score).score;
    int _bestScore = (*score).bestScore[ID].bestScore;
    if (_score > _bestScore) {
        (*score).bestScore[ID].bestScore = _score;
        _bestScore = _score;
        saveBestScore(*score);
    }
    _score /= SCORE_DIVISOR;
    _bestScore /= SCORE_DIVISOR;
    i = lenHelper(_score);
    for (int j = 1; j <= i; j++) {
        board.val[ROW - 3][DISPLAY_POS + 7 + j] = 48 + (_score / ipow(10, i - j)) % 10;
        board.type[ROW - 3][DISPLAY_POS + 7 + j] = DISPLAY;
    }
    i = lenHelper(_bestScore);
    for (int j = 1; j <= i; j++) {
        board.val[ROW - 2][DISPLAY_POS + 12 + j] = 48 + (_bestScore / ipow(10, i - j)) % 10;
        board.type[ROW - 2][DISPLAY_POS + 12 + j] = DISPLAY;
    }
}
// Update ultimate charge
void ultimate_update(slot board) {
    for (int i = 1; i <= 10; i++) {
        if (i <= ULTIMATE_CHARGE) {
            board.val[ROW - 3][(COL / 2) - 6 + i] = '#';
        } else {
            board.val[ROW - 3][(COL / 2) - 6 + i] = ' ';
        }
    }
}
// Update cooldown charge
void cooldown_update(slot board) {
    for (int i = 1; i <= 6; i++) {
        if (i <= FIRE_COOLDOWN) {
            if (i <= FIRE_COOLDOWN_CHARGE) {
                board.val[ROW - 3][COL - 11 + i] = '#';
            } else {
                board.val[ROW - 3][COL - 11 + i] = ' ';
            }
        } else {
            board.val[ROW - 3][COL - 11 + i] = '-';
        }
    }
}
// Radius of power force (8 rows)
void ultimate_power(int x, slot board) {
    for (int i = 1; i <= 8; i++) {
        for (int j = x; j <= x + 1; j++) {
            if (board.type[i][j] != NONE && board.type[i][j] != ULTIMATE) {
                board.type[i][j] = NONE;
                board.val[i][j] = ' ';
            }
        }
    }
}
// Remove the leftover
void ultimate_remove(int y, slot board) {
    for (int i = y - 1; i <= y + 1; i++) {
        for (int j = PLAYER_POS + 1; j <= COL - 1; j++) {
            if (board.type[i][j] == ULTIMATE) {
                board.type[i][j] = NONE;
                board.val[i][j] = ' ';
            }
        }
    }
    for (int i = 0; i <= 11; i++)
        board.type[ROW - 3][(COL / 2) - 6 + i] = ULTIMATE;

    ULTIMATE_CHARGE = 0;
    IS_ULTIMATE_FIRED = false;
}
// Kill Bird (5*10 radius)
void kill_bird(int y, int x, slot board) {
    if (ULTIMATE_CHARGE < 10) {
        ULTIMATE_CHARGE += 1;
        if (ULTIMATE_CHARGE == 10) {
            for (int i = 0; i <= 11; i++)
                board.type[ROW - 3][(COL / 2) - 6 + i] = PLAYER;
        }
    }
    for (int i = y - 2; i < y + 3; i++) {
        for (int j = x - 3; j < x + 7; j++) {
            if (board.type[i][j] == BIRD || board.type[i][j] == AMMO) {
                board.type[i][j] = NONE;
                board.val[i][j] = ' ';
            }
        }
    }
}

// Tick execution.
void tick_loop(slot board, jump_parameters* jump, score_tab* score, int* obstacle_time, int* last_obstacle, int* bird_time, int* last_bird) {
    // Difficulty Evolution
    if (TICKS == 150) {
        GAME_PHASE = MEDIUM;
    }
    if (TICKS % 300 == 0 && TICKS != 0 && GAME_PHASE != HELL) {
        GAME_PHASE += 1;
        FIRE_COOLDOWN += 1;
    }

    // Enemy Generation
    if (*last_obstacle % *obstacle_time == 0) {
        int enemy_type = rand() % (min(GAME_PHASE, HELL - 1) + 1);
        if (enemy_type == 0) {
            place(COL - 7, 1, board, OBSTACLE, S_ENEMY1, 2, 3);
        } else if (enemy_type == 1) {
            place(COL - 7, 1, board, OBSTACLE, S_ENEMY2, 3, 3);
        } else if (enemy_type == 2) {
            place(COL - 7, 1, board, OBSTACLE, S_ENEMY3, 3, 6);
        }
        *obstacle_time = 30 + rand() % 20;
        *last_obstacle = 0;
    }
    if (GAME_PHASE != EASY && *last_bird % *bird_time == 0) {
        place(COL - 8, 4 + rand() % 2, board, BIRD, S_BIRD, 3, 6);
        *bird_time = 15 * (1 + HELL - GAME_PHASE) + rand() % 60;
        *last_bird = 0;
    }

    // Temp vars for loop
    jump_parameters jump_temp = *jump;
    int mrow = 8;
    bool monte = jump_temp.isJumping && jump_temp.jumpPhase < jump_temp.jumpHeight;
    bool descend = jump_temp.isJumping && jump_temp.jumpPhase >= jump_temp.jumpLength - jump_temp.jumpHeight && jump_temp.jumpPhase < jump_temp.jumpLength;
    register int i = 0;
    register int j = 0;

    for (i = mrow - 1; i >= 0; i--) {
        // Temp board row
        char* i_val = board.val[i];
        int* i_type = board.type[i];
        for (j = COL - 1; j >= 0; j--) {
            // Most used values
            char c_val = i_val[j];
            int c_type = i_type[j];
            int c_1 = j + 1;
            int c_n1 = COL - c_1;
            if (c_val != ' ') {
                if (c_type == ULTIMATE /*ultimate firing*/) {
                    if (j != COL - 1) {
                        i_val[c_1] = c_val;
                        i_type[c_1] = ULTIMATE;
                        ultimate_power(j, board);
                    } else {
                        ultimate_remove(i, board);
                    }
                } else if (c_type == AMMO /*ammo firing*/) {
                    i_val[j] = ' ';
                    i_type[j] = NONE;
                    if (j != COL - 1) {
                        if (i_type[c_1] == NONE) {
                            i_val[c_1] = c_val;
                            i_type[c_1] = AMMO;
                        } else if (i_type[c_1] == BIRD) {
                            kill_bird(i, c_1, board);
                        }
                    }
                }
            }
            if (i_val[c_n1] != ' ') {
                if (i_type[c_n1] == BIRD /*bird movement*/) {
                    char bird = i_val[c_n1];
                    i_val[c_n1] = ' ';
                    i_type[c_n1] = NONE;
                    if (j != 0) {
                        if (i_type[c_n1 - 1] == AMMO || i_type[c_n1 - 2] == AMMO) {
                            kill_bird(i, c_n1 - 1, board);
                        } else if (i_type[c_n1 - 1] == NONE && i_type[c_n1 - 2] == NONE) {
                            i_val[c_n1 - 2] = bird;
                            i_type[c_n1 - 2] = BIRD;
                        } else if (i_type[c_n1 - 1] == PLAYER || i_type[c_n1 - 2] == PLAYER) {
                            IS_GAME_OVER = true;
                        }
                    }
                } else if (i_type[c_n1] == OBSTACLE /*obstacle movement*/) {
                    char obs = i_val[c_n1];
                    i_val[c_n1] = ' ';
                    i_type[c_n1] = NONE;
                    if (j != 0) {
                        if (i_type[c_n1 - 1] != PLAYER) {
                            i_val[c_n1 - 1] = obs;
                            i_type[c_n1 - 1] = OBSTACLE;
                        } else if (i_type[c_n1 - 1] == PLAYER) {
                            IS_GAME_OVER = true;
                        }
                    }
                }
            }
            if (ANIMATION) {
                if (i == 0 && c_type == FLOOR /*infinite floor effect*/) {
                    i_val[j] = (i_val[j] == '_') ? '-' : '_';
                } else if (j < PLAYER_POS && !jump_temp.isJumping && TICKS % 5 == 0 && c_type == PLAYER && (c_val == '/' || c_val == '\\') /*player animation*/) {
                    i_val[j] = (i_val[j] == '/') ? '\\' : '/';
                }
            }

            if (!IS_ULTIMATE_FIRED) {
                if (c_type == PLAYER /*jump movement (1st phase)*/) {
                    if (monte && TICKS % 2 == 0) {
                        i_val[j] = ' ';
                        i_type[j] = NONE;
                        if (board.type[i + 1][j] != OBSTACLE) {
                            board.val[i + 1][j] = c_val;
                            board.type[i + 1][j] = PLAYER;
                        } else {
                            IS_GAME_OVER = true;
                        }
                    }
                }
                if (board.type[mrow - 1 - i][j] == PLAYER /*jump movement (3st phase)*/) {
                    if (descend && TICKS % 2 == 0) {
                        char c = board.val[mrow - 1 - i][j];
                        board.val[mrow - 1 - i][j] = ' ';
                        board.type[mrow - 1 - i][j] = NONE;
                        if (board.type[mrow - 2 - i][j] != OBSTACLE) {
                            board.val[mrow - 2 - i][j] = c;
                            board.type[mrow - 2 - i][j] = PLAYER;
                        } else {
                            IS_GAME_OVER = true;
                        }
                    }
                }
            }
        }
    }
    if (!IS_ULTIMATE_FIRED && jump_temp.isJumping && TICKS % 2 == 0 /*update jump status*/) {
        if (jump_temp.jumpPhase == jump_temp.jumpLength) {
            (*jump).isJumping = false;
            (*jump).jumpPhase = 0;
        } else {
            if (monte) {
                PLAYER_HEIGHT += 1;
            } else if (descend) {
                PLAYER_HEIGHT -= 1;
            }
            (*jump).jumpPhase += 1;
        }
    }
    if (!IS_GAME_OVER /*score update*/) {
        (*score).score += 1;
        score_update(board, score);
        ultimate_update(board);
        cooldown_update(board);
    }
}
// Evaluate the pressed character
void compute_entry(int n, slot board, jump_parameters* jump) {
    if (!IS_ULTIMATE_FIRED && FIRE_COOLDOWN_CHARGE == FIRE_COOLDOWN && (n == 72 || n == 40) /*ammo (X / x)?*/) {
        board.val[2 + PLAYER_HEIGHT][PLAYER_POS + 1] = '>';
        board.type[2 + PLAYER_HEIGHT][PLAYER_POS + 1] = AMMO;
        FIRE_COOLDOWN_CHARGE = 0;
    } else if (ULTIMATE_CHARGE == 10 && (n == 22 || n == 54) /*ultimate fire (F / f)*/) {
        board.val[2 + PLAYER_HEIGHT][PLAYER_POS + 1] = '=';
        board.type[2 + PLAYER_HEIGHT][PLAYER_POS + 1] = ULTIMATE;
        board.val[1 + PLAYER_HEIGHT][PLAYER_POS + 1] = '=';
        board.type[1 + PLAYER_HEIGHT][PLAYER_POS + 1] = ULTIMATE;
        IS_ULTIMATE_FIRED = true;
    } else if (n == -16 /*jump*/) {
        if ((*jump).isJumping != true) {
            (*jump).isJumping = true;
        }
    } else if (n == 24 || n == 56 /*light mode (H / h)*/) {
        COLOR = !COLOR;
        // ANIMATION = !ANIMATION;
    }
}

// Death Animation
void player_death(slot board, bool* death) {
    // Delete player part by part
    for (int i = ROW - 1; i >= 0; i--) {
        for (int j = 0; j < COL; j++) {
            if (board.type[i][j] == PLAYER) {
                board.val[i][j] = ' ';
                board.type[i][j] = NONE;
                return;
            }
        }
    }

    // Game Over Screen
    load_sprite(COL / 2 - 26, ROW / 2 - 1, board, GAME_OVER, "sprite_game_over.txt");
    load_sprite(COL / 2 - 8, ROW / 2 - 3, board, MENU, "sprite_jump_to_start.txt");
    load_sprite(COL / 2 - 8, ROW / 2 - 4, board, MENU, "l_to_leaderboard.txt");
    print_board(board);
    *death = false;
}

// Sort score array
void sort_score(score_tab* score) {
    int c = -1;
    for (int j = 1; j < MAX_PLAYER && c != 0; j++) {
        c = 0;
        for (int i = 1; i < MAX_PLAYER; i++) {
            if ((*score).bestScore[(*score).ids[i]].bestScore > (*score).bestScore[(*score).ids[i - 1]].bestScore) {
                swap((*score).ids, i, i - 1);
                c++;
            }
        }
    }
}
// Print leaderboard position
void print_pos(int row, int col, bool reverse, int pos, slot board) {
    int len = lenHelper(pos);
    col = reverse ? col - len : col;

    board.val[row][col] = '(';
    board.type[row][col] = DISPLAY;
    board.val[row][col + len + 1] = ')';
    board.type[row][col + len + 1] = DISPLAY;
    for (int j = 0; j < len; j++) {
        board.val[row][col + 1 + j] = 48 + (pos / ipow(10, len - 1 - j)) % 10;
        board.type[row][col + 1 + j] = DISPLAY;
    }
}
// Get player position in the leaderboard
void score_pos(score_tab* score) {
    for (int i = 0; i < MAX_PLAYER; i++) {
        if ((*score).ids[i] == ID) {
            SCORE_POS = i;
        }
    }
}
// Print the leaderboard
void leaderboard(slot board, score_tab* score) {
    sort_score(score);
    score_pos(score);
    load_sprite(COL / 2 - 27, ROW - 10, board, MENU, "leaderboard.txt");
    load_sprite(COL / 2 - 8, 0, board, MENU, "sprite_jump_to_start.txt");

    int len1, len2, len, offset, id, x, y, type;
    bool isUser = false;
    int total = min(MAX_PLAYER, 19);

    int relative_score;

    for (int i = 0; i < total; i++) {
        // Force user to appear even if last
        id = !isUser && i == 18 ? ID : (*score).ids[i];
        isUser = id == ID || isUser;
        // Player highlighting
        type = id == ID ? FLOOR : LEADERBOARD;

        relative_score = (*score).bestScore[id].bestScore / SCORE_DIVISOR;

        len1 = strlen((*score).bestScore[id].pseudo);
        len2 = lenHelper(relative_score);
        len = len1 + len2 + 3;

        // Display in columns
        offset = i == 0 ? len / 2 : (i <= total / 2 ? len + 4 : -4);
        y = i == 0 ? ROW - 12 : (i <= total / 2 ? ROW - 13 - i : ROW - 13 - (i - total / 2));
        x = COL / 2 - offset;

        // Print positions
        if (i != 0) {
            int p = id == ID ? SCORE_POS + 1 : i + 1;
            if (i <= total / 2) {
                print_pos(y, (COL / 2) - len - 7, true, p, board);
            } else {
                print_pos(y, (COL / 2) + len + 5, false, p, board);
            }
        }
        // Print Player : Score
        for (int j = 0; j < len1; j++) {
            board.val[y][x + j] = (*score).bestScore[id].pseudo[j];
            board.type[y][x + j] = type;
        }

        board.val[y][x + len1 + 1] = ':';
        board.type[y][x + len1 + 1] = type;

        for (int j = 0; j < len2; j++) {
            board.val[y][x + len - len2 + j] = 48 + (relative_score / ipow(10, len2 - 1 - j)) % 10;
            board.type[y][x + len - len2 + j] = type;
        }
    }
    print_board(board);
}

// Use ESCAPE to quit
void game_loop(slot board, double* refresh_rate, score_tab* score) {
    // Time variables
    struct timeval last, now;

    // Var init
    int obstacle_time, last_obstacle, bird_time, last_bird;
    jump_parameters jump = {false, 4, 12, 0};
    bool death = false;
    int n = 0;

    // Init time
    gettimeofday(&last, NULL);

    while (n != -21 /*esc char*/) {
        n = getch() - 48;

        if (!IS_GAME_OVER)
            compute_entry(n, board, &jump);

        gettimeofday(&now, NULL);
        double time_taken = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec) * 1e-6;

        if (time_taken > *refresh_rate /*Update Loop*/) {
            if (!IS_GAME_OVER /*Tick Loop*/) {
                tick_loop(board, &jump, score, &obstacle_time, &last_obstacle, &bird_time, &last_bird);
                TICKS += 1;
                FIRE_COOLDOWN_CHARGE += FIRE_COOLDOWN_CHARGE < FIRE_COOLDOWN ? 1 : 0;
                last_obstacle += 1; /*2* ?*/
                last_bird += 1;
            } else if (death /*Death Animation*/) {
                COLOR = 1; /*I like colors*/
                ANIMATION = 1;
                player_death(board, &death);
            }

            if (!IS_GAME_OVER || death /*Screen Update*/) {
                print_board(board);
                last = now;
            } else if (n == -16 /*Restart a game ? Why not !*/) {
                GAME_PHASE = EASY;
                FIRE_COOLDOWN_CHARGE = FIRE_COOLDOWN = 4;

                jump.jumpHeight = PLAYER_POS = 4;
                jump.jumpLength = 12;
                jump.jumpPhase = TICKS = (*score).score = n = IS_GAME_OVER = ULTIMATE_CHARGE = IS_ULTIMATE_FIRED = 0;
                jump.isJumping = death = PLAYER_HEIGHT = obstacle_time = last_obstacle = bird_time = last_bird = 1;

                init_board(board);
            } else if (n == 28 || n == 60 /*Leaderboard (L/l)*/) {
                init_board(board);
                leaderboard(board, score);
            }
        }
    }
}

// Diapositive for presentation
void intro(slot board) {
    FILE* file = fopen("animation.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    struct timespec tim, tim2;
    struct timeval last, now;
    gettimeofday(&last, NULL);
    gettimeofday(&now, NULL);
    double time_taken = 0;
    tim.tv_sec = 0;
    tim.tv_nsec = 20000000L;
    for (int diapo = 0; diapo < 23; diapo++) {
        time_taken = 0;
        int t = PLAYER;
        char c = fgetc(file);
        int horse_max_pos = 0;

        while (c != ',') {
            horse_max_pos = horse_max_pos * 10 + (c - 48);
            c = fgetc(file);
        }

        if (diapo == 0)
            horse_max_pos += 4;
        c = fgetc(file);
        c = fgetc(file);
        for (int i = ROW - 1; i >= 0; i--) {
            if (i == 0) {
                t = FLOOR;
            } else {
                if (diapo == 0 && i > 3) {
                    t = NONE;
                } else {
                    t = PLAYER;
                }
            }
            for (int j = 0; j < COL; j++) {
                if (j >= horse_max_pos && i != 0)
                    t = GAME_OVER;
                c = fgetc(file);
                board.val[i][j] = c;
                board.type[i][j] = t;
            }
            c = fgetc(file);
            if (c != EOF)
                c = fgetc(file);
        }
        gettimeofday(&last, NULL);
        double wait_time = (diapo == 0 || diapo == 22) ? 2 : ((diapo < 10) ? 0.15 : 0.3);
        tim.tv_nsec = (diapo == 0 || diapo == 22) ? 200000000L : ((diapo < 10) ? 15000000L : 30000000L);
        print_board(board);
        while (time_taken <= wait_time) {
            if (nanosleep(&tim, &tim2) < 0) {
                printf("Nano sleep system call failed \n");
                return;
            }
            gettimeofday(&now, NULL);
            time_taken = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec) * 1e-6;
        }
    }

    fclose(file);
}
// Ask politely for your pseudonym
void ask_username(slot board) {
    init_board(board);
    load_sprite(COL / 2 - 27, ROW / 2 - 3, board, MENU, "sprite_username.txt");

    int sc = -1;
    int taille = -1;
    int c;
    while (sc <= 0 || (taille < 3 || taille > 10)) {
        print_board(board);
        // 4-10 letters/numerals
        sc = scanf("%18[0-9A-Za-z]%n", USERNAME, &taille);
        printf("%s", USERNAME);
        // Flush stdin
        while ((c = fgetc(stdin)) != '\n' && c != EOF);
    }
}

int main() {
    // Global Variable For Timers
    double refresh_rate = 0.05;
    score_tab score = init_scores();

    IS_GAME_OVER = true;
    srand(time(NULL));

    // Init Board
    slot board = generate_board();
    PLAYER_POS = 4;

    ask_username(board);
    printf("\33[?25l");
    getBestScore(score);

    // Preloading for reused sprites
    S_ENEMY1 = load("sprite_enemy0.txt");
    S_ENEMY2 = load("sprite_enemy1.txt");
    S_ENEMY3 = load("sprite_enemy2.txt");
    S_BIRD = load("sprite_piaf.txt");

    // Welcome Screen
    intro(board);
    init_board(board);
    load_sprite(COL / 2 - 43, ROW / 2 - 5, board, MENU, "sprite_jump_to_start_big.txt");
    print_board(board);

    game_loop(board, &refresh_rate, &score);
}
/// Les milles et une ligne !