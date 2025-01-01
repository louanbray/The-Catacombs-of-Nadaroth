#include "parser.h"

#include <stdio.h>

/// @brief Read and parse the given file using the dodjo format to update the chunk decorations
/// @param d Array of chunk decorations/items
/// @param filename file to read
void parse_chunk_file(dynarray* d, char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    int x, y, type, display, row_repeat, size, col_repeat;
    while (fscanf(file, "%d,%d,%d,%d,%d,%d,%d", &x, &y, &type, &display, &row_repeat, &size, &col_repeat) == 7) {
        for (int i = 0; i < row_repeat; i++) {
            for (int j = 0; j < col_repeat; j++) {
                append(d, generate_item(x - (1 + size) * i, y - j, type, display, len_dyn(d)));
            }
        }
    }
    fclose(file);
}

//? TO ADD A LEVEL: modify
void parse_chunk(dynarray* d, int type) {
    switch (type) {
        // case DUMMY:
        //     parse_chunk_file(d, "assets/chunks/DUMMY.dodjo");
        //     break;
        case SPAWN:
            parse_chunk_file(d, "assets/chunks/spawn.dodjo");
            break;
        case DEFAULT2:
            parse_chunk_file(d, "assets/chunks/default2.dodjo");
            break;
        default:
            parse_chunk_file(d, "assets/chunks/default.dodjo");
            break;
    }
}