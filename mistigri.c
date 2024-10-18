#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*

1) DC)finition des types et CHOIX DE LA STRUCTURE DE DONNC	ES
2) Initialisation du paquet de cartes
3) MC)lange du paquet de cartes
4) Distribution des cartes aux nb_player joueurs
5) Suppression de doublons : complexitC) ?
6) Tours de jeu : pioche d'une carte au hasard, suppression C)ventuelle de la carte piochC)e avec sa paire : complexitC) ?, ajout de la carte dans la main du joueur sinon.
7) Ecriture de la fonction STATS

*/
bool DEBUG = true;
/// 1) Type pour les cartes :
typedef int card;
// Convention : les cartes sont numC)rotC)es de -N C  N.
// N est donnC) en entrC)e de la fonction stats et devra C*tre passC) en argument dans toutes les fonctions auxilaires utiles.

// Un 'deck' (paquet de carte en anglais) est un tableau de cartes :
typedef card *deck;

/// 1) Type pour la main d'un joueur :
struct hand_s {
    int nb_cards;
    card *cards;
};
typedef struct hand_s hand;
// Le joueur a nb_cards cartes en main, stockC) C  l'adresse 'cards'.

/// 1) Type pour stocker LES MAINS des joueurs :
typedef hand *hands;  // C'est un tableau de 'mains', de longueur nb_player, variable qui est passC)e explicitement en argument lC  oC9 nC)cessaire.

struct stat_s {
    int **pos;
    int nb_game;
    int nb_player;
    int loser_id;
    clock_t *timestamp;
};
typedef struct stat_s stat;

/* ----------- MELANGE DE CARTE -------------- */

/// DRAWING UTILS

void print_cards(card *cards, int len) {
    for (int i = 0; i < len; i++) {
        printf("%d ", cards[i]);
    }
    printf(" (%d cartes)\n", len);
}

void print_hand(hands main, int p) {
    print_cards(main[p].cards, main[p].nb_cards);
}

void print_hands(hands jeu, int players) {
    printf("\n\nJeu actuel (%d joueurs vivants): \n", players);
    for (int i = 0; i < players; i++) {
        print_hand(jeu, i);
    }
}

/// UTILS
void swap(deck a, int i, int j) {
    card tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
}

card abs(int x) {
    return (x < 0) ? -x : x;
}

void insert(hands jeu, int p, card c) {
    jeu[p].cards[jeu[p].nb_cards] = c;
    jeu[p].nb_cards += 1;
}

void remove_card(hands jeu, int p, int i) {
    swap(jeu[p].cards, i, jeu[p].nb_cards - 1);
    jeu[p].nb_cards -= 1;
}

/// GENERATION

deck raw_deck(int N) {
    deck j = malloc(sizeof(card) * (2 * N + 1));
    for (int i = -N; i <= N; i++) {
        j[i + N] = i;
    }
    return j;
}

hands raw_hands(int N, int p) {
    hands jeu = malloc(sizeof(hand) * p);
    for (int i = 0; i < p; i++) {
        jeu[i].cards = malloc(sizeof(card) * (N / 2 + 1));
        jeu[i].nb_cards = 0;
    }
    return jeu;
}

/// INITIALISER

// Algorithme Fisher-Yates
void shuffle(int len, deck a) {
    int j = 0;
    for (int i = len - 1; i > 0; i--) {
        j = rand() % (i + 1);
        swap(a, i, j);
    }
}

hands distribue(int N, deck cartes, int p) {
    int j = 0;
    int i = 0;

    hands jeu = raw_hands(N, p);
    while (i < N) {
        for (int player = 0; player < p; player++) {
            jeu[player].cards[j] = cartes[i];
            jeu[player].nb_cards += 1;
            i++;
            if (i >= N) {
                break;
            }
        }
        j++;
    }
    return jeu;
}

// GAME UTILS

void remove_player(int *ids, int player, int nb) {
    for (int i = 0; i < nb - 1 - player; i++) {
        swap(ids, player, nb - 1 - i);
    }
}

void remove_pair(hands jeu, int p, int i, int j) {
    if (j == jeu[p].nb_cards - 1) {
        j = i;
    }
    remove_card(jeu, p, i);
    remove_card(jeu, p, j);
}

void destroy_pair(hands j, int p) {
    int c = 1;
    while (c) {
        c = 0;
        for (int i = 0; (i < j[p].nb_cards) && (c == 0); i++) {
            for (int k = 0; k < j[p].nb_cards; k++) {
                if (k != i && j[p].cards[i] + j[p].cards[k] == 0 && j[p].cards[i] != 0) {
                    remove_pair(j, p, i, k);
                    c++;
                    break;
                }
            }
        }
    }
}

void destroy_pairs(hands jeu, int p) {
    for (int i = 0; i < p; i++) {
        destroy_pair(jeu, i);
    }
}

// TURN EXECUTION

int clear_losers(int *ids, hands jeu, int player, int player_in_game, stat data, clock_t begin) {
    int id_player = ids[player];
    if (jeu[id_player].nb_cards == 0) {
        data.pos[id_player][data.nb_player - player_in_game] += 1;
        data.timestamp[data.nb_player - player_in_game] += clock() - begin;
        remove_player(ids, player, player_in_game);
        player_in_game--;
    }
    for (int i = 0; i < player_in_game; i++) {
        if (jeu[ids[i]].nb_cards == 0) {
            data.pos[ids[i]][data.nb_player - player_in_game] += 1;
            data.timestamp[data.nb_player - player_in_game] += clock() - begin;
            remove_player(ids, i, player_in_game);
            player_in_game--;
        }
    }
    return player_in_game;
}

void play(hands jeu, int player, int *nb, int *ids, stat data, clock_t begin) {
    if (jeu[ids[player]].nb_cards != 0 && *nb != 1) {
        int joueur = (player == 0) ? ids[*nb - 1] : ids[player - 1];
        int size = jeu[joueur].nb_cards;
        if (player == 0 && size == 0) {
            *nb = clear_losers(ids, jeu, player, *nb, data, begin);
            joueur = (player == 0) ? ids[*nb - 1] : ids[player - 1];
            size = jeu[joueur].nb_cards;
        }
        int index = rand() % size;
        card carte = jeu[joueur].cards[index];

        insert(jeu, ids[player], carte);
        remove_card(jeu, joueur, index);
        destroy_pair(jeu, ids[player]);
    }
}

// GAME EXECUTION

void free_game(hands jeu, int nb_players) {
    for (int i = 0; i < nb_players; i++) {
        free(jeu[i].cards);
    }
    free(jeu);
}

void game_execution(deck cartes, int N, int nb_player, stat data, clock_t begin) {
    int *ids = malloc(sizeof(int) * nb_player);
    for (int i = 0; i < nb_player; i++) {
        ids[i] = i;
    }

    shuffle(N, cartes);
    hands jeu = distribue(N, cartes, nb_player);
    destroy_pairs(jeu, nb_player);
    int player_in_game = nb_player;

    while (player_in_game > 1) {
        for (int player = 0; player < player_in_game; player++) {
            play(jeu, player, &player_in_game, ids, data, begin);
            player_in_game = clear_losers(ids, jeu, player, player_in_game, data, begin);
        }
    }
    data.loser_id = ids[0];
    data.pos[ids[0]][nb_player - 1] += 1;
    data.timestamp[nb_player - 1] += clock() - begin;

    free_game(jeu, nb_player);
    free(ids);
}

void execute_all(int N, stat data) {
    deck cartes = raw_deck(N);

    for (int game = 0; game < data.nb_game; game++) {
        clock_t begin = clock();
        game_execution(cartes, 2 * N + 1, data.nb_player, data, begin);
    }
    free(cartes);
}

// STATS

int **init_player_data(int p) {
    int **player_data = malloc(sizeof(int *) * p);
    for (int i = 0; i < p; i++) {
        int *pos = malloc(sizeof(int) * p);
        for (int j = 0; j < p; j++) {
            pos[j] = 0;
        }
        player_data[i] = pos;
    }
    return player_data;
}

clock_t *init_time_stamp(int p) {
    clock_t *ts = malloc(sizeof(clock_t) * p);
    for (int i = 0; i < p; i++) {
        ts[i] = 0;
    }
    return ts;
}

void stats(int N, int nb_game, int nb_player) {
    clock_t begin = clock();
    stat data = {init_player_data(nb_player), nb_game, nb_player, 0, init_time_stamp(nb_player)};

    execute_all(N, data);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Temps moyen de jeu : %f", time_spent / nb_game);

    printf("\nSortie du jeu : ");
    for (int i = 0; i < nb_player; i++) {
        printf("\nJoueur %d: ", i + 1);
        for (int j = 0; j < nb_player; j++) {
            printf(" %f%%,", (100 * data.pos[i][j]) / (nb_game + 0.));
        }
    }
    for (int j = 0; j < nb_player; j++) {
        printf("\nFin de %deme sortie: ", j + 1);
        double ts = (double)(data.timestamp[j]) / CLOCKS_PER_SEC;
        printf(" %f,", ts / nb_game);
    }
}

int main() {
    srand(time(NULL));
    stats(500, 1000, 40);
}