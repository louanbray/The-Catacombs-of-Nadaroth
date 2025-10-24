# Système de Sauvegarde V3 - Documentation

## 📋 Vue d'ensemble

Le système de sauvegarde a été complètement revu pour sauvegarder l'état exact de la map, incluant :
- ✅ Tous les chunks visités
- ✅ L'état des ennemis (HP, dégâts subis, morts)
- ✅ Les coffres ouverts/pillés
- ✅ Les items ramassés
- ✅ La topologie de la map (liens entre chunks)

## 🆕 Changements par rapport à V2

### Version de sauvegarde
- **V2** : Sauvegarde minimale (joueur + hotbar uniquement)
- **V3** : Sauvegarde complète de la map avec tous les chunks

⚠️ **Attention** : Pas de rétrocompatibilité avec V2. Les anciennes sauvegardes ne fonctionneront pas.

## 📦 Structure de la sauvegarde

```
SAVE FILE V3
├── Magic Number (0x4E414430 = "NAD0")
├── Version (3)
├── PLAYER DATA
│   ├── Position (x, y, px, py)
│   ├── Stats (health, max_health, mental_health, damage, etc.)
│   ├── Design, score, deaths, phase
│   └── Current chunk coordinates (chunk_x, chunk_y) ← NOUVEAU !
├── HOTBAR DATA
│   ├── Taille et slot sélectionné
│   └── Pour chaque slot : item complet avec specs
├── MAP DATA
    ├── Nombre de chunks
    └── Pour chaque chunk :
        ├── Coordonnées (x, y, spawn_x, spawn_y)
        ├── Type de chunk (ChunkType)
        ├── Liens vers les 5 directions (coordonnées des chunks liés)
        ├── ITEMS du chunk
        │   ├── Nombre d'items
        │   └── Pour chaque item :
        │       ├── Position (x, y)
        │       ├── Type, display, hidden, used, usable_type
        │       ├── Si ENEMY : specs complètes (hp, damage, speed, etc.)
        │       └── Si LOOTABLE : contenu (bronze, silver, gold, nadino)
        └── ENEMIES du chunk
            ├── Nombre d'ennemis
            └── Pour chaque ennemi :
                ├── Brain item (avec specs enemy)
                ├── Nombre de parts de l'entité
                └── Pour chaque part : item complet
```

## 🔧 Fonctions modifiées/ajoutées

### Dans `save_manager.c` :

#### Nouvelles fonctions privées :
- `save_item_data(FILE* f, item* it)` : Sauvegarde un item avec ses specs
- `load_item_data(FILE* f, chunk* c, dynarray* items)` : Charge un item
- `save_chunk_data(FILE* f, chunk* ck)` : Sauvegarde un chunk complet
- `count_chunk_callback()` : Callback pour compter les chunks
- `save_chunk_callback()` : Callback pour sauvegarder chaque chunk

#### Fonctions modifiées :
- `save_map_data()` : Maintenant sauvegarde TOUS les chunks de la hashmap
- `load_map_data()` : Reconstruit tous les chunks avec leur état exact

### Dans `hash.h/c` :
- `for_each_hm()` : Nouvelle fonction pour itérer sur tous les éléments d'une hashmap

### Dans `generation.h/c` :
- `get_chunk_type()` : Getter pour le type du chunk
- `get_chunk_links()` : Getter pour les liens du chunk

### Dans `map.h/c` :
- `get_map_hashmap()` : Getter pour accéder à la hashmap de chunks

## 💾 Que sauvegarde-t-on exactement ?

### Pour chaque CHUNK :
1. **Métadonnées** :
   - Coordonnées (x, y)
   - Position de spawn (spawn_x, spawn_y)
   - Type de chunk (permet de savoir comment il a été généré)

2. **Liens vers les chunks adjacents** :
   - 5 directions : STARGATE, EAST, NORTH, WEST, SOUTH
   - Stocke les coordonnées des chunks liés (ou -9999 si pas de lien)

3. **Tous les items** :
   - Position, type, affichage
   - États : caché, utilisé
   - **Pour les ENNEMIS** : HP actuels, dégâts, vitesse, score, etc.
   - **Pour les LOOTABLES** : Contenu restant (clés bronze/silver/gold/nadino)

4. **Toutes les entités** :
   - Le "brain" (item principal avec les stats)
   - Toutes les "parts" (parties visuelles de l'entité)
   - État de santé de chaque partie

## 🎮 Cas d'usage

### Ennemi blessé :
1. Le joueur tire sur un ennemi qui a 10 HP
2. L'ennemi perd 3 HP → il lui reste 7 HP
3. Le joueur sauvegarde
4. Au chargement : l'ennemi est recréé avec le type du chunk MAIS son HP est restauré à 7

### Coffre pillé :
1. Le joueur ouvre un coffre et prend 2 clés bronze sur 3
2. Le coffre a maintenant : bronze=1, silver=0, gold=0, nadino=0
3. Le joueur sauvegarde
4. Au chargement : le coffre est recréé avec seulement 1 clé bronze restante

### Item ramassé :
1. Le joueur ramasse une golden apple dans un chunk
2. L'item est marqué `used=true` ou retiré du chunk
3. Le joueur sauvegarde
4. Au chargement : la golden apple n'apparaît plus dans le chunk

## 🔍 Comment ça marche

### Sauvegarde :
1. Itération sur tous les chunks de la hashmap via `for_each_hm()`
2. Pour chaque chunk :
   - Sauvegarde des métadonnées
   - Sauvegarde de tous les items un par un
   - Sauvegarde de tous les ennemis avec leurs parties

### Chargement :
1. Lecture du nombre de chunks
2. Pour chaque chunk :
   - Création ou récupération du chunk
   - **Si le chunk existe déjà : nettoyage complet** (suppression des anciens items)
   - Chargement des items dans l'ordre
   - Reconstruction des entités avec leurs parties
   - Restauration des specs (HP des ennemis, contenu des coffres, etc.)
   - **Vérification des coffres vides** : si un coffre a été complètement pillé (tous les compteurs à 0), il n'est PAS recréé

### Reconstruction des liens :
Les liens entre chunks sont sauvegardés mais pas immédiatement restaurés au chargement.
Ils seront naturellement reconstruits quand le joueur se déplace via `get_chunk_from()`.

### Gestion des coffres pillés :
- Quand un coffre est pillé, l'entité est supprimée du chunk via `remove_entity_from_chunk()`
- Lors de la sauvegarde, les items supprimés sont marqués comme `item_exists = false`
- Lors du chargement :
  - Si un coffre (LOOTABLE) a tous ses compteurs à 0, il n'est PAS recréé
  - Le chunk existant est nettoyé avant d'être rechargé pour éviter les doublons

## 🚀 Performance

### Taille des fichiers :
- **V2** : ~1-2 Ko (seulement joueur + hotbar)
- **V3** : Variable selon l'exploration
  - ~10-50 Ko pour une petite exploration (10-20 chunks)
  - ~100-500 Ko pour une grande exploration (100+ chunks)
  - Dépend du nombre d'items et d'ennemis par chunk

### Temps de sauvegarde/chargement :
- Sauvegarde : O(n × m) où n = nombre de chunks, m = items par chunk
- Chargement : O(n × m) idem
- Généralement < 100ms pour des sauvegardes normales

## ⚠️ Limitations et notes

1. **Pas de compression** : Les fichiers sont en binaire brut
2. **Pas de checksum** : Pas de vérification d'intégrité
3. **Liens partiels** : Les liens entre chunks ne sont pas complètement restaurés (mais se reconstruisent naturellement)
4. **Mémoire** : Tous les chunks restent en mémoire (pas de garbage collection des chunks lointains)

## 🔮 Améliorations futures possibles

1. **Compression** : Utiliser zlib pour réduire la taille des fichiers
2. **Checksum/CRC** : Vérifier l'intégrité des sauvegardes
3. **Sauvegarde incrémentale** : Ne sauvegarder que les chunks modifiés
4. **Garbage collection** : Supprimer les chunks loin du joueur
5. **Versioning** : Support de plusieurs versions avec migration
6. **Encryption** : Chiffrer les sauvegardes pour éviter la triche

## 📝 Exemple de flux

```c
// SAUVEGARDE
save_game("save.dat", player, map, hotbar);
// -> Sauvegarde le joueur
// -> Sauvegarde l'inventaire
// -> Itère sur TOUS les chunks de la map
// -> Pour chaque chunk : sauvegarde items + ennemis + état

// CHARGEMENT
load_game("save.dat", player, map, hotbar);
// -> Charge le joueur
// -> Charge l'inventaire
// -> Lit le nombre de chunks
// -> Pour chaque chunk : recrée le chunk avec son état exact
// -> Les ennemis blessés ont leurs HP restaurés
// -> Les coffres ont leur contenu restauré
```

## ✅ Tests recommandés

1. **Test de base** :
   - Créer un nouveau jeu
   - Visiter 5 chunks
   - Blesser un ennemi sans le tuer
   - Sauvegarder
   - Charger → Vérifier que l'ennemi a les bons HP

2. **Test de coffre** :
   - Ouvrir un coffre partiellement
   - Sauvegarder
   - Charger → Vérifier que le contenu est correct

3. **Test de grande map** :
   - Explorer 50+ chunks
   - Sauvegarder (vérifier la taille du fichier)
   - Charger (vérifier le temps de chargement)

4. **Test de corruption** :
   - Tenter de charger une sauvegarde V2 (devrait échouer proprement)
   - Tenter de charger un fichier corrompu

---

**Auteur** : Système de sauvegarde V3 - Octobre 2025
**Version** : 3.0
**Compatibilité** : The Catacombs of Nadaroth
