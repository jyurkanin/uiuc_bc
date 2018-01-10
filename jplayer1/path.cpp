#include "path.hpp"

#ifdef DEBUG
int testMap[MAX_MAP_SIZE_X][MAX_MAP_SIZE_X];
int pathTest(int argc, char *argv[]){
//    printf("ayy lmao Im startin\n");
    if(argc < 2) return EXIT_FAILURE;
    
    srand(0xDEADBEE);
    for(int i = 0; i < 50; i++){
        for(int j = 0; j < 50; j++){
            if(rand() < RAND_MAX/std::stof(argv[1], nullptr))
                testMap[i][j] = BLOCKED;
            else
                testMap[i][j] = PASSABLE;
        }
    }

    std::vector<bc_Direction> moves;
    return Path::getPath(moves, testMap, {0, 1}, {49, 49});
    
}

int debugMap(int (&directionMap)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y]){
    for(int i = 0; i < 50; i++){
        for(int j = 0; j < 50; j++){
            if(map[i][j] == BLOCKED)
                mvaddch(j, i, '#');
            else if(directionMap[i][j] >= 0)
                mvaddch(j, i, directionMap[i][j] + 48);
        }
    }
    return EXIT_SUCCESS;
}
#endif

int Path::getAllPaths(int (&directionMap)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], Pos2D start, Pos2D stop){
    std::vector<Pos2D> waveFront;
    std::vector<Pos2D> nextWave;
    Pos2D testPos;
    int counter = 0;
    int pathExists = 0;
    
    memset(directionMap, -1, sizeof(directionMap));
    
    waveFront.push_back(stop);
    directionMap[stop.x][stop.y] = counter;

    while(1){
        counter++;
        for(int k = 0; k < waveFront.size(); k++){
            for(int i = -1; i <= 1; i++){
                for(int j = -1; j <= 1; j++){
                    if(i == 0 && j == 0) continue; //pretty lazy.
                    testPos.x = waveFront[k].x + i;
                    testPos.y = waveFront[k].y + j;
                    if(testPos.x < 0 || testPos.x >= MAX_MAP_SIZE_X ||
                       testPos.y < 0 || testPos.y >= MAX_MAP_SIZE_Y) continue;
                        
                    
                    
                    if(map[testPos.x][testPos.y] == BLOCKED) continue;
                    if(directionMap[testPos.x][testPos.y] != -1) continue;
                    pathExists = 1;
                    directionMap[testPos.x][testPos.y] = counter;
                    if(testPos.x == start.x && testPos.y == start.y) return EXIT_SUCCESS;
                    nextWave.push_back(testPos);
                }
            }
        }
        if(!pathExists){
            return EXIT_FAILURE;
        }
        pathExists = 0;
        waveFront = nextWave;
        nextWave.clear();
        
#ifdef DEBUG
//        debugMap(directionMap, map);
//        refresh();
//        getch();
#endif
    }
    
    
    
    
}

int Path::getNextMove(int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], Pos2D start){
    
}

int Path::getPath(std::vector<bc_Direction> &moveList, int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], Pos2D start, Pos2D stop){
    static int directionMap[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y]; //will it be faster if this is static?
    return getAllPaths(directionMap, map, start, stop);
}

