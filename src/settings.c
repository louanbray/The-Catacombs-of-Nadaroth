#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_manager.h"
#include "logger.h"

#define SETTINGS_FILE "data/player_settings.dodjo"
#define DATA_FILE "assets/definitions/settings.dodjo"

typedef struct setting {
    enum SettingID id;
    const char* name;
    const char* description;
    int max_value;
    int value;
} setting;

setting** settings = NULL;

const char* get_setting_name(enum SettingID id) {
    if (id < 0 || id >= SETTINGS_COUNT) return NULL;
    return settings[id]->name;
}

const char* get_setting_description(enum SettingID id) {
    if (id < 0 || id >= SETTINGS_COUNT) return NULL;
    return settings[id]->description;
}

int get_setting_value(enum SettingID id) {
    if (id < 0 || id >= SETTINGS_COUNT) return 0;
    return settings[id]->value;
}

int get_setting_max_value(enum SettingID id) {
    if (id < 0 || id >= SETTINGS_COUNT) return 0;
    return settings[id]->max_value;
}

void set_setting_value(enum SettingID id, int value) {
    if (id < 0 || id >= SETTINGS_COUNT) return;
    if (value > settings[id]->max_value) value = settings[id]->max_value;
    if (value < 0) value = 0;
    if (value != settings[id]->value) {
        settings[id]->value = value;
        LOG_INFO("Setting %s value set to %d/%d", settings[id]->name, value, settings[id]->max_value);
        save_settings();
    }
}

bool modify_setting_value(enum SettingID id, int value) {
    if (id < 0 || id >= SETTINGS_COUNT) return false;

    int new_value = settings[id]->value + value;
    if (new_value > settings[id]->max_value)
        new_value = settings[id]->max_value;
    if (new_value < 0)
        new_value = 0;
    if (new_value != settings[id]->value) {
        settings[id]->value = new_value;
        LOG_INFO("Setting %s value modified to %d/%d", settings[id]->name, new_value, settings[id]->max_value);
        play_sound_effect_by_id(AUDIO_CHANGE_SETTING_VALUE);
        save_settings();
        return true;
    } else {
        return false;
    }
}

Color get_setting_color(enum SettingID id) {
    if (id < 0 || id >= SETTINGS_COUNT) return COLOR_DEFAULT;
    if (settings[id]->max_value != 1) return COLOR_DEFAULT;
    return settings[id]->value == 1 ? COLOR_GREEN : COLOR_RED;
}

void load_settings() {
    settings = calloc(SETTINGS_COUNT, sizeof(setting*));
    FILE* settings_file = fopen(SETTINGS_FILE, "r");
    FILE* data_file = fopen(DATA_FILE, "r");

    if (data_file == NULL) return;
    while (!feof(data_file)) {
        int id;
        char name[256];
        char description[512];
        int max_value;
        fscanf(data_file, "%d;%255[^;];%511[^;];%d\n", &id, name, description, &max_value);
        if (id >= 0 && id < SETTINGS_COUNT) {
            setting* param = malloc(sizeof(setting));
            param->id = (enum SettingID)id;
            param->name = strdup(name);
            param->description = strdup(description);
            param->max_value = max_value;
            param->value = 0;
            settings[id] = param;
        }
    }

    int id = 0;
    while (settings_file && !feof(settings_file)) {
        int status = 0;
        fscanf(settings_file, "%d\n", &status);
        if (id >= 0 && id < SETTINGS_COUNT) {
            settings[id]->value = status <= settings[id]->max_value ? status : settings[id]->max_value;
        }
        id++;
    }

    fclose(data_file);
    if (settings_file) fclose(settings_file);
}
void save_settings() {
    FILE* file = fopen(SETTINGS_FILE, "w");
    for (int i = 0; i < SETTINGS_COUNT; i++) {
        fprintf(file, "%d\n", settings[i]->value);
    }
    fclose(file);
}