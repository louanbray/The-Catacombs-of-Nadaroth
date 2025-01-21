#include "assets_manager.h"

#include <stdio.h>
#include <string.h>

EntityAssetFile* parse_entity_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) return NULL;

    EntityAssetFile* entity_file = malloc(sizeof(EntityAssetFile));
    if (!entity_file) return NULL;

    entity_file->parts = NULL;
    entity_file->count = 0;

    // Parse the specs line
    char specs_line[256];
    if (fgets(specs_line, sizeof(specs_line), file) != NULL) {
        if (specs_line[0] == '[' && specs_line[strlen(specs_line) - 3] == ']') {
            specs_line[strlen(specs_line) - 3] = '\0';
            char* start = specs_line + 1;

            // Parse dynamic specs
            size_t count = 0;
            int* specs = NULL;

            char* token = strtok(start, ",");
            while (token != NULL) {
                specs = realloc(specs, (count + 1) * sizeof(int));

                if (!specs) {
                    fclose(file);
                    free(entity_file);
                    return NULL;
                }

                specs[count++] = atoi(token);
                token = strtok(NULL, ",");
            }

            entity_file->specs.specs = specs;
            entity_file->specs.spec_count = count;
        } else {
            fclose(file);
            free(entity_file);
            return NULL;
        }
    }

    // Parse entity parts
    EntityPart part;
    while (fscanf(file, "%d,%d,%d,%d,%d", &part.x, &part.y, &part.display, &part.row_repeat, &part.col_repeat) == 5) {
        entity_file->parts = realloc(entity_file->parts, (entity_file->count + 1) * sizeof(EntityPart));

        if (!entity_file->parts) {
            fclose(file);
            free(entity_file->specs.specs);
            free(entity_file);
            return NULL;
        }
        entity_file->parts[entity_file->count++] = part;
    }

    fclose(file);
    return entity_file;
}

AssetManager* create_asset_manager() {
    AssetManager* manager = malloc(sizeof(AssetManager));
    if (!manager) return NULL;

    for (int i = 0; i < ENTITY_TYPE_COUNT; i++) {
        manager->entities[i] = NULL;
    }

    return manager;
}

bool add_entity_file(AssetManager* manager, const char* filename, EntityType type) {
    if (type < 0 || type >= ENTITY_TYPE_COUNT) {
        return false;  // Invalid type
    }

    EntityAssetFile* entity = parse_entity_file(filename);
    if (!entity) return false;

    manager->entities[type] = entity;
    return true;
}

EntityAssetFile* get_entity_file(AssetManager* manager, EntityType type) {
    if (type < 0 || type >= ENTITY_TYPE_COUNT) {
        return NULL;  // Invalid type
    }

    return manager->entities[type];
}

// Free all resources used by the asset manager
void destroy_asset_manager(AssetManager* manager) {
    for (size_t i = 0; i < ENTITY_TYPE_COUNT; i++) {
        free(manager->entities[i]->specs.specs);
        free(manager->entities[i]->parts);
        free(manager->entities[i]);
    }
    free(manager->entities);
    free(manager);
}