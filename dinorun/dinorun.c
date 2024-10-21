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
#define red() printf("\x1b[31;1m");
#define gray() printf("\x1b[30;1m");
#define green() printf("\033[1;32m");
#define cyan() printf("\033[0;36m");
#define yellow() printf("\x1b[33;1m");
#define light_blue() printf("\x1b[34;1m");
#define reset() printf("\033[0m");

const int DISPLAY_POS = 2;
char USERNAME[20];
int MAX_PLAYER = 0;
int ID = -1;
int GAME_PHASE;
int SCORE_POS;

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
    char val;
    int type;
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
    LEADERBOARD
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

slot** generate_board(int row, int col) {
    slot** board = malloc(sizeof(slot*) * row);
    for (int i = 0; i < row; i++) {
        slot* col_ = malloc(sizeof(slot) * col);
        for (int j = 0; j < col; j++) {
            slot slot_ = {' ', NONE};
            col_[j] = slot_;
        }
        board[i] = col_;
    }
    return board;
}

// Prints the game board.
void print_board(int row, int col, slot** board) {
    clear_screen();
    for (int i = row - 1; i >= 0; i--) {
        for (int j = 0; j < col; j++) {
            if (board[i][j].type == OBSTACLE) {
                gray();
                printf("%c", board[i][j].val);
                reset();
            } else if (board[i][j].type == FLOOR) {
                green();
                printf("%c", board[i][j].val);
                reset();
            } else if (board[i][j].type == DISPLAY) {
                cyan();
                printf("%c", board[i][j].val);
                reset();
            } else if (board[i][j].type == GAME_OVER) {
                red();
                printf("%c", board[i][j].val);
                reset();
            } else if (board[i][j].type == PLAYER || board[i][j].type == LEADERBOARD) {
                yellow();
                printf("%c", board[i][j].val);
                reset();
            } else if (board[i][j].type == MENU) {
                light_blue();
                printf("%c", board[i][j].val);
                reset();
            } else {
                printf("%c", board[i][j].val);
            }
        }
        printf("\n");
    }
}

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

// Load a sprite from a file on the board at specific coordinates
int load_sprite(int x, int y, slot** board, int t, char* sprite) {
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
            if (board[i + y][j + x].type == NONE && c != ' ') {
                board[i + y][j + x].val = c;
                board[i + y][j + x].type = t;
            }
        }
        c = fgetc(file);
        if (c != EOF) {
            c = fgetc(file);
        }
    }

    fclose(file);
    return x + length - 1;
}

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
void init_board(int* pos, int row, int col, slot** board, bool gameOver) {
    for (int i = 0; i < row; i++ /*waw ... space*/) {
        for (int j = 0; j < col; j++) {
            board[i][j].val = ' ';
            board[i][j].type = NONE;
        }
    }

    if (!gameOver) {
        for (int j = 0; j < col; j++ /*build floor*/) {
            board[0][j].val = j % 2 == 0 ? '-' : '_';
            board[0][j].type = FLOOR;
        }
        load_sprite(DISPLAY_POS, row - 2, board, DISPLAY, "sprite_best_score.txt");
        load_sprite(DISPLAY_POS, row - 3, board, DISPLAY, "sprite_score.txt");
        *pos = load_sprite(*pos, 1, board, PLAYER, "sprite_player.txt");
    }
}

// Graphic Update
void score_update(int row, slot** board, score_tab* score) {
    int i = 0;
    int _score = (*score).score;
    int _bestScore = (*score).bestScore[ID].bestScore;
    if (_score > _bestScore) {
        (*score).bestScore[ID].bestScore = _score;
        _bestScore = _score;
        saveBestScore(*score);
    }
    _score /= 2;
    _bestScore /= 2;
    for (i = 0; _score / ipow(10, i) != 0; i++) {
    }
    for (int j = 1; j <= i; j++) {
        board[row - 3][DISPLAY_POS + 7 + j].val = 48 + (_score / ipow(10, i - j)) % 10;
        board[row - 3][DISPLAY_POS + 7 + j].type = DISPLAY;
    }
    for (i = 0; _bestScore / ipow(10, i) != 0; i++) {
    }
    for (int j = 1; j <= i; j++) {
        board[row - 2][DISPLAY_POS + 16 + j].val = 48 + (_bestScore / ipow(10, i - j)) % 10;
        board[row - 2][DISPLAY_POS + 16 + j].type = DISPLAY;
    }
}

// Tick execution.
void tick_loop(int row, int col, slot** board, int* height, jump_parameters* jump, score_tab* score, bool* gameOver, int* ticks, int* obstacle_time, int* last_obstacle) {
    if (*ticks % 300 == 0 && *ticks != 0 && GAME_PHASE != HARD)
        GAME_PHASE += 1;
    if (*last_obstacle % *obstacle_time == 0) {
        char sprite[20];
        sprintf(sprite, "sprite_enemy%1d.txt", rand() % (GAME_PHASE + 1));
        load_sprite(col - 7, 1, board, OBSTACLE, sprite);
        *obstacle_time = 30 + rand() % 20;
        *last_obstacle = 0;
    }
    jump_parameters jump_temp = *jump;
    int mrow = 8;
    for (int i = mrow - 1; i >= 0; i--) {
        for (int j = col - 1; j >= 0; j--) {
            if (board[i][j].type == AMMO /*ammo firing*/) {
                char ammo = board[i][j].val;
                board[i][j].val = ' ';
                board[i][j].type = NONE;
                if (j != col - 1 && (board[i][j + 1].type == BIRD || board[i][j + 1].type == NONE)) {
                    board[i][j + 1].val = ammo;
                    board[i][j + 1].type = AMMO;
                }
            }
            if (board[i][col - 1 - j].type == OBSTACLE /*obstacle movement*/) {
                char obs = board[i][col - 1 - j].val;
                board[i][col - 1 - j].val = ' ';
                board[i][col - 1 - j].type = NONE;
                if (j != 0) {
                    if (board[i][col - 2 - j].type != PLAYER) {
                        board[i][col - 2 - j].val = obs;
                        board[i][col - 2 - j].type = OBSTACLE;
                    } else if (board[i][col - 2 - j].type == PLAYER) {
                        *gameOver = true;
                    }
                }
            }

            if (board[i][j].type == FLOOR /*infinite floor effect*/) {
                board[i][j].val = (board[i][j].val == '_') ? '-' : '_';
            }
            if (board[i][j].type == PLAYER && (board[i][j].val == '/' || board[i][j].val == '\\') && !jump_temp.isJumping && *ticks % 5 == 0 /*player animation*/) {
                board[i][j].val = (board[i][j].val == '/') ? '\\' : '/';
            }
            if (board[i][j].type == PLAYER /*jump movement (1st phase)*/) {
                if (jump_temp.isJumping && jump_temp.jumpPhase < jump_temp.jumpHeight && *ticks % 2 == 0) {
                    char c = board[i][j].val;
                    board[i][j].val = ' ';
                    board[i][j].type = NONE;
                    if (board[i + 1][j].type != OBSTACLE) {
                        board[i + 1][j].val = c;
                        board[i + 1][j].type = PLAYER;
                    } else {
                        *gameOver = true;
                    }
                }
            }
            if (board[mrow - 1 - i][j].type == PLAYER /*jump movement (3st phase)*/) {
                if (jump_temp.isJumping && jump_temp.jumpPhase >= jump_temp.jumpLength - jump_temp.jumpHeight && jump_temp.jumpPhase < jump_temp.jumpLength && *ticks % 2 == 0) {
                    char c = board[mrow - 1 - i][j].val;
                    board[mrow - 1 - i][j].val = ' ';
                    board[mrow - 1 - i][j].type = NONE;
                    if (board[mrow - 2 - i][j].type != OBSTACLE) {
                        board[mrow - 2 - i][j].val = c;
                        board[mrow - 2 - i][j].type = PLAYER;
                    } else {
                        *gameOver = true;
                    }
                }
            }
        }
    }
    if (jump_temp.isJumping && *ticks % 2 == 0 /*update jump status*/) {
        if (jump_temp.jumpPhase == jump_temp.jumpLength) {
            (*jump).isJumping = false;
            (*jump).jumpPhase = 0;
        } else {
            if (jump_temp.jumpPhase < jump_temp.jumpHeight) {
                *height += 1;
            } else if (jump_temp.jumpPhase >= jump_temp.jumpLength - jump_temp.jumpHeight && jump_temp.jumpPhase < jump_temp.jumpLength) {
                *height -= 1;
            }
            (*jump).jumpPhase += 1;
        }
    }
    if (!*gameOver /*score update*/) {
        (*score).score += 1;
        score_update(row, board, score);
    }
}

void compute_entry(int n, slot** board, int* pos, int* height, jump_parameters* jump) {
    if (n == 72 || n == 40 /*0 <= n && n <= 3*/ /*ammo (X / x)?*/) {
        board[2 + *height][*pos + 1].val = '>';
        board[2 + *height][*pos + 1].type = AMMO;
    } else if (n == -16 /*jump*/) {
        if ((*jump).isJumping != true) {
            (*jump).isJumping = true;
        }
    }
}

// Death Animation
void player_death(int row, int col, slot** board, bool* death) {
    for (int i = row - 1; i >= 0; i--) {
        for (int j = 0; j < col; j++) {
            if (board[i][j].type == PLAYER) {
                board[i][j].val = ' ';
                board[i][j].type = NONE;
                return;
            }
        }
    }
    load_sprite(col / 2 - 26, row / 2 - 1, board, GAME_OVER, "sprite_game_over.txt");
    load_sprite(col / 2 - 8, row / 2 - 3, board, MENU, "sprite_jump_to_start.txt");
    load_sprite(col / 2 - 8, row / 2 - 4, board, MENU, "l_to_leaderboard.txt");
    print_board(row, col, board);
    *death = false;
}

void swap(int* arr, int i, int j) {
    int mem = arr[j];
    arr[j] = arr[i];
    arr[i] = mem;
}

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

int min(int i1, int i2) {
    return (i1 < i2) ? i1 : i2;
}

void print_pos(int row, int col, bool reverse, int pos, slot** board) {
    int len = lenHelper(pos);
    col = reverse ? col - len : col;

    board[row][col].val = '(';
    board[row][col].type = DISPLAY;
    board[row][col + len + 1].val = ')';
    board[row][col + len + 1].type = DISPLAY;
    for (int j = 0; j < len; j++) {
        board[row][col + 1 + j].val = 48 + (pos / ipow(10, len - 1 - j)) % 10;
        board[row][col + 1 + j].type = DISPLAY;
    }
}

void score_pos(score_tab* score) {
    for (int i = 0; i < MAX_PLAYER; i++) {
        if ((*score).ids[i] == ID) {
            SCORE_POS = i;
        }
    }
}

void leaderboard(int row, int col, slot** board, score_tab* score) {
    sort_score(score);
    score_pos(score);
    load_sprite(col / 2 - 27, row - 10, board, MENU, "leaderboard.txt");
    load_sprite(col / 2 - 8, 0, board, MENU, "sprite_jump_to_start.txt");

    int len1, len2, len, offset, id, x, y, type;
    bool isUser = false;
    int total = min(MAX_PLAYER, 19);
    for (int i = 0; i < total; i++) {
        id = !isUser && i == 18 ? ID : (*score).ids[i];
        isUser = id == ID || isUser;
        type = id == ID ? FLOOR : LEADERBOARD;

        len1 = strlen((*score).bestScore[id].pseudo);
        len2 = lenHelper((*score).bestScore[id].bestScore);
        len = len1 + len2 + 3;

        offset = i == 0 ? len / 2 : (i <= total / 2 ? len + 4 : -4);
        y = i == 0 ? row - 12 : (i <= total / 2 ? row - 13 - i : row - 13 - (i - total / 2));
        x = col / 2 - offset;
        if (i != 0) {
            int p = id == ID ? SCORE_POS + 1 : i + 1;
            if (i <= total / 2) {
                print_pos(y, (col / 2) - len - 7, true, p, board);
            } else {
                print_pos(y, (col / 2) + len + 5, false, p, board);
            }
        }
        for (int j = 0; j < len1; j++) {
            board[y][x + j].val = (*score).bestScore[id].pseudo[j];
            board[y][x + j].type = type;
        }

        board[y][x + len1 + 1].val = ':';
        board[y][x + len1 + 1].type = type;

        for (int j = 0; j < len2; j++) {
            board[y][x + len - len2 + j].val = 48 + ((*score).bestScore[id].bestScore / ipow(10, len2 - 1 - j)) % 10;
            board[y][x + len - len2 + j].type = type;
        }
    }
    print_board(row, col, board);
}

// Use ESCAPE to quit
void game_loop(int* pos, int row, int col, slot** board, double* refresh_rate, score_tab* score, bool* gameOver, int* ticks) {
    // Time variables
    struct timeval last, now;

    int height, obstacle_time, last_obstacle;
    jump_parameters jump = {false, 4, 12, 0};
    bool death = false;
    int n = 0;

    // Init time
    gettimeofday(&last, NULL);

    while (n != -21 /*esc char*/) {
        n = getch() - 48;

        if (!*gameOver)
            compute_entry(n, board, pos, &height, &jump);

        gettimeofday(&now, NULL);
        double time_taken = (now.tv_sec - last.tv_sec) + (now.tv_usec - last.tv_usec) * 1e-6;

        if (time_taken > *refresh_rate /*Update Loop*/) {
            if (!*gameOver /*Tick Loop*/) {
                tick_loop(row, col, board, &height, &jump, score, gameOver, ticks, &obstacle_time, &last_obstacle);
                *ticks += 1;
                last_obstacle += 1; /*2* ?*/
            } else if (death /*Death Animation*/)
                player_death(row, col, board, &death);

            if (!*gameOver || death /*Screen Update*/) {
                print_board(row, col, board);
                last = now;
            } else if (n == -16 /*Restart a game ? Why not !*/) {
                GAME_PHASE = EASY;

                jump.jumpHeight = *pos = 4;
                jump.jumpLength = 12;
                jump.jumpPhase = *ticks = (*score).score = n = *gameOver = 0;
                jump.isJumping = death = height = obstacle_time = last_obstacle = 1;

                init_board(pos, row, col, board, *gameOver);
            } else if (n == 28 || n == 60) {
                init_board(pos, row, col, board, gameOver);
                leaderboard(row, col, board, score);
            }
        }
    }
}

void ask_username(int* pos, int row, int col, slot** board, bool gameOver) {
    init_board(pos, row, col, board, gameOver);
    load_sprite(col / 2 - 27, row / 2 - 3, board, MENU, "sprite_username.txt");

    int sc = -1;
    int taille = -1;
    int c;
    while (sc <= 0 || (taille < 3 || taille > 10)) {
        print_board(row, col, board);
        sc = scanf("%18[0-9A-Za-z]%n", USERNAME, &taille);
        printf("%s", USERNAME);
        while ((c = fgetc(stdin)) != '\n' && c != EOF);
    }
}

int main() {
    // Global Variable For Timers
    double refresh_rate = 0.05;
    score_tab score = init_scores();

    bool gameOver = true;
    int ticks;

    srand(time(NULL));

    // Init Board
    int row = 24;
    int col = 100;
    slot** board = generate_board(row, col);

    int pos = 4;

    ask_username(&pos, row, col, board, gameOver);

    printf("\33[?25l");

    getBestScore(score);

    // Welcome Screen
    init_board(&pos, row, col, board, gameOver);
    load_sprite(col / 2 - 43, row / 2 - 5, board, MENU, "sprite_jump_to_start_big.txt");
    print_board(row, col, board);

    game_loop(&pos, row, col, board, &refresh_rate, &score, &gameOver, &ticks);
}