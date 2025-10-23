#include "save_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "logger.h"
#include "inventory.h"
#include "item.h"
#include "map.h"
#include "generation.h"
#include "hash.h"
#include "player.h"

#define SAVE_VERSION 2
#define SAVE_MAGIC 0x4E414430  // "NAD0" in hex

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
    GamePhase phase = get_player_phase(p);
    
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
    fwrite(&phase, sizeof(int), 1, f);

    return true;
}

/// @brief Load player data from file
static bool load_player_data(FILE* f, player* p) {
    if (!f || !p) return false;

    int x, y, px, py, health, max_health, mental_health, damage, arrow_speed, range;
    bool infinity;
    int design, score, deaths, phase;

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
    fread(&phase, sizeof(int), 1, f);

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

    return true;
}

/// @brief Save map data to file
static bool save_map_data(FILE* f, map* m) {
    if (!f || !m) return false;

    // For now, we only save a placeholder
    // The map will be partially regenerated from the seed
    // but we preserve loaded chunks

    // Save number of chunks (for now, just 0 as placeholder)
    // Full map state saving would require extensive chunk serialization
    int chunk_count = 0;
    fwrite(&chunk_count, sizeof(int), 1, f);

    return true;
}

/// @brief Load map data from file
static bool load_map_data(FILE* f, map* m) {
    if (!f || !m) return false;

    // Load number of chunks
    int chunk_count;
    fread(&chunk_count, sizeof(int), 1, f);

    // For now, map will regenerate via seed
    // Full reconstruction would happen here

    return true;
}

bool save_game(const char* filename, player* p, map* m, hotbar* h) {
    if (!filename || !p || !m || !h) return false;

    FILE* f = fopen(filename, "wb");
    if (!f) {
        LOG_ERROR("Failed to open save file: %s", filename);
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
        return false;
    }

    // Save hotbar data
    if (!save_hotbar_data(f, h)) {
        LOG_ERROR("Failed to save hotbar data");
        fclose(f);
        return false;
    }

    // Save map data
    if (!save_map_data(f, m)) {
        LOG_ERROR("Failed to save map data");
        fclose(f);
        return false;
    }

    fclose(f);
    LOG_INFO("Game saved to: %s", filename);
    return true;
}

bool load_game(const char* filename, player* p, map* m, hotbar* h) {
    if (!filename || !p || !m || !h) return false;

    FILE* f = fopen(filename, "rb");
    if (!f) {
        LOG_ERROR("Failed to open save file: %s", filename);
        return false;
    }

    // Read and verify magic number and version
    uint32_t magic, version;
    fread(&magic, sizeof(uint32_t), 1, f);
    fread(&version, sizeof(uint32_t), 1, f);

    if (magic != SAVE_MAGIC) {
        LOG_ERROR("Invalid save file (wrong magic number)");
        fclose(f);
        return false;
    }

    if (version != SAVE_VERSION) {
        LOG_ERROR("Incompatible save file version: %d (expected %d)", version, SAVE_VERSION);
        fclose(f);
        return false;
    }

    // Load player data
    if (!load_player_data(f, p)) {
        LOG_ERROR("Failed to load player data");
        fclose(f);
        return false;
    }

    // Load hotbar data
    if (!load_hotbar_data(f, h)) {
        LOG_ERROR("Failed to load hotbar data");
        fclose(f);
        return false;
    }

    // Load map data
    if (!load_map_data(f, m)) {
        LOG_ERROR("Failed to load map data");
        fclose(f);
        return false;
    }

    fclose(f);
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
