#include "parser.h"

#include <stdio.h>

#include "assets_manager.h"
#include "entity.h"

/// @brief Read and parse the given file using the dodjo format to update the chunk decorations
/// @param c chunk
/// @param d Array of chunk decorations/items
/// @param chunk_type type of the chunk
//? TO ADD A LEVEL: modify
void parse_chunk(chunk* c, dynarray* d, ChunkType chunk_type) {
    ChunkAssetFile* chunkFile = get_chunk_file(chunk_type);
    if (!chunkFile) return;

    for (size_t i = 0; i < chunkFile->item_count; i++) {
        ChunkItem* entry = &chunkFile->items[i];

        if (!entry->entity_type) {
            // Regular decorations
            for (int row = 0; row < entry->row_repeat; row++) {
                for (int col = 0; col < entry->col_repeat; col++) {
                    append(d, generate_item(
                                  entry->x - (1 + entry->size) * row,
                                  entry->y - col,
                                  entry->type,
                                  entry->display,
                                  len_dyn(d)));
                }
            }
        } else {
            // Entity with multiple parts
            EntityAssetFile* entityFile;
            if (entityFile == NULL) {
                fprintf(stderr, "Error: Unable to load entity file for type %d\n", entry->entity_type);
                continue;
            }

            // Create the brain item and entity
            item* brain = generate_item(entry->x, entry->y, entry->type, entry->display, -1);
            entity* e = create_entity(brain, c);

            entityFile = get_entity_file(entry->entity_type);

            // TODO: IMPLEMENT SPECIALIZE_ENTITY FUNCTION
            switch (entry->type) {
                case ENEMY: {
                    enemy* elt = malloc(sizeof(enemy));
                    elt->hp = entityFile->specs.specs[0];
                    specialize(brain, false, false, elt);
                    break;
                }

                default:
                    break;
            }

            // Add parts of the entity
            for (int j = 0; j < entityFile->count; j++) {
                EntityPart* part = &entityFile->parts[j];
                for (int row = 0; row < part->row_repeat; row++) {
                    for (int col = 0; col < part->col_repeat; col++) {
                        item* it = generate_item(
                            entry->x + part->x - (1 + 1) * row,
                            entry->y + part->y - col,
                            entry->type,
                            part->display,
                            len_dyn(d));
                        add_entity_part(e, it);
                        link_entity(it, e);
                        append(d, it);
                    }
                }
            }
        }
    }
}