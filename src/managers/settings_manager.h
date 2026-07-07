#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <stdbool.h>

#include "../utils/constants.h"

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

/**
 * Get the display name of a setting.
 *
 * @return NULL when the id is out of range.
 */
const char* get_setting_name(enum SettingID id);

/**
 * Get the human-readable description of a setting.
 *
 * @return NULL when the id is out of range.
 */
const char* get_setting_description(enum SettingID id);

/**
 * Get the current value of a setting.
 *
 * @return 0 when the id is out of range.
 */
int get_setting_value(enum SettingID id);

/**
 * Get the maximum allowed value of a setting.
 *
 * @return 0 when the id is out of range.
 */
int get_setting_max_value(enum SettingID id);

/**
 * Set a setting value, clamping it to the valid range and saving it when it changes.
 */
void set_setting_value(enum SettingID id, int value);

/**
 * Adjust a setting value by a delta, clamping it to the valid range.
 *
 * @return true when the value changed, false otherwise.
 */
bool modify_setting_value(enum SettingID id, int value);

/**
 * Get the UI color associated with a setting.
 *
 * Boolean settings use green for enabled and red for disabled; other settings
 * use the default color.
 */
Color get_setting_color(enum SettingID id);

/**
 * Load setting definitions and saved values from disk.
 *
 * This reads the static setting metadata from assets/data/settings.dodjo and
 * the persisted values from assets/data/player_settings.dodjo.
 */
void load_settings();

/**
 * Persist the current setting values to disk.
 */
void save_settings();

#endif