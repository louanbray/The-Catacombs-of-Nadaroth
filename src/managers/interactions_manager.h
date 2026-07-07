#ifndef INTERACTIONS_MANAGER_H
#define INTERACTIONS_MANAGER_H

#include <stdbool.h>

typedef struct Render_Buffer Render_Buffer;

// Initialise / détruit le système
void init_interactions_system();
void destroy_interactions_system();

// Charge un fichier d'interactions et l'associe à un id (ex "skin").
// Le fichier d'interactions est un fichier texte contenant des lignes comme:
// (§):{0,1,2,3,4,5}
// [0,2]:{10/47/50,11/47/50,12/47/50,13/47/50}:[>]
// [1,3]:{16/47/50,16/58/61,16/70/73,17/47/50,17/58/61,17/70/73}:[§■]
bool load_interactions_file(const char* filename, const char* id);

// Libère les interactions associées à id
void unload_interactions(const char* id);

// Optionnel : remapper les touches de direction (0..3)
void set_direction_key(int dir_index, char key);
int* display_interface_with_interactions_main(Render_Buffer* r, const char* visual_filename, const char* interaction_id, int* out_selected_indices);
#endif  // INTERACTIONS_MANAGER_H