#define _GNU_SOURCE
#include "interactions_manager.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#include "../display/render.h"  // update_screen, setup_render_buffer, finalize_render_buffer, USE_KEY, KEY_PRESSED
#include "../managers/input_manager.h"
#include "../utils/win_compat.h"

// ----- Types internes -----
typedef struct {
    // mapping pour cette animation:
    // pair d'indices (a,b) indiquant quelles actions déclenchent dec/inc
    int act_dec;  // ex 0
    int act_inc;  // ex 2

    int pos_count;
    Pos* positions;  // positions en board-coordinates (absolute 0..RENDER_HEIGHT-1 / 0..RENDER_WIDTH-1)

    // pattern to draw, ex "[>]" or "[§■]"
    char* pattern;               // chaîne ASCII, e.g. "[]", "[>]"
    bool has_color_placeholder;  // true si pattern contient un token couleur (ex '§')
    char color_symbol[5];        // symbole couleur complet en UTF-8 (jusqu'à 4 octets + NUL)

    // si has_color_placeholder true : color_symbol refers to a color set defined elsewhere
    // color_sequence length == pos_count (we'll index pos_i % color_seq_len)
    int* color_sequence;  // integers parsed from definitions (e.g. 0,1,2,3...) or NULL
    int color_sequence_len;

    // runtime index
    int current_index;
    int weight;
} UIAnimation;

typedef struct InteractionSet {
    char* id;  // ex "skin"
    UIAnimation** anims;
    int anim_count;

    // color symbol definitions: map symbol string -> int array
    char** color_keys;   // symbol strings (peuvent faire plusieurs octets en UTF-8, ex "§")
    int** color_values;  // arrays of ints
    int* color_value_counts;
    int color_key_count;
} InteractionSet;

// ----- Global manager -----
static InteractionSet** g_sets = NULL;
static int g_set_count = 0;

static char g_dir_keys[4] = {'z', 'd', 's', 'q'};
static char g_mdir_keys[4] = {'Z', 'D', 'S', 'Q'};

// ----- Helpers -----
static InteractionSet* find_set_by_id(const char* id) {
    for (int i = 0; i < g_set_count; i++) {
        if (strcmp(g_sets[i]->id, id) == 0) return g_sets[i];
    }
    return NULL;
}

void set_direction_key(int dir_index, char key) {
    if (dir_index >= 0 && dir_index < 4) {
        g_dir_keys[dir_index] = tolower(key);
        g_mdir_keys[dir_index] = toupper(key);
    }
}

// trim helpers
static char* strtrim(char* s) {
    if (!s) return s;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

// Returns the byte-length of the UTF-8 sequence starting at the given
// leading byte. Falls back to 1 for continuation/invalid bytes so we
// never overrun a buffer on malformed input.
static int utf8_seq_len(unsigned char c) {
    if ((c & 0x80) == 0x00) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

// parse integer list "0,1,2" into newly malloc'd int[], returns count via out_count
static int* parse_int_list(const char* s, int* out_count) {
    *out_count = 0;
    if (!s) return NULL;
    char* tmp = strdup(s);
    char* tok = strtok(tmp, ",");
    int* arr = NULL;
    while (tok) {
        arr = realloc(arr, sizeof(int) * (*out_count + 1));
        arr[*out_count] = atoi(strtrim(tok));
        (*out_count)++;
        tok = strtok(NULL, ",");
    }
    free(tmp);
    return arr;
}

// parse positions list like "10/47/50,11/47/50" -> Pos array with count
static Pos* parse_pos_list(const char* s, int* out_count) {
    *out_count = 0;
    if (!s) return NULL;
    char* tmp = strdup(s);
    char* token = strtok(tmp, ",");
    Pos* arr = NULL;
    while (token) {
        // token format a/b/c
        int a = 0, b = 0, c = 0;
        sscanf(token, "%d/%d/%d", &a, &b, &c);
        arr = realloc(arr, sizeof(Pos) * (*out_count + 1));
        arr[*out_count].y = a;
        arr[*out_count].x = b;
        arr[*out_count].len = c;
        (*out_count)++;
        token = strtok(NULL, ",");
    }
    free(tmp);
    return arr;
}

// parse pattern in square brackets "[§■]" -> returns content inside (malloc'd)
static char* parse_bracket_pattern(const char* s) {
    const char* l = strchr(s, '[');
    const char* r = strrchr(s, ']');
    if (!l || !r || r <= l) return NULL;
    int len = r - l - 1;
    char* out = malloc(len + 1);
    memcpy(out, l + 1, len);
    out[len] = '\0';
    return out;
}

// Adds a new InteractionSet to global manager
static InteractionSet* create_interaction_set(const char* id) {
    InteractionSet* s = malloc(sizeof(InteractionSet));
    s->id = strdup(id);
    s->anims = NULL;
    s->anim_count = 0;
    s->color_keys = NULL;
    s->color_values = NULL;
    s->color_value_counts = NULL;
    s->color_key_count = 0;

    g_sets = realloc(g_sets, sizeof(InteractionSet*) * (g_set_count + 1));
    g_sets[g_set_count++] = s;
    return s;
}

void init_interactions_system() {
    g_sets = NULL;
    g_set_count = 0;

    load_interactions_file("assets/interfaces/interactions/skin.interact.dodjo", "skin");
    load_interactions_file("assets/interfaces/interactions/difficulty.interact.dodjo", "difficulty");
}

void destroy_interactions_system() {
    for (int si = 0; si < g_set_count; si++) {
        InteractionSet* s = g_sets[si];
        for (int ai = 0; ai < s->anim_count; ai++) {
            UIAnimation* a = s->anims[ai];
            free(a->positions);
            free(a->pattern);
            if (a->color_sequence) free(a->color_sequence);
            free(a);
        }
        for (int k = 0; k < s->color_key_count; k++) {
            free(s->color_keys[k]);
            free(s->color_values[k]);
        }
        free(s->anims);
        free(s->color_keys);
        free(s->color_values);
        free(s->color_value_counts);
        free(s->id);
        free(s);
    }
    free(g_sets);
    g_sets = NULL;
    g_set_count = 0;
}

// Add color definition to set: symbol like "§" (full UTF-8 string, not a single byte)
// and comma-separated ints "0,1,2"
static void add_color_definition(InteractionSet* s, const char* symbol_str, const char* val_list_str) {
    int cnt = 0;
    int* vals = parse_int_list(val_list_str, &cnt);
    if (!vals || cnt == 0) {
        if (vals) free(vals);
        return;
    }
    s->color_keys = realloc(s->color_keys, sizeof(char*) * (s->color_key_count + 1));
    s->color_values = realloc(s->color_values, sizeof(int*) * (s->color_key_count + 1));
    s->color_value_counts = realloc(s->color_value_counts, sizeof(int) * (s->color_key_count + 1));
    s->color_keys[s->color_key_count] = strdup(symbol_str);
    s->color_values[s->color_key_count] = vals;
    s->color_value_counts[s->color_key_count] = cnt;
    s->color_key_count++;
}

// find color sequence by full symbol string (handles multi-byte UTF-8 symbols)
static int* lookup_color_sequence(InteractionSet* s, const char* symbol, int* out_len) {
    for (int k = 0; k < s->color_key_count; k++) {
        if (strcmp(s->color_keys[k], symbol) == 0) {
            *out_len = s->color_value_counts[k];
            return s->color_values[k];
        }
    }
    *out_len = 0;
    return NULL;
}

// Expands anim->pattern into printable chars for the given position index.
// out_pattern/colors may be NULL if only the resulting length is needed
// (e.g. when clearing a previous draw). Centralizes the UTF-8-aware
// expansion logic that used to be triplicated across the draw/redraw/clear
// paths, driven by the real (possibly multi-byte) color_symbol rather than
// a hardcoded 2-byte constant.
static int expand_anim_pattern(UIAnimation* a, int idx, char* out_pattern, int* colors) {
    int out_len = 0;    // Index d'écriture dans out_pattern
    int color_idx = 0;  // Index d'écriture dans colors (colonnes)
    int symbol_len = a->has_color_placeholder ? (int)strlen(a->color_symbol) : 0;
    int i = 0;
    int pat_len = (int)strlen(a->pattern);

    while (i < pat_len) {
        if (symbol_len > 0 &&
            i + symbol_len <= pat_len &&
            memcmp(&a->pattern[i], a->color_symbol, symbol_len) == 0) {
            i += symbol_len;  // On saute le tag (§ ou @)

            if (i < pat_len) {
                // On récupère la taille en octets du caractère qui suit (ex: ■ = 3)
                int char_len = utf8_seq_len((unsigned char)a->pattern[i]);
                if (i + char_len > pat_len) char_len = pat_len - i;

                int color_val = COLOR_DEFAULT;
                if (a->color_sequence_len > 0) {
                    color_val = a->color_sequence[idx % a->color_sequence_len];
                }

                // Une seule couleur pour ce caractère multi-octets
                if (colors) colors[color_idx] = color_val;
                color_idx++;

                // On copie TOUS les octets de ce caractère coloré
                for (int c = 0; c < char_len; c++) {
                    if (out_pattern) out_pattern[out_len] = a->pattern[i];
                    out_len++;
                    i++;
                }
            }
        } else {
            // Caractère normal (ex: '[', ']', ou lettre)
            int char_len = utf8_seq_len((unsigned char)a->pattern[i]);
            if (i + char_len > pat_len) char_len = pat_len - i;

            if (colors) colors[color_idx] = COLOR_DEFAULT;
            color_idx++;

            // On copie tous les octets du caractère normal
            for (int c = 0; c < char_len; c++) {
                if (out_pattern) out_pattern[out_len] = a->pattern[i];
                out_len++;
                i++;
            }
        }
    }
    if (out_pattern) out_pattern[out_len] = '\0';

    return color_idx;  // Renvoie le nombre de colonnes pour draw_pattern_at et clear_pattern_at
}

// Parse interaction file lines. We expect the file only contains the interactions part.
// Lines starting with '(' => color def. Lines starting with '[' => animation.
// Comments or empty lines ignored.
bool load_interactions_file(const char* filename, const char* id) {
    if (!filename || !id) return false;
    FILE* f = fopen(filename, "r");
    if (!f) return false;

    // Skip a UTF-8 BOM if present. Files saved/edited on Windows are
    // sometimes written as "UTF-8 with BOM"; without this check the BOM
    // bytes end up prefixed to the first line, l[0] never matches '(' / '['
    // / '#', and the entire first line is silently dropped.
    {
        unsigned char bom[3];
        size_t n = fread(bom, 1, 3, f);
        if (!(n == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
            fseek(f, 0, SEEK_SET);
        }
    }

    InteractionSet* set = create_interaction_set(id);

    char* line = NULL;
    size_t sz = 0;
    ssize_t nread;
    while ((nread = getline(&line, &sz, f)) != -1) {
        if (nread <= 0) continue;
        char* l = strtrim(line);
        if (!l) continue;
        if (l[0] == '\0' || l[0] == '#') continue;

        if (l[0] == '(') {
            // color definition form: (§):{0,1,2,3,4,5}
            // The symbol may be a multi-byte UTF-8 character (e.g. § is
            // 0xC2 0xA7), so we can't rely on sscanf's %c (single byte)
            // to capture it correctly. Parse manually and copy the full
            // UTF-8 sequence.
            char* p = strchr(l, '(');
            char* q = strchr(l, ')');
            char* r = strchr(l, '{');
            char* s2 = strchr(l, '}');
            if (p && q && r && s2 && q > p && s2 > r) {
                int avail = (int)(q - p - 1);
                int sym_len = utf8_seq_len((unsigned char)*(p + 1));
                if (sym_len > avail) sym_len = avail;  // guard malformed lines
                if (sym_len > 0) {
                    char sstr[5] = {0};
                    memcpy(sstr, p + 1, sym_len);
                    sstr[sym_len] = '\0';

                    int lenv = s2 - r - 1;
                    char* val = malloc(lenv + 1);
                    memcpy(val, r + 1, lenv);
                    val[lenv] = '\0';
                    add_color_definition(set, sstr, val);
                    free(val);
                }
            }
        } else if (l[0] == '[') {
            // animation line:
            // [a,b]:{pos,pos,..}:[pattern]
            int a = -1, b = -1, w = 0;
            char posbuf[1024] = {0};
            char patternbuf[256] = {0};
            // We'll parse roughly: scan "[a,b] : { ... } : [ ... ]"
            char* p_close = strchr(l, ']');
            if (!p_close) continue;
            // parse a,b
            {
                char tmp[64] = {0};
                int len = p_close - l - 1;
                if (len > 0 && len < (int)sizeof(tmp)) {
                    memcpy(tmp, l + 1, len);
                    tmp[len] = '\0';
                    // tmp should be "a,b"
                    if (sscanf(tmp, "%d , %d , %d", &a, &b, &w) != 2) {
                        sscanf(tmp, "%d,%d,%d", &a, &b, &w);
                    }
                }
            }
            // find first '{' after p_close
            char* p_brace = strchr(p_close, '{');
            char* p_brace_close = p_brace ? strchr(p_brace, '}') : NULL;
            if (p_brace && p_brace_close) {
                int len = p_brace_close - p_brace - 1;
                if (len > 0 && len < (int)sizeof(posbuf)) {
                    memcpy(posbuf, p_brace + 1, len);
                    posbuf[len] = '\0';
                }
            }
            // find last '[' ... ']' for pattern
            char* p_pat_l = strchr(p_brace_close ? p_brace_close : p_close, '[');
            char* p_pat_r = p_pat_l ? strchr(p_pat_l, ']') : NULL;
            if (p_pat_l && p_pat_r && p_pat_r > p_pat_l) {
                int len = p_pat_r - p_pat_l - 1;
                if (len > 0 && len < (int)sizeof(patternbuf)) {
                    memcpy(patternbuf, p_pat_l + 1, len);
                    patternbuf[len] = '\0';
                }
            }

            // Build UIAnimation
            UIAnimation* anim = malloc(sizeof(UIAnimation));
            memset(anim, 0, sizeof(UIAnimation));
            anim->act_dec = a;
            anim->act_inc = b;
            anim->weight = w;
            anim->positions = parse_pos_list(posbuf, &anim->pos_count);
            anim->pattern = parse_bracket_pattern(p_pat_l ? p_pat_l - 1 : "[]");  // safe fallback
            if (!anim->pattern) {
                // fallback to patternbuf
                anim->pattern = strdup(patternbuf);
            }
            // detect color placeholder inside pattern: any symbol string that
            // is a defined key in set (e.g. "§", handled as full UTF-8 bytes)
            anim->has_color_placeholder = false;
            memset(anim->color_symbol, 0, sizeof(anim->color_symbol));
            anim->color_sequence = NULL;
            anim->color_sequence_len = 0;
            anim->current_index = 0;

            for (int k = 0; k < set->color_key_count; k++) {
                // Use strstr to find the symbol string (handles multi-byte UTF-8 chars like §)
                if (strstr(anim->pattern, set->color_keys[k])) {
                    anim->has_color_placeholder = true;
                    strncpy(anim->color_symbol, set->color_keys[k], sizeof(anim->color_symbol) - 1);
                    // fetch color sequence; but sequence length may differ from positions count
                    int seq_len = 0;
                    int* seq = lookup_color_sequence(set, set->color_keys[k], &seq_len);
                    if (seq && seq_len > 0) {
                        anim->color_sequence = malloc(sizeof(int) * seq_len);
                        memcpy(anim->color_sequence, seq, sizeof(int) * seq_len);
                        anim->color_sequence_len = seq_len;
                    }
                    break;
                }
            }

            // Append anim to set
            set->anims = realloc(set->anims, sizeof(UIAnimation*) * (set->anim_count + 1));
            set->anims[set->anim_count++] = anim;
        }
    }

    if (line) free(line);
    fclose(f);
    return true;
}

void unload_interactions(const char* id) {
    InteractionSet* set = find_set_by_id(id);
    if (!set) return;
    // free set content
    for (int ai = 0; ai < set->anim_count; ai++) {
        UIAnimation* a = set->anims[ai];
        free(a->positions);
        if (a->pattern) free(a->pattern);
        if (a->color_sequence) free(a->color_sequence);
        free(a);
    }
    for (int k = 0; k < set->color_key_count; k++) {
        free(set->color_keys[k]);
        free(set->color_values[k]);
    }
    free(set->anims);
    free(set->color_keys);
    free(set->color_values);
    free(set->color_value_counts);
    free(set->id);
    // remove from g_sets array
    int idx = -1;
    for (int i = 0; i < g_set_count; i++)
        if (g_sets[i] == set) {
            idx = i;
            break;
        }
    if (idx >= 0) {
        for (int j = idx; j < g_set_count - 1; j++) g_sets[j] = g_sets[j + 1];
        g_sets = realloc(g_sets, sizeof(InteractionSet*) * (g_set_count - 1));
        g_set_count--;
    }
    free(set);
}

// main loop: display interface and handle interactions
int* display_interface_with_interactions_main(Render_Buffer* r, const char* visual_filename, const char* interaction_id, int* out_selected_indices) {
    *out_selected_indices = 0;
    // open visual file and fill board like display_interface
    FILE* file = fopen(visual_filename, "r");
    if (!file) {
        perror("Error opening visual file");
        return NULL;
    }
    InteractionSet* set = find_set_by_id(interaction_id);

    setup_render_buffer(r);

    read_text_into_render(r, file);
    fclose(file);

    // If no set found: just update and wait like old display_interface
    if (!set || set->anim_count == 0) {
        update_screen(r);
        while (!USE_KEY('H') && !USE_KEY('h'));
        finalize_render_buffer(r);
        return NULL;
    }

    // Draw initial animations (index 0 for all)
    for (int ai = 0; ai < set->anim_count; ai++) {
        UIAnimation* a = set->anims[ai];
        if (!a || a->pos_count == 0) continue;
        int idx = a->current_index % a->pos_count;
        Pos p = a->positions[idx];

        char out_pattern[256] = {0};
        int per_char_colors[256];
        int out_len = expand_anim_pattern(a, idx, out_pattern, per_char_colors);

        draw_pattern_at(r, p, out_pattern, COLOR_DEFAULT, true, per_char_colors, out_len);
    }

    update_screen(r);

    // Main interaction loop: update animations when corresponding actions keys pressed.
    // We'll sample keys with USE_KEY for the mapped chars (g_dir_keys).
    while (1) {
        // exit checks
        if (USE_KEY('H') || USE_KEY('h') || USE_KEY('\n') || USE_KEY(' ')) break;

        // Poll all direction keys and produce action ids 0..3
        bool any_action = false;
        int action_indices[4] = {0, 0, 0, 0};
        for (int d = 0; d < 4; d++) {
            if (USE_KEY(g_dir_keys[d]) || USE_KEY(g_mdir_keys[d])) {
                action_indices[d] = 1;
                any_action = true;
            }
        }
        if (!any_action) {
            usleep(16000);  // ~60 fps idle
            continue;
        }

        // For each animation, check if it listens to the action pair
        for (int ai = 0; ai < set->anim_count; ai++) {
            UIAnimation* a = set->anims[ai];
            if (!a) continue;
            int old_index = a->current_index;
            bool changed = false;
            // For each direction d (0..3), if pressed and equals act_inc or act_dec...
            for (int d = 0; d < 4; d++) {
                if (!action_indices[d]) continue;
                if (d == a->act_inc) {
                    a->current_index = (a->current_index + 1) % a->pos_count;
                    changed = true;
                } else if (d == a->act_dec) {
                    a->current_index = (a->current_index - 1);
                    if (a->current_index < 0) a->current_index += a->pos_count;
                    changed = true;
                }
            }
            if (changed) {
                // erase old
                Pos oldp = a->positions[old_index];
                int pat_len = expand_anim_pattern(a, old_index, NULL, NULL);
                clear_pattern_at(r, oldp, pat_len);

                // draw new
                int idx = a->current_index % a->pos_count;
                Pos p = a->positions[idx];

                char out_pattern[256] = {0};
                int per_char_colors[256];
                int out_len = expand_anim_pattern(a, idx, out_pattern, per_char_colors);

                draw_pattern_at(r, p, out_pattern, COLOR_DEFAULT, true, per_char_colors, out_len);
            }
        }

        // update changed rows and push to terminal
        update_screen(r);
        // small debounce
        usleep(80000);
    }

    *out_selected_indices = set->anim_count;
    int* selected_indices = malloc(sizeof(int) * set->anim_count);
    for (int ai = 0; ai < set->anim_count; ai++) {
        UIAnimation* a = set->anims[ai];
        selected_indices[ai] = a->current_index % a->pos_count;
    }

    finalize_render_buffer_silent(r);

    return selected_indices;
}