#include "parser.h"

#include <stdio.h>

#include "entity.h"

/// @brief Read and parse the given file using the dodjo format to update the chunk decorations
/// @param d Array of chunk decorations/items
/// @param filename file to read
void parse_chunk_file(chunk* c, dynarray* d, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    int x, y, type, display, row_repeat, size, col_repeat, entity_type;
    while (fscanf(file, "%d,%d,%d,%d,%d,%d,%d,%d", &x, &y, &type, &display, &row_repeat, &size, &col_repeat, &entity_type) == 8) {
        if (!entity_type) {
            for (int i = 0; i < row_repeat; i++) {
                for (int j = 0; j < col_repeat; j++) {
                    append(d, generate_item(x - (1 + size) * i, y - j, type, display, len_dyn(d)));
                }
            }
        } else {
            char entity_filename[32];
            snprintf(entity_filename, sizeof(entity_filename), "assets/entities/data/%d.dodjo", entity_type);
            FILE* entity_file = fopen(entity_filename, "r");
            if (entity_file == NULL) {
                perror("Error opening file");
                return;
            }
            item* brain = generate_item(x, y, type, display, -1);
            entity* e = create_entity(brain, c);
            switch (type) {
                case ENEMY: {
                    enemy* elt = malloc(sizeof(enemy));

                    fscanf(entity_file, "*%d*", &elt->hp);
                    specialize(brain, false, false, elt);
                    break;
                }

                default:
                    break;
            }

            int nx, ny;
            while (fscanf(entity_file, "%d,%d,%d,%d,%d", &nx, &ny, &display, &row_repeat, &col_repeat) == 5) {
                for (int i = 0; i < row_repeat; i++) {
                    for (int j = 0; j < col_repeat; j++) {
                        item* it = generate_item(nx + x - (1 + 1) * i, ny + y - j, type, display, len_dyn(d));
                        add_entity_part(e, it);
                        link_entity(it, e);
                        append(d, it);
                    }
                }
            }
            fclose(entity_file);
        }
    }
    fclose(file);
}

//? TO ADD A LEVEL: modify
void parse_chunk(chunk* c, dynarray* d, enum ChunkType type) {
    switch (type) {
        // case DUMMY:
        //     parse_chunk_file(d, "assets/chunks/DUMMY.dodjo");
        //     break;
        case SPAWN:
            parse_chunk_file(c, d, "assets/chunks/spawn.dodjo");
            break;
        case DEFAULT2:
            parse_chunk_file(c, d, "assets/chunks/default2.dodjo");
            break;
        default:
            parse_chunk_file(c, d, "assets/chunks/default.dodjo");
            break;
    }
}