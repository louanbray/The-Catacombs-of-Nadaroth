from enum import IntEnum

class Sprite(IntEnum):
    WALL = 69
    STARGATE = 9055
    VGATE = 9608
    UGATE = 9600
    DGATE = 9604
    BRONZE = 66
    SILVER = 83
    GOLD = 71
    NADINO = 78


items = {"S":[Sprite.STARGATE],
         "T":[Sprite.WALL],
         "A":[Sprite.BRONZE],
         "B":[Sprite.SILVER],
         "C":[Sprite.GOLD],
         "D":[Sprite.NADINO],}

        #                                                               C
        #                                                               E
        #                                                               N
        #                                                               T
        #                                                               E
        #                                                               R
        #----------------------------------------------------------------ඞ----------------------------------------------------------------
specs = ["3"]
entity = ["S*S*S",
          "S*D*S",
          "S*S*S"]
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",#CENTER 
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " ",
        # " "]
        #----------------------------------------------------------------ඞ----------------------------------------------------------------
        #                                                               C
        #                                                               E
        #                                                               N
        #                                                               T
        #                                                               E
        #                                                               R

if __name__ == "__main__":
    print(f"*{",".join(specs)}*")
    for i in range(len(entity)):
        row_repeat = col_repeat = 1
        
        row = []
        row[:] = entity[i]
        for j in range(len(row)):
            if row[j] == '*' or row[j] == ' ':
                pass
            elif j < len(row)-1-1 and row[j] == row[j+1+1]:
                row_repeat += 1              
            else:
                if i < len(entity) - 1 and row_repeat == 1:
                    for k in range(len(entity) - 1 - i):
                        col = []
                        col[:] = entity[i+1+k]
                        if row[j] == col[j]:
                            col_repeat += 1
                            col[j] = ' '
                            entity[i+1+k] = "".join(col)
                        else:
                            break
                print(f"{j-len(entity[i])//2},{-i+len(entity)//2},{items[row[j]][0]},{row_repeat},{col_repeat}") # X, Y, TYPE, DISPLAY CHAR, ROW REPEAT, SPACE, COL REPEAT
                row_repeat = col_repeat = 1

