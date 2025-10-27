#include "save_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

#include "entity.h"
#include "generation.h"
#include "hash.h"
#include "inventory.h"
#include "item.h"
#include "logger.h"
#include "map.h"
#include "player.h"

#define SAVE_VERSION 3
#define SAVE_MAGIC 0x4E414430  // "NAD0" in hex

// Temporary storage for player's chunk coordinates during load
static int g_player_chunk_x = 0;
static int g_player_chunk_y = 0;

/// @brief Save player data to file
static bool save_player_data(FILE* f, player* p) {
    if (!f || !p) return false;

    // Save player position and state using getter functions
    int x = get_player_x(p);
    int y = get_player_y(p);
    int px = get_player_px(p);
    int py = get_player_py(p);
    int health = get_player_health(p);
    int max_health = get_player_max_health(p);
    int mental_health = get_player_mental_health(p);
    int damage = get_player_damage(p);
    int arrow_speed = get_player_arrow_speed(p);
    int range = get_player_range(p);
    bool infinity = has_infinity(p);
    int design = get_player_design(p);
    int score = get_player_score(p);
    int deaths = get_player_deaths(p);
    Color color = get_player_color(p);
    GamePhase phase = get_player_phase(p);

    // Save current chunk coordinates
    chunk* current = get_player_chunk(p);
    int chunk_x = get_chunk_x(current);
    int chunk_y = get_chunk_y(current);

    fwrite(&x, sizeof(int), 1, f);
    fwrite(&y, sizeof(int), 1, f);
    fwrite(&px, sizeof(int), 1, f);
    fwrite(&py, sizeof(int), 1, f);
    fwrite(&health, sizeof(int), 1, f);
    fwrite(&max_health, sizeof(int), 1, f);
    fwrite(&mental_health, sizeof(int), 1, f);
    fwrite(&damage, sizeof(int), 1, f);
    fwrite(&arrow_speed, sizeof(int), 1, f);
    fwrite(&range, sizeof(int), 1, f);
    fwrite(&infinity, sizeof(bool), 1, f);
    fwrite(&design, sizeof(int), 1, f);
    fwrite(&score, sizeof(int), 1, f);
    fwrite(&deaths, sizeof(int), 1, f);
    fwrite(&color, sizeof(int), 1, f);
    fwrite(&phase, sizeof(int), 1, f);
    fwrite(&chunk_x, sizeof(int), 1, f);
    fwrite(&chunk_y, sizeof(int), 1, f);

    return true;
}

/// @brief Load player data from file (pass 1: without chunk restoration)
static bool load_player_data(FILE* f, player* p) {
    if (!f || !p) return false;

    int x, y, px, py, health, max_health, mental_health, damage, arrow_speed, range;
    bool infinity;
    int design, score, deaths, color, phase;
    int chunk_x, chunk_y;

    fread(&x, sizeof(int), 1, f);
    fread(&y, sizeof(int), 1, f);
    fread(&px, sizeof(int), 1, f);
    fread(&py, sizeof(int), 1, f);
    fread(&health, sizeof(int), 1, f);
    fread(&max_health, sizeof(int), 1, f);
    fread(&mental_health, sizeof(int), 1, f);
    fread(&damage, sizeof(int), 1, f);
    fread(&arrow_speed, sizeof(int), 1, f);
    fread(&range, sizeof(int), 1, f);
    fread(&infinity, sizeof(bool), 1, f);
    fread(&design, sizeof(int), 1, f);
    fread(&score, sizeof(int), 1, f);
    fread(&deaths, sizeof(int), 1, f);
    fread(&color, sizeof(int), 1, f);
    fread(&phase, sizeof(int), 1, f);
    fread(&chunk_x, sizeof(int), 1, f);
    fread(&chunk_y, sizeof(int), 1, f);

    // Store chunk coordinates in globals for later restoration
    g_player_chunk_x = chunk_x;
    g_player_chunk_y = chunk_y;

    // Apply loaded data using setter functions
    set_player_x(p, x);
    set_player_y(p, y);
    set_player_px(p, px);
    set_player_py(p, py);
    set_player_damage(p, damage);
    set_player_max_health(p, max_health);
    set_player_mental_health(p, mental_health);
    set_player_arrow_speed(p, arrow_speed);
    set_player_range(p, range);
    set_player_infinity(p, infinity);
    set_player_design(p, design);
    set_player_score(p, score);
    set_player_color(p, (Color)color);
    set_player_phase(p, (GamePhase)phase);
    set_player_deaths(p, deaths);

    // Health needs special handling
    if (health < get_player_health(p)) {
        damage_player(p, get_player_health(p) - health);
    } else if (health > get_player_health(p)) {
        heal_player(p, health - get_player_health(p));
    }

    return true;
}

/// @brief Save hotbar data to file
static bool save_hotbar_data(FILE* f, hotbar* h) {
    if (!f || !h) return false;

    int max_size = get_hotbar_max_size(h);
    int selected_slot = get_selected_slot(h);

    // Save hotbar metadata
    fwrite(&max_size, sizeof(int), 1, f);
    fwrite(&selected_slot, sizeof(int), 1, f);

    // Save each slot
    for (int i = 0; i < max_size; i++) {
        item* it = get_hotbar(h, i);

        // Save item presence flag
        bool has_item = (it != NULL);
        fwrite(&has_item, sizeof(bool), 1, f);

        if (has_item) {
            // Save item data
            int item_x = get_item_x(it);
            int item_y = get_item_y(it);
            int item_type = get_item_type(it);
            int item_display = get_item_display(it);
            bool item_hidden = is_item_hidden(it);
            bool item_used = is_item_used(it);
            int item_usable_type = get_item_usable_type(it);

            fwrite(&item_x, sizeof(int), 1, f);
            fwrite(&item_y, sizeof(int), 1, f);
            fwrite(&item_type, sizeof(int), 1, f);
            fwrite(&item_display, sizeof(int), 1, f);
            fwrite(&item_hidden, sizeof(bool), 1, f);
            fwrite(&item_used, sizeof(bool), 1, f);
            fwrite(&item_usable_type, sizeof(int), 1, f);
        }
    }

    return true;
}

/// @brief Load hotbar data from file
static bool load_hotbar_data(FILE* f, hotbar* h) {
    if (!f || !h) return false;

    int max_size, selected_slot;
    fread(&max_size, sizeof(int), 1, f);
    fread(&selected_slot, sizeof(int), 1, f);

    if (max_size != get_hotbar_max_size(h)) {
        LOG_ERROR("Hotbar size mismatch in save file");
        return false;
    }

    // Load each slot
    for (int i = 0; i < max_size; i++) {
        bool has_item;
        fread(&has_item, sizeof(bool), 1, f);

        if (has_item) {
            int item_x, item_y, item_type, item_display, item_usable_type;
            bool item_hidden, item_used;

            fread(&item_x, sizeof(int), 1, f);
            fread(&item_y, sizeof(int), 1, f);
            fread(&item_type, sizeof(int), 1, f);
            fread(&item_display, sizeof(int), 1, f);
            fread(&item_hidden, sizeof(bool), 1, f);
            fread(&item_used, sizeof(bool), 1, f);
            fread(&item_usable_type, sizeof(int), 1, f);

            // Recreate item
            item* it = generate_item(item_x, item_y, (ItemType)item_type, item_display, (UsableItem)item_usable_type, i);
            if (it) {
                set_item_hidden(it, item_hidden);
                set_item_used(it, item_used);
                set_hotbar(h, i, it);
            }
        }
    }

    // Restore selected slot
    select_slot(h, selected_slot);

    // Force update of selected_item (in case selected_slot was already 0)
    // This ensures render_item_title displays correctly on first load
    select_slot(h, (selected_slot + 1) % max_size);
    select_slot(h, selected_slot);

    return true;
}

/// @brief Save an item to file
static bool save_item_data(FILE* f, item* it) {
    if (!f || !it) return false;

    int x = get_item_x(it);
    int y = get_item_y(it);
    int type = get_item_type(it);
    int display = get_item_display(it);
    bool hidden = is_item_hidden(it);
    bool used = is_item_used(it);
    int usable_type = get_item_usable_type(it);
    bool has_entity = is_an_entity(it);

    fwrite(&x, sizeof(int), 1, f);
    fwrite(&y, sizeof(int), 1, f);
    fwrite(&type, sizeof(int), 1, f);
    fwrite(&display, sizeof(int), 1, f);
    fwrite(&hidden, sizeof(bool), 1, f);
    fwrite(&used, sizeof(bool), 1, f);
    fwrite(&usable_type, sizeof(int), 1, f);
    fwrite(&has_entity, sizeof(bool), 1, f);

    // Save specs based on item type
    if (type == ENEMY) {
        enemy* e = (enemy*)get_item_spec(it);
        if (e) {
            // Save enemy state (hp is the most important for restoring state)
            fwrite(&e->hp, sizeof(int), 1, f);
            fwrite(&e->damage, sizeof(int), 1, f);
            fwrite(&e->from_id, sizeof(int), 1, f);
            fwrite(&e->speed, sizeof(int), 1, f);
            fwrite(&e->infinity, sizeof(int), 1, f);
            fwrite(&e->score, sizeof(int), 1, f);
            fwrite(&e->attack_delay, sizeof(int), 1, f);
            fwrite(&e->attack_interval, sizeof(int), 1, f);
        } else {
            // Write zeros if no spec (shouldn't happen for enemies)
            int zero = 0;
            for (int i = 0; i < 8; i++) {
                fwrite(&zero, sizeof(int), 1, f);
            }
        }
    } else if (type == LOOTABLE) {
        lootable* loot = (lootable*)get_item_spec(it);
        if (loot) {
            fwrite(&loot->bronze, sizeof(int), 1, f);
            fwrite(&loot->silver, sizeof(int), 1, f);
            fwrite(&loot->gold, sizeof(int), 1, f);
            fwrite(&loot->nadino, sizeof(int), 1, f);
        } else {
            int zero = 0;
            for (int i = 0; i < 4; i++) {
                fwrite(&zero, sizeof(int), 1, f);
            }
        }
    }

    return true;
}

/// @brief Load an item from file and add it to the chunk
/// @param f file pointer
/// @param c chunk
/// @param items_array dynarray to get the index
/// @return loaded item or NULL
static item* load_item_data(FILE* f, chunk* c, dynarray* items_array) {
    if (!f || !c) return NULL;

    int x, y, type, display, usable_type;
    bool hidden, used, has_entity;

    fread(&x, sizeof(int), 1, f);
    fread(&y, sizeof(int), 1, f);
    fread(&type, sizeof(int), 1, f);
    fread(&display, sizeof(int), 1, f);
    fread(&hidden, sizeof(bool), 1, f);
    fread(&used, sizeof(bool), 1, f);
    fread(&usable_type, sizeof(int), 1, f);
    fread(&has_entity, sizeof(bool), 1, f);  // Read but ignore - kept for compatibility

    // Create the item
    item* it = generate_item(x, y, (ItemType)type, display, (UsableItem)usable_type, len_dyn(items_array));

    // Load specs based on item type
    if (type == ENEMY) {
        enemy* e = malloc(sizeof(enemy));
        fread(&e->hp, sizeof(int), 1, f);
        fread(&e->damage, sizeof(int), 1, f);
        fread(&e->from_id, sizeof(int), 1, f);
        fread(&e->speed, sizeof(int), 1, f);
        fread(&e->infinity, sizeof(int), 1, f);
        fread(&e->score, sizeof(int), 1, f);
        fread(&e->attack_delay, sizeof(int), 1, f);
        fread(&e->attack_interval, sizeof(int), 1, f);
        specialize(it, used, hidden, e);
    } else if (type == LOOTABLE) {
        lootable* loot = malloc(sizeof(lootable));
        fread(&loot->bronze, sizeof(int), 1, f);
        fread(&loot->silver, sizeof(int), 1, f);
        fread(&loot->gold, sizeof(int), 1, f);
        fread(&loot->nadino, sizeof(int), 1, f);
        specialize(it, used, hidden, loot);
    } else {
        // For other types, just set the states
        set_item_hidden(it, hidden);
        set_item_used(it, used);
    }

    return it;
}

/// @brief Save a chunk to file
static bool save_chunk_data(FILE* f, chunk* ck) {
    if (!f || !ck) return false;

    // Save chunk coordinates and metadata
    int x = get_chunk_x(ck);
    int y = get_chunk_y(ck);
    int spawn_x = get_chunk_spawn_x(ck);
    int spawn_y = get_chunk_spawn_y(ck);
    ChunkType type = get_chunk_type(ck);

    fwrite(&x, sizeof(int), 1, f);
    fwrite(&y, sizeof(int), 1, f);
    fwrite(&spawn_x, sizeof(int), 1, f);
    fwrite(&spawn_y, sizeof(int), 1, f);
    fwrite(&type, sizeof(int), 1, f);

    // Save chunk links (coordinates of linked chunks, or -9999 if no link)
    chunk_link links = get_chunk_links(ck);
    for (int i = 0; i < 5; i++) {
        if (links[i] != NULL) {
            int link_x = get_chunk_x(links[i]);
            int link_y = get_chunk_y(links[i]);
            fwrite(&link_x, sizeof(int), 1, f);
            fwrite(&link_y, sizeof(int), 1, f);
        } else {
            int no_link = -9999;
            fwrite(&no_link, sizeof(int), 1, f);
            fwrite(&no_link, sizeof(int), 1, f);
        }
    }

    // Save items in the chunk (excluding entity parts)
    dynarray* items = get_chunk_furniture_list(ck);
    int total_item_count = len_dyn(items);

    // First pass: count non-entity-part items
    int item_count = 0;
    for (int i = 0; i < total_item_count; i++) {
        item* it = get_dyn(items, i);
        // Count NULL items and items that are NOT entity parts
        if (it == NULL || !is_an_entity(it)) {
            item_count++;
        }
    }

    fwrite(&item_count, sizeof(int), 1, f);

    // Second pass: save non-entity-part items
    for (int i = 0; i < total_item_count; i++) {
        item* it = get_dyn(items, i);

        // Skip entity parts - they will be saved with their entity in the enemies section
        if (it != NULL && is_an_entity(it)) {
            continue;
        }

        if (it != NULL) {
            bool item_exists = true;
            fwrite(&item_exists, sizeof(bool), 1, f);
            if (!save_item_data(f, it)) {
                LOG_ERROR("Failed to save item %d in chunk (%d, %d)", i, x, y);
                return false;
            }
        } else {
            bool item_exists = false;
            fwrite(&item_exists, sizeof(bool), 1, f);
        }
    }

    // Save entities (ENEMY brains from enemies array + LOOTABLE brains found via entity_link)
    // First, collect all unique entity brains to save
    dynarray* brains_to_save = create_dyn();

    // Add ENEMY brains from enemies array
    dynarray* enemies = get_chunk_enemies(ck);
    int enemies_count = len_dyn(enemies);

    for (int i = 0; i < enemies_count; i++) {
        item* brain = get_dyn(enemies, i);
        if (brain != NULL) {
            append(brains_to_save, brain);
        }
    }

    // Add LOOTABLE brains by finding entity parts in elements array
    for (int i = 0; i < total_item_count; i++) {
        item* it = get_dyn(items, i);
        if (it != NULL && is_an_entity(it)) {
            entity* ent = get_entity_link(it);
            if (ent == NULL) {
                LOG_WARN("Item at index %d has entity_link but entity is NULL!", i);
                continue;
            }

            item* brain = get_entity_brain(ent);
            if (brain == NULL) {
                LOG_WARN("Entity at index %d has NULL brain!", i);
                continue;
            }

            // CRITICAL: Never add a brain that has entity_link - that means it's corrupted!
            if (is_an_entity(brain)) {
                LOG_ERROR("Brain at entity index %d has entity_link - CORRUPTED! Skipping.", i);
                continue;
            }

            // Check if we already added this brain (to avoid duplicates)
            bool already_added = false;
            int brains_count = len_dyn(brains_to_save);
            for (int j = 0; j < brains_count; j++) {
                if (get_dyn(brains_to_save, j) == brain) {
                    already_added = true;
                    break;
                }
            }

            if (!already_added) {
                append(brains_to_save, brain);
            }
        }
    }

    // Now save all collected brains
    int brain_count = len_dyn(brains_to_save);
    fwrite(&brain_count, sizeof(int), 1, f);

    for (int i = 0; i < brain_count; i++) {
        item* brain = get_dyn(brains_to_save, i);

        if (brain != NULL) {
            bool entity_exists = true;
            fwrite(&entity_exists, sizeof(bool), 1, f);

            // Save brain item
            if (!save_item_data(f, brain)) {
                LOG_ERROR("Failed to save entity brain %d in chunk (%d, %d)", i, x, y);
                free_dyn(brains_to_save);
                return false;
            }

            // Find the entity for this brain
            // For ENEMY brains, they have no entity_link, so we need to search in elements
            entity* ent = NULL;

            // First try to get entity from the brain's entity_link (won't work for ENEMY but try anyway)
            if (is_an_entity(brain)) {
                ent = get_entity_link(brain);
            }

            // If not found, search in elements for a part with this brain's entity
            if (ent == NULL) {
                for (int k = 0; k < total_item_count; k++) {
                    item* elem = get_dyn(items, k);
                    if (elem != NULL && is_an_entity(elem)) {
                        entity* test_ent = get_entity_link(elem);
                        if (test_ent != NULL && get_entity_brain(test_ent) == brain) {
                            ent = test_ent;
                            break;
                        }
                    }
                }
            }

            // Save entity parts
            if (ent != NULL) {
                dynarray* parts = get_entity_parts(ent);
                int part_count = len_dyn(parts);
                fwrite(&part_count, sizeof(int), 1, f);

                for (int j = 0; j < part_count; j++) {
                    item* part = get_dyn(parts, j);
                    if (part != NULL) {
                        bool part_exists = true;
                        fwrite(&part_exists, sizeof(bool), 1, f);
                        if (!save_item_data(f, part)) {
                            LOG_ERROR("Failed to save entity part %d for brain %d", j, i);
                            free_dyn(brains_to_save);
                            return false;
                        }
                    } else {
                        bool part_exists = false;
                        fwrite(&part_exists, sizeof(bool), 1, f);
                    }
                }
            } else {
                int part_count = 0;
                fwrite(&part_count, sizeof(int), 1, f);
            }
        } else {
            bool entity_exists = false;
            fwrite(&entity_exists, sizeof(bool), 1, f);
        }
    }

    free_dyn_no_item(brains_to_save);

    return true;
}

/// @brief Context for counting chunks in the hashmap
typedef struct {
    int count;
} chunk_counter_ctx;

/// @brief Callback to count chunks
static void count_chunk_callback(int x, int y, element_h elt, void* user_data) {
    (void)x;
    (void)y;
    (void)elt;
    chunk_counter_ctx* ctx = (chunk_counter_ctx*)user_data;
    ctx->count++;
}

/// @brief Context for saving chunks
typedef struct {
    FILE* f;
    bool success;
} chunk_saver_ctx;

/// @brief Callback to save a chunk
static void save_chunk_callback(int x, int y, element_h elt, void* user_data) {
    (void)x;
    (void)y;
    chunk_saver_ctx* ctx = (chunk_saver_ctx*)user_data;
    if (!ctx->success) return;  // Stop if already failed

    chunk* ck = (chunk*)elt;
    if (!save_chunk_data(ctx->f, ck)) {
        ctx->success = false;
        LOG_ERROR("Failed to save chunk at (%d, %d)", x, y);
    }
}

/// @brief Save map data to file
static bool save_map_data(FILE* f, map* m) {
    if (!f || !m) return false;

    hm* chunks_map = get_map_hashmap(m);

    // Count chunks
    chunk_counter_ctx counter = {0};
    for_each_hm(chunks_map, count_chunk_callback, &counter);

    LOG_INFO("Saving %d chunks to file", counter.count);
    fwrite(&counter.count, sizeof(int), 1, f);

    // Save all chunks
    chunk_saver_ctx saver = {f, true};
    for_each_hm(chunks_map, save_chunk_callback, &saver);

    return saver.success;
}

// Temporary structure to store chunk link info during loading
typedef struct {
    int chunk_x;
    int chunk_y;
    int link_coords[5][2];  // For each direction: [x, y] or [-9999, -9999] if no link
} chunk_link_info;

/// @brief Load map data from file
static bool load_map_data(FILE* f, map* m) {
    if (!f || !m) return false;

    int chunk_count;
    fread(&chunk_count, sizeof(int), 1, f);
    LOG_INFO("Loading %d chunks from file", chunk_count);

    hm* chunks_map = get_map_hashmap(m);

    // Store link info for second pass
    chunk_link_info* link_infos = malloc(sizeof(chunk_link_info) * chunk_count);

    // First pass: load all chunks (structure and items)
    for (int i = 0; i < chunk_count; i++) {
        int x, y, spawn_x, spawn_y;
        ChunkType type;

        fread(&x, sizeof(int), 1, f);
        fread(&y, sizeof(int), 1, f);
        fread(&spawn_x, sizeof(int), 1, f);
        fread(&spawn_y, sizeof(int), 1, f);
        fread(&type, sizeof(int), 1, f);

        // Read link coordinates (we'll restore them in second pass)
        link_infos[i].chunk_x = x;
        link_infos[i].chunk_y = y;
        for (int j = 0; j < 5; j++) {
            fread(&link_infos[i].link_coords[j][0], sizeof(int), 1, f);
            fread(&link_infos[i].link_coords[j][1], sizeof(int), 1, f);
        }

        // Get or create the chunk
        chunk* ck = get_hm(chunks_map, x, y);
        if (ck == NULL) {
            // Create chunk manually since it doesn't exist
            ck = malloc(sizeof(chunk));
            ck->link = calloc(5, sizeof(chunk*));
            ck->x = x;
            ck->y = y;
            ck->spawn_x = spawn_x;
            ck->spawn_y = spawn_y;
            ck->type = type;
            ck->elements = create_dyn();
            ck->enemies = create_dyn();
            ck->hashmap = create_hashmap();
            set_hm(chunks_map, x, y, ck);
        } else {
            // Update existing chunk - clear its contents first!
            LOG_INFO("Clearing existing chunk (%d, %d) before loading", x, y);

            // Free old items
            int old_item_count = len_dyn(ck->elements);
            for (int k = 0; k < old_item_count; k++) {
                item* old_it = get_dyn(ck->elements, k);
                if (old_it != NULL) {
                    free_item(old_it);
                }
            }
            free_dyn_no_item(ck->elements);

            // Free old enemies (but not the items, they're already in elements)
            free_dyn_no_item(ck->enemies);

            // Free old hashmap
            free_hm(ck->hashmap);

            // Recreate empty structures
            ck->elements = create_dyn();
            ck->enemies = create_dyn();
            ck->hashmap = create_hashmap();

            // Update metadata
            ck->spawn_x = spawn_x;
            ck->spawn_y = spawn_y;
            ck->type = type;
        }

        // Load items (entity parts are NOT saved here, they're in the enemies section)
        int item_count;
        fread(&item_count, sizeof(int), 1, f);

        for (int j = 0; j < item_count; j++) {
            bool item_exists;
            fread(&item_exists, sizeof(bool), 1, f);

            if (item_exists) {
                item* it = load_item_data(f, ck, ck->elements);
                if (it) {
                    append(ck->elements, it);
                    set_hm(ck->hashmap, get_item_x(it), get_item_y(it), it);
                } else {
                    LOG_ERROR("Failed to load item %d in chunk (%d, %d)", j, x, y);
                    append(ck->elements, NULL);
                }
            } else {
                append(ck->elements, NULL);
            }
        }

        // Load enemies
        int enemy_count;
        fread(&enemy_count, sizeof(int), 1, f);

        for (int j = 0; j < enemy_count; j++) {
            bool enemy_exists;
            fread(&enemy_exists, sizeof(bool), 1, f);

            if (enemy_exists) {
                // Load enemy brain
                item* brain = load_item_data(f, ck, ck->enemies);

                if (brain) {
                    // Load entity parts count
                    int part_count;
                    fread(&part_count, sizeof(int), 1, f);

                    // Check if this is a lootable that has been emptied
                    bool should_skip = false;
                    if (get_item_type(brain) == LOOTABLE) {
                        lootable* loot = (lootable*)get_item_spec(brain);
                        if (loot && loot->bronze == 0 && loot->silver == 0 &&
                            loot->gold == 0 && loot->nadino == 0) {
                            // Chest is empty, don't recreate it
                            should_skip = true;
                            LOG_INFO("Skipping empty chest in chunk (%d, %d)", x, y);
                        }
                    }

                    if (!should_skip) {
                        // For ENEMY, update the from_id to match the index where it will be added
                        if (get_item_type(brain) == ENEMY) {
                            enemy* e = (enemy*)get_item_spec(brain);
                            if (e) {
                                e->from_id = len_dyn(ck->enemies);
                            }
                        }

                        if (part_count > 0) {
                            // Create entity
                            entity* ent = create_entity(brain, ck);

                            // Load all parts
                            for (int k = 0; k < part_count; k++) {
                                bool part_exists;
                                fread(&part_exists, sizeof(bool), 1, f);

                                if (part_exists) {
                                    item* part = load_item_data(f, ck, ck->elements);
                                    if (part) {
                                        add_entity_part(ent, part);
                                        link_entity(part, ent);
                                        append(ck->elements, part);
                                        set_hm(ck->hashmap, get_item_x(part), get_item_y(part), part);
                                    }
                                }
                            }
                        }

                        // Add brain to enemies array ONLY if it's an ENEMY type
                        // LOOTABLE brains are NOT stored in enemies, only accessible via entity_link
                        if (get_item_type(brain) == ENEMY) {
                            append(ck->enemies, brain);
                        }
                    } else {
                        // Skip the parts data even if we don't create the entity
                        for (int k = 0; k < part_count; k++) {
                            bool part_exists;
                            fread(&part_exists, sizeof(bool), 1, f);
                            if (part_exists) {
                                // Just read and discard the part data
                                load_item_data(f, ck, ck->elements);
                            }
                        }
                        // Free the brain since we won't use it
                        free_item(brain);
                    }
                }
            } else {
                append(ck->enemies, NULL);
            }
        }
    }

    // Second pass: restore chunk links
    for (int i = 0; i < chunk_count; i++) {
        chunk* ck = get_hm(chunks_map, link_infos[i].chunk_x, link_infos[i].chunk_y);
        if (!ck) continue;

        chunk_link links = get_chunk_links(ck);
        for (int j = 0; j < 5; j++) {
            int link_x = link_infos[i].link_coords[j][0];
            int link_y = link_infos[i].link_coords[j][1];

            if (link_x != -9999) {
                // Find the linked chunk
                chunk* linked_chunk = get_hm(chunks_map, link_x, link_y);
                if (linked_chunk) {
                    links[j] = linked_chunk;
                } else {
                    LOG_WARN("Chunk (%d, %d) link[%d] points to non-existent chunk (%d, %d)",
                             link_infos[i].chunk_x, link_infos[i].chunk_y, j, link_x, link_y);
                    links[j] = NULL;
                }
            } else {
                links[j] = NULL;
            }
        }
    }

    free(link_infos);
    return true;
}

bool save_game(const char* filename, player* p, map* m, hotbar* h) {
    if (!filename || !p || !m || !h) return false;

    // First, save to an uncompressed temporary file
    char temp_filename[512];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE* f = fopen(temp_filename, "wb");
    if (!f) {
        LOG_ERROR("Failed to open temp save file: %s", temp_filename);
        return false;
    }

    // Write magic number and version
    uint32_t magic = SAVE_MAGIC;
    uint32_t version = SAVE_VERSION;
    fwrite(&magic, sizeof(uint32_t), 1, f);
    fwrite(&version, sizeof(uint32_t), 1, f);

    // Save player data
    if (!save_player_data(f, p)) {
        LOG_ERROR("Failed to save player data");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    // Save hotbar data
    if (!save_hotbar_data(f, h)) {
        LOG_ERROR("Failed to save hotbar data");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    // Save map data
    if (!save_map_data(f, m)) {
        LOG_ERROR("Failed to save map data");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    fclose(f);

    // Now compress the temporary file to the final file
    FILE* src = fopen(temp_filename, "rb");
    if (!src) {
        LOG_ERROR("Failed to reopen temp file for compression");
        remove(temp_filename);
        return false;
    }

    gzFile dest = gzopen(filename, "wb9");  // 9 = maximum compression
    if (!dest) {
        LOG_ERROR("Failed to open compressed save file: %s", filename);
        fclose(src);
        remove(temp_filename);
        return false;
    }

    // Read from temp and write compressed
    unsigned char buffer[8192];
    size_t bytes_read;
    size_t uncompressed_size = 0;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        uncompressed_size += bytes_read;
        if (gzwrite(dest, buffer, bytes_read) != (int)bytes_read) {
            LOG_ERROR("Failed to write compressed data");
            fclose(src);
            gzclose(dest);
            remove(temp_filename);
            remove(filename);
            return false;
        }
    }

    fclose(src);
    gzclose(dest);
    remove(temp_filename);

    // Get compressed file size
    struct stat st;
    size_t compressed_size = 0;
    if (stat(filename, &st) == 0) {
        compressed_size = st.st_size;
    }

    float ratio = (uncompressed_size > 0) ? (100.0f * compressed_size / uncompressed_size) : 0.0f;
    LOG_INFO("Game saved to: %s (%zu bytes -> %zu bytes, %.1f%%)",
             filename, uncompressed_size, compressed_size, ratio);
    return true;
}

bool load_game(const char* filename, player* p, map* m, hotbar* h) {
    if (!filename || !p || !m || !h) return false;

    // First, decompress the file to a temporary file
    gzFile src = gzopen(filename, "rb");
    if (!src) {
        LOG_ERROR("Failed to open compressed save file: %s", filename);
        return false;
    }

    char temp_filename[512];
    snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", filename);

    FILE* dest = fopen(temp_filename, "wb");
    if (!dest) {
        LOG_ERROR("Failed to create temp file for decompression");
        gzclose(src);
        return false;
    }

    // Decompress
    unsigned char buffer[8192];
    int bytes_read;
    while ((bytes_read = gzread(src, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytes_read, dest) != (size_t)bytes_read) {
            LOG_ERROR("Failed to write decompressed data");
            fclose(dest);
            gzclose(src);
            remove(temp_filename);
            return false;
        }
    }

    gzclose(src);
    fclose(dest);

    // Now read from the decompressed temp file
    FILE* f = fopen(temp_filename, "rb");
    if (!f) {
        LOG_ERROR("Failed to open decompressed temp file");
        remove(temp_filename);
        return false;
    }

    // Read and verify magic number and version
    uint32_t magic, version;
    fread(&magic, sizeof(uint32_t), 1, f);
    fread(&version, sizeof(uint32_t), 1, f);

    if (magic != SAVE_MAGIC) {
        LOG_ERROR("Invalid save file (wrong magic number)");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    if (version != SAVE_VERSION) {
        LOG_ERROR("Incompatible save file version: %d (expected %d)", version, SAVE_VERSION);
        fclose(f);
        remove(temp_filename);
        return false;
    }

    // Load player data
    if (!load_player_data(f, p)) {
        LOG_ERROR("Failed to load player data");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    // Load hotbar data
    if (!load_hotbar_data(f, h)) {
        LOG_ERROR("Failed to load hotbar data");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    // Load map data
    if (!load_map_data(f, m)) {
        LOG_ERROR("Failed to load map data");
        fclose(f);
        remove(temp_filename);
        return false;
    }

    fclose(f);
    remove(temp_filename);

    // Restore player's current chunk using the stored coordinates
    chunk* current_chunk = get_chunk(m, g_player_chunk_x, g_player_chunk_y);
    if (current_chunk) {
        set_player_chunk(p, current_chunk);
        LOG_INFO("Player chunk restored to (%d, %d)", g_player_chunk_x, g_player_chunk_y);
    } else {
        LOG_ERROR("Failed to restore player chunk at (%d, %d)", g_player_chunk_x, g_player_chunk_y);
        // Fallback to spawn
        set_player_chunk(p, get_spawn(m));
    }

    LOG_INFO("Game loaded from: %s", filename);
    return true;
}

bool save_file_exists(const char* filename) {
    if (!filename) return false;
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

bool delete_save(const char* filename) {
    if (!filename) return false;
    if (remove(filename) == 0) {
        LOG_INFO("Save file deleted: %s", filename);
        return true;
    }
    LOG_ERROR("Failed to delete save file: %s", filename);
    return false;
}
