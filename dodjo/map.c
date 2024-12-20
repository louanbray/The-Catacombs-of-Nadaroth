#include "hash.h";
#include "map.h";

enum Type;
enum Direction {
  STARGATE,
  NORTH,
  EAST,
  SOUTH,
  WEST
};

typedef void* elements;
typedef struct chunk {
  link link;
  int x;
  int y;
  type Type;
  elements elements;
}

typedef struct link {
  chunk* stargate;
  chunk* north;
  chunk* east;
  chunk* south;
  chunk* west;
}


typedef struct map {
  hm* hashmap;
  chunk* spawn;
} map;
