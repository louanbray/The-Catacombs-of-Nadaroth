#include "chunk_parser.h"

#include <stdio.h>

#include "../game_objects/entity.h"
#include "../managers/assets_manager.h"
#include "../utils/logger.h"

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
                LOG_ERROR("Unable to load entity file for type %d\n", entry->entity_type);
                continue;
            }

            // Create the brain item and entity
            item* brain = generate_item(entry->x, entry->y, entry->type, entry->display, entry->usable_item, -1);
            entity* e = create_entity(brain, c);

            // TODO: IMPLEMENT SPECIALIZE_ENTITY FUNCTION
            switch (entry->type) {
                case ITEMTYPE_ENEMY: {
                    enemy* elt = malloc(sizeof(enemy));
                    elt->hp = entityFile->specs.specs[0];
                    elt->damage = entityFile->specs.specs[1];
                    elt->from_id = len_dyn(get_chunk_enemies(c));
                    elt->speed = entityFile->specs.specs[2];
                    elt->infinity = entityFile->specs.specs[3];
                    elt->score = entityFile->specs.specs[4];
                    elt->attack_delay = entityFile->specs.specs[5];
                    elt->attack_interval = entityFile->specs.specs[6];

                    elt->can_drop = (bool)entityFile->specs.specs[7];
                    elt->entity_type = entry->entity_type;

                    if (elt->can_drop) {
                        elt->loot = (lootable){
                            .key = USABLE_ITEM_NONE,
                            .none = entityFile->specs.specs[8],
                            .bronze = entityFile->specs.specs[9],
                            .silver = entityFile->specs.specs[10],
                            .gold = entityFile->specs.specs[11],
                            .nadino = entityFile->specs.specs[12],
                            .id = entityFile->specs.specs[13],
                        };
                    } else {
                        elt->loot = (lootable){0};
                    }

                    specialize(brain, false, false, elt);
                    Color color = COLOR_DEFAULT;
                    if (entry->entity_type == ENTITY_ENEMY_BRONZE_1 || entry->entity_type == ENTITY_ENEMY_BRONZE_2) {
                        color = COLOR_RED;
                    } else if (entry->entity_type == ENTITY_ENEMY_SILVER_1 || entry->entity_type == ENTITY_ENEMY_SILVER_2) {
                        color = COLOR_CYAN;
                    } else if (entry->entity_type == ENTITY_ENEMY_GOLD_1 || entry->entity_type == ENTITY_ENEMY_GOLD_2) {
                        color = COLOR_YELLOW;
                    } else if (entry->entity_type == ENTITY_ENEMY_NADINO_1 || entry->entity_type == ENTITY_ENEMY_NADINO_2) {
                        color = COLOR_MAGENTA;
                    }

                    set_item_color(brain, color);
                    append(get_chunk_enemies(c), brain);
                    break;
                }
                case ITEMTYPE_LOOTABLE: {
                    lootable* loot = malloc(sizeof(lootable));
                    loot->key = entityFile->specs.specs[0];
                    loot->none = entityFile->specs.specs[1];
                    loot->bronze = entityFile->specs.specs[2];
                    loot->silver = entityFile->specs.specs[3];
                    loot->gold = entityFile->specs.specs[4];
                    loot->nadino = entityFile->specs.specs[5];
                    loot->id = entityFile->specs.specs[6];
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
                            USABLE_ITEM_NOT_USABLE,
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
    if (chunkFile->can_free) free_assets_chunk(chunkFile);
}