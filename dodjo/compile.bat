gcc -Wall -Wextra -Wpedantic -Wshadow -Wformat=0 -Wcast-align -Wnull-dereference -g3 -O0 main.c render.c inventory.c item.c player.c map.c hash.c dynarray.c -o .\build\Debug\outDebug.exe
.\build\Debug\outDebug.exe
sleep 50
@REM gcc -Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wcast-align -Wnull-dereference -g3 -O0 main.c render.c player.c map.c hash.c dynarray.c -o .\build\Debug\outDebug.exe