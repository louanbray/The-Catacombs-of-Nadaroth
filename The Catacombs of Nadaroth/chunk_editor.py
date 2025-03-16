from enum import IntEnum

class Type(IntEnum): # Item Type
    WALL = 0
    GATE = 1
    SGATE = 2
    PICKABLE = 3
    ENEMY = 4

class Size(IntEnum): # Space between identical items
    COLLAPSE = 0
    SPACED = 1

class Sprite(IntEnum):
    WALL = 69
    STARGATE = 9055
    VGATE = 9608
    UGATE = 9600
    DGATE = 9604
    ENTITY = 0
    KEY = 9919
    A = 65
    B = 66
    G = 71
    O = 79
    S = 83

class UsableItem(IntEnum):
    NOT_USABLE_ITEM = 0
    BASIC_BOW = 1
    ADVANCED_BOW = 2
    SUPER_BOW = 3
    NADINO_BOW = 4
    BRONZE_KEY = 5
    SILVER_KEY = 6
    GOLD_KEY = 7 
    NADINO_KEY = 8
    ONION_RING = 9 
    STOCKFISH = 10
    SCHOOL_DISHES = 11
    GOLDEN_APPLE = 12
    BOMB = 13

class Entity(IntEnum):
    NOENTITY = 0
    ENEMY1 = 1

CHUNK_WIDTH = 127 # RENDER_WIDTH - 2 for the walls
CHUNK_HEIGHT = 35 # without the hotbar space

items = {"V":[Type.GATE,Sprite.VGATE,Size.COLLAPSE,Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
         "U":[Type.GATE,Sprite.UGATE,Size.COLLAPSE,Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
         "D":[Type.GATE,Sprite.DGATE,Size.COLLAPSE, Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
         "T":[Type.WALL,Sprite.WALL,Size.COLLAPSE,Entity.NOENTITY, UsableItem.NOT_USABLE_ITEM],
         "B":[Type.PICKABLE,Sprite.B,Size.COLLAPSE,Entity.ENEMY1, UsableItem.BOMB],
         "S":[Type.PICKABLE,Sprite.G,Size.COLLAPSE,Entity.ENEMY1, UsableItem.GOLDEN_APPLE],
         "G":[Type.PICKABLE,Sprite.O,Size.COLLAPSE,Entity.ENEMY1, UsableItem.ONION_RING],
         "N":[Type.PICKABLE,Sprite.S,Size.COLLAPSE,Entity.ENEMY1, UsableItem.STOCKFISH],
         "E":[Type.ENEMY,Sprite.STARGATE,Size.COLLAPSE,Entity.ENEMY1, UsableItem.NOT_USABLE_ITEM]}

        #                                                               C
        #                                                               E
        #                                                               N
        #                                                               T
        #                                                               E
        #                                                               R
        #----------------------------------------------------------------ඞ----------------------------------------------------------------
chunk =[" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *UUUUUU * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *E* * * ",
        "V* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *V",
        "V* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *V",#CENTER 
        "V* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *V",
        " * * * *E* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * *E* * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * ",
        " * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *DDDDDD * * * * * * * * * * * * * * * * * * * * * * * * * * * * * "]
        #----------------------------------------------------------------ඞ----------------------------------------------------------------
        #                                                               C
        #                                                               E
        #                                                               N
        #                                                               T
        #                                                               E
        #                                                               R

if __name__ == "__main__":
    for i in range(CHUNK_HEIGHT):
        row_repeat = col_repeat = 1
        
        row = []
        row[:] = chunk[i]
        for j in range(CHUNK_WIDTH):
            if row[j] == '*' or row[j] == ' ':
                pass
            elif j < CHUNK_WIDTH-1-items[row[j]][2] and row[j] == row[j+1+items[row[j]][2]]:
                row_repeat += 1              
            else:
                if i < CHUNK_HEIGHT - 1 and row_repeat == 1:
                    for k in range(CHUNK_HEIGHT - 1 - i):
                        col = []
                        col[:] = chunk[i+1+k]
                        if row[j] == col[j]:
                            col_repeat += 1
                            col[j] = ' '
                            chunk[i+1+k] = "".join(col)
                        else:
                            break
                print(f"{j-63},{-i+17},{items[row[j]][0]},{items[row[j]][1]},{row_repeat},{items[row[j]][2]},{col_repeat},{items[row[j]][3]},{items[row[j]][4]}") # X, Y, TYPE, DISPLAY CHAR, ROW REPEAT, SPACE, COL REPEAT, ENTITY_TYPE, USABLE_ITEM (NEEDS TO BE PICKABLE)
                row_repeat = col_repeat = 1