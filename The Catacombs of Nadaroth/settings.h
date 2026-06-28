#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

#include "constants.h"

enum SettingID {
    SETTING_SKIP_INTRO,
    SETTING_SKIP_START_MENU_ON_RESET,
    SETTING_SKIP_START_MENU_ON_NEW_GAME,
    SETTING_SKIP_PLAYER_CUSTOM_ON_RESET,
    SETTING_SKIP_PLAYER_CUSTOM_ON_NEW_GAME,
    SETTING_SKIP_DIFFICULTY_SELECTION_ON_RESET,
    SETTING_SKIP_DIFFICULTY_SELECTION_ON_NEW_GAME,
    SETTINGS_COUNT
};

const char* get_setting_name(enum SettingID id);
const char* get_setting_description(enum SettingID id);
int get_setting_value(enum SettingID id);
int get_setting_max_value(enum SettingID id);
void set_setting_value(enum SettingID id, int value);
bool modify_setting_value(enum SettingID id, int value);
Color get_setting_color(enum SettingID id);
void load_settings();
void save_settings();

#endif