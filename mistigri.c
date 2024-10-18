#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*

1) Définition des types et CHOIX DE LA STRUCTURE DE DONNÉES
2) Initialisation du paquet de cartes
3) Mélange du paquet de cartes
4) Distribution des cartes aux nb_player joueurs
5) Suppression de doublons : complexité ?
6) Tours de jeu : pioche d'une carte au hasard, suppression éventuelle de la carte piochée avec sa paire : complexité ?, ajout de la carte dans la main du joueur sinon.
7) Ecriture de la fonction STATS

*/
bool DEBUG = true;
/// 1) Type pour les cartes :
typedef int card;
// Convention : les cartes sont numérotées de -N à N.
// N est donné en entrée de la fonction stats et devra être passé en argument dans toutes les fonctions auxilaires utiles.

// Un 'deck' (paquet de carte en anglais) est un tableau de cartes :
typedef card *deck;

/// 1) Type pour la main d'un joueur :
struct hand_s {
    int nb_cards;
    card *cards;
};
typedef struct hand_s hand;
// Le joueur a nb_cards cartes en main, stocké à l'adresse 'cards'.

/// 1) Type pour stocker LES MAINS des joueurs :
typedef hand *hands;  // C'est un tableau de 'mains', de longueur nb_player, variable qui est passée explicitement en argument là où nécessaire.

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

void play(hands jeu, int player, int nb, int *ids) {
    if (jeu[ids[player]].nb_cards != 0 && nb != 1) {
        int joueur = (player == 0) ? ids[nb - 1] : ids[player - 1];
        int size = jeu[joueur].nb_cards;
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

int clear_losers(int *ids, hands jeu, int player, int player_in_game) {
    if (jeu[ids[player]].nb_cards == 0) {
        remove_player(ids, player, player_in_game);
        player_in_game -= 1;
    }
    for (int i = 0; i < player_in_game; i++) {
        if (jeu[ids[i]].nb_cards == 0) {
            remove_player(ids, i, player_in_game);
            player_in_game -= 1;
        }
    }
    return player_in_game;
}

void game_execution(deck cartes, int N, int nb_player, int *ratio) {
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
            play(jeu, player, player_in_game, ids);
            player_in_game = clear_losers(ids, jeu, player, player_in_game);
        }
    }
    ratio[ids[0]] += 1;

    free_game(jeu, nb_player);
    free(ids);
}

// STATS

int *init_ratio(int N) {
    int *ratio = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++) {
        ratio[i] = 0;
    }
    return ratio;
}

void stats(int N, int nb_game, int nb_player) {
    clock_t begin = clock();

    int *ratio = init_ratio(nb_player);

    deck cartes = raw_deck(N);

    for (int game = 0; game < nb_game; game++) {
        game_execution(cartes, 2 * N + 1, nb_player, ratio);
    }
    free(cartes);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Temps moyen de jeu : %f\n", time_spent / nb_game);

    for (int i = 0; i < nb_player; i++) {
        printf("Taux de defaite du joueur %d : %f%%\n", i + 1, (100 * ratio[i]) / (nb_game + 0.));
    }
}

int main() {
    srand(time(NULL));
    stats(25, 100, 8);
}