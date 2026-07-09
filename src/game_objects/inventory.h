#ifndef INVENTORY_H
#define INVENTORY_H

#include <stdbool.h>
#include <stdlib.h>

#include "../utils/constants.h"

typedef struct item item;
typedef struct hotbar hotbar;
typedef struct keyholder keyholder;
typedef struct player player;

/// @brief Create a new hotbar
/// @return hotbar
hotbar* create_hotbar();

/// @brief Creates a new keyholder
/// @return keyholder
keyholder* create_keyholder();

/// @brief Destroy a hotbar and free all its items
/// @param hotbar hotbar
void destroy_hotbar(hotbar* hotbar);

/// @brief Destroys a keyholder
/// @param k keyholder
void destroy_keyholder(keyholder* k);

/// @brief Set hotbar[index] to e
/// @param hotbar hotbar
/// @param index index
/// @param item item
void set_hotbar(hotbar* hotbar, int index, item* item);

/// @brief Clear all items and cached state from a hotbar
/// @param hotbar hotbar
void clear_hotbar(hotbar* hotbar);

/// @brief Set the number of keys of a rarity to nb
/// @param k keyholder
/// @param rarity
/// @param nb
void set_keyholder_keys_of_rarity(keyholder* k, Rarity rarity, int nb);

/// @brief Adds nb to the number of keys of a rarity (< 0 => 0)
/// @param k keyholder
/// @param rarity
/// @param nb
void add_keyholder_keys_of_rarity(keyholder* k, Rarity rarity, int nb);

/// @brief Get hotbar[index]
/// @param hotbar hotbar
/// @param index index
/// @return item
item* get_hotbar(hotbar* hotbar, int index);

/// @brief Returns index of selected slot
/// @param hotbar hotbar
/// @return index
int get_selected_slot(hotbar* hotbar);

/// @brief Returns item in selected slot
/// @param hotbar hotbar
/// @return item
item* get_selected_item(hotbar* hotbar);

/// @brief Get hotbar max size
/// @param hotbar  hotbar
/// @return maxsize
int get_hotbar_max_size(hotbar* hotbar);

/// @brief Get the number of keys of a rarity
/// @param k keyholder
/// @param rarity
/// @return nb of keys of rarity
int get_keyholder_keys_of_rarity(keyholder* k, Rarity rarity);

/// @brief Returns the keyholder level
/// @param k
KeyHolderLevel get_keyholder_level(keyholder* k);

/// @brief Set the keyholder level
/// @param k keyholder
/// @param level KeyHolderLevel
void set_keyholder_level(keyholder* k, int level);

/// @brief Returns true if there is at least one key of said rarity in the keyholder
/// @param k
/// @param rarity
bool keyholder_has_key_of_rarity(keyholder* k, Rarity rarity);

/// @brief Level up the keyholder
/// @param k keyholder
void keyholder_level_up(keyholder* k);

/// @brief Returns the key's rarity
/// @param key
/// @return RARITY_NONE if not a key
Rarity get_key_rarity(UsableItem key);

/// @brief Returns if the hotbar is full
/// @param hotbar hotbar
/// @return true if full
bool is_hotbar_full(hotbar* hotbar);

/// @brief Pickup an item, set to first free spot
/// @param player player
/// @param item item
void pickup(player* player, item* item);

/// @brief Destroys the item at index
/// @param hotbar hotbar
/// @param index index
/// @param free_item_dropped if true, free the item dropped
void hotbar_drop(hotbar* hotbar, int index, bool free_item_dropped);

/// @brief Select hotbar slot
/// @param hotbar hotbar
/// @param index index
void select_slot(hotbar* hotbar, int index);

/// @brief Get current number of items in hotbar
/// @param hotbar hotbar
/// @return number of items
int get_hotbar_entries(hotbar* hotbar);

/**
 * Reset the cached hotbar slot tracking so the next shot re-evaluates weapon stats.
 */
void bow_check_flag();

int get_last_hotbar_index();
void set_last_hotbar_index(int index);

// Returns the index of the first item in hotbar matching the usable item type given, if none returns -1
int get_hotbar_index_of_usable_item(hotbar* h, UsableItem type);

#endif