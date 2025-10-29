#include "parser.h"

#include <stdio.h>

#include "assets_manager.h"
#include "entity.h"

/// @brief Read and parse the given file using the dodjo format to update the chunk decorations
/// @param c chunk
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
                                  entry->usable_item,
                                  len_dyn(d)));
                }
            }
        } else {
            // Entity with multiple parts
            EntityAssetFile* entityFile = get_entity_file(entry->entity_type);
            if (entityFile == NULL) {
                fprintf(stderr, "Error: Unable to load entity file for type %d\n", entry->entity_type);
                continue;
            }

            // Create the brain item and entity
            item* brain = generate_item(entry->x, entry->y, entry->type, entry->display, entry->usable_item, -1);
            entity* e = create_entity(brain, c);

            // TODO: IMPLEMENT SPECIALIZE_ENTITY FUNCTION
            switch (entry->type) {
                case ENEMY: {
                    enemy* elt = malloc(sizeof(enemy));
                    elt->hp = entityFile->specs.specs[0];
                    elt->damage = entityFile->specs.specs[1];
                    elt->from_id = len_dyn(get_chunk_enemies(c));
                    elt->speed = entityFile->specs.specs[2];
                    elt->infinity = entityFile->specs.specs[3];
                    elt->score = entityFile->specs.specs[4];
                    elt->attack_delay = entityFile->specs.specs[5];
                    elt->attack_interval = entityFile->specs.specs[6];
                    specialize(brain, false, false, elt);
                    Color color = COLOR_DEFAULT;
                    if (entry->entity_type == ENEMY_BRONZE_1 || entry->entity_type == ENEMY_BRONZE_2) {
                        color = COLOR_RED;
                    } else if (entry->entity_type == ENEMY_SILVER_1 || entry->entity_type == ENEMY_SILVER_2) {
                        color = COLOR_CYAN;
                    } else if (entry->entity_type == ENEMY_GOLD_1 || entry->entity_type == ENEMY_GOLD_2) {
                        color = COLOR_YELLOW;
                    } else if (entry->entity_type == ENEMY_NADINO_1 || entry->entity_type == ENEMY_NADINO_2) {
                        color = COLOR_MAGENTA;
                    }

                    set_item_color(brain, color);
                    append(get_chunk_enemies(c), brain);
                    break;
                }
                case LOOTABLE: {
                    lootable* loot = malloc(sizeof(lootable));
                    loot->bronze = entityFile->specs.specs[0];
                    loot->silver = entityFile->specs.specs[1];
                    loot->gold = entityFile->specs.specs[2];
                    loot->nadino = entityFile->specs.specs[3];
                    specialize(brain, false, false, loot);
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
                            NOT_USABLE_ITEM,
                            len_dyn(d));
                        set_item_color(it, get_item_color(brain));
                        add_entity_part(e, it);
                        link_entity(it, e);
                        append(d, it);
                    }
                }
            }
        }
    }
}