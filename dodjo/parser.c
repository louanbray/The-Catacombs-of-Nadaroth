#include "parser.h"

#include <stdio.h>

void parse_chunk_file(dynarray* d, char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    int x = 0;
    int y = 0;
    int t = 0;
    char c = fgetc(file);
    while (c != EOF) {
        x = 0;
        bool m1 = c == '-';
        while (c != ',') {
            if (c != '-')
                x = x * 10 + (c - 48);
            c = fgetc(file);
        }
        c = fgetc(file);
        y = 0;
        bool m2 = c == '-';
        while (c != ',') {
            if (c != '-')
                y = y * 10 + (c - 48);
            c = fgetc(file);
        }
        c = fgetc(file);
        t = 0;
        bool m3 = c == '-';
        while (c != ',') {
            if (c != '-')
                t = t * 10 + (c - 48);
            c = fgetc(file);
        }
        x = m1 ? -x : x;
        y = m2 ? -y : y;
        t = m3 ? -t : t;
        item* i = create_item(x, y, t);
        append(d, i);
        c = fgetc(file);
        c = fgetc(file);
    }

    fclose(file);
}

void parse_chunk(dynarray* d, int type) {
    switch (type) {
        case SPAWN:
            parse_chunk_file(d, "assets/chunk_spawn.dodjo");
            break;
        default:
            parse_chunk_file(d, "assets/chunk_default.dodjo");
            break;
    }
}
void parse_item(item* i, int type) {}