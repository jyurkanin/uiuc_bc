#include "path.hpp"
#define WHEREAMI(x) printf("[%s:%d] %d\n", __FILE__, __LINE__, x); fflush(stdout)

#ifdef DEBUG
int testMap[MAX_MAP_SIZE_X][MAX_MAP_SIZE_X];
int pathTest(int argc, char *argv[]){
//    printf("ayy lmao Im startin\n");
    if(argc < 2) return EXIT_FAILURE;
    
    srand(0xDEADBEEF);
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

void print_map(int (&directionMap)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int width, int height){
    printf("\n");
    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            if(map[x][y] & TERRAIN) printf("#");
            else if(directionMap[x][y] != -1) printf("%c", 65+directionMap[x][y]);      
            else printf(" ");
        }
        printf("\n");
    }
}

bc_GameController *Path::gc;
int Path::height;
int Path::width;

//directionMap is a map of distances from the stop point. map is a map of impassable terrain
int Path::getAllPaths(int (&directionMap)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], bc_MapLocation *ml_start, bc_MapLocation *ml_stop){
    std::vector<Pos2D> waveFront;
    std::vector<Pos2D> nextWave;
    Pos2D start;
    Pos2D stop;
    Pos2D testPos;
    int counter = 0;
    int pathExists = 0;
//    int debug_int = 0;
    bc_MapLocation *temp_ml = new_bc_MapLocation(Earth, 0, 0);
    
    start.x = bc_MapLocation_x_get(ml_start);
    start.y = bc_MapLocation_y_get(ml_start);
    stop.x = bc_MapLocation_x_get(ml_stop);
    stop.y = bc_MapLocation_y_get(ml_stop);
    
    memset(directionMap, -1, sizeof(directionMap));
    
    waveFront.push_back(stop);
    directionMap[stop.x][stop.y] = counter;

    int loopies = 0;
    while(1){
        counter++;
//        WHEREAMI(counter);
        for(int k = 0; k < waveFront.size(); k++){
            for(int i = -1; i <= 1; i++){
                for(int j = -1; j <= 1; j++){
                    loopies++;
                    //printf("Died: %d, (%d,%d)\n", debug_int, testPos.x, testPos.y);
                    if(i == 0 && j == 0) continue; //pretty lazy.
                    testPos.x = waveFront[k].x + i;
                    testPos.y = waveFront[k].y + j;
//                    debug_int = 1;
                    if(testPos.x < 0 || testPos.x >= width ||
                       testPos.y < 0 || testPos.y >= height) continue;
//                    debug_int = 2;
                    if(directionMap[testPos.x][testPos.y] != -1) continue;
//                    debug_int = 3;
                    if(map[testPos.x][testPos.y] & TERRAIN) continue;
//                    if((map[testPos.x][testPos.y] >> 1) < round){ //our info is out of date
                    //rescan if possible.
                    bc_MapLocation_x_set(temp_ml, testPos.x);
                    bc_MapLocation_y_set(temp_ml, testPos.y);
//                    debug_int = 4;
                    if(bc_GameController_can_sense_location(gc, temp_ml) && !bc_GameController_has_unit_at_location(gc, temp_ml)){
                        continue;
                    }
//                    debug_int = 5;
                    pathExists = 1;
                    directionMap[testPos.x][testPos.y] = counter;
                    if(testPos.x == start.x && testPos.y == start.y){
                        delete_bc_MapLocation(temp_ml);
                        printf("loopies %d", loopies);
                        return EXIT_SUCCESS;
                    }
                    nextWave.push_back(testPos);
                }
            }
        }
        if(!pathExists){
            delete_bc_MapLocation(temp_ml);
            printf("loopies %d", loopies);
            return EXIT_FAILURE;
        }
        pathExists = 0;
        waveFront = nextWave;
//        print_map(directionMap, map,  width, height);
        nextWave.clear();
        
#ifdef DEBUG
        debugMap(directionMap, map);
        refresh();
        getch();
#endif
    }
}




int Path::getAllPaths2(int (&directionMap)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], bc_MapLocation *ml_start, bc_MapLocation *ml_stop){
    static Pos2D waveFront[MAX_MAP_SIZE_X*MAX_MAP_SIZE_Y]; //maximum size this thing can be.
    static Pos2D nextWave[MAX_MAP_SIZE_X*MAX_MAP_SIZE_Y]; //maximum size this thing can be.
    int waveFrontLen = 0;
    int nextWaveLen = 0;
    Pos2D start;
    Pos2D stop;
    Pos2D testPos;
    int counter = 0;
    int pathExists = 0;
    int debug_int = 0;
    bc_Planet planet = bc_MapLocation_planet_get(ml_start);
    bc_MapLocation *temp_ml = new_bc_MapLocation(planet, 0, 0);
    
    start.x = bc_MapLocation_x_get(ml_start);
    start.y = bc_MapLocation_y_get(ml_start);
    stop.x = bc_MapLocation_x_get(ml_stop);
    stop.y = bc_MapLocation_y_get(ml_stop);
    
    memset(directionMap, -1, sizeof(directionMap));
    
    waveFront[0] = stop;
    waveFrontLen = 1;
    directionMap[stop.x][stop.y] = counter;

    while(1){
        counter++;
        for(int k = 0; k < waveFrontLen; k++){
            for(int i = -1; i <= 1; i++){
                for(int j = -1; j <= 1; j++){
//                    printf("Died: %d, (%d,%d)\n", debug_int, testPos.x, testPos.y);
                    if(i == 0 && j == 0) continue; //pretty lazy.
                    testPos.x = waveFront[k].x + i;
                    testPos.y = waveFront[k].y + j;
                    debug_int = 1;
                    if(testPos.x < 0 || testPos.x >= width ||
                       testPos.y < 0 || testPos.y >= height) continue;
                    debug_int = 2;
                    if(directionMap[testPos.x][testPos.y] != -1) continue;
                    debug_int = 3;
                    if(map[testPos.x][testPos.y] & TERRAIN) continue;
//                    if((map[testPos.x][testPos.y] >> 1) < round){ //our info is out of date
                    //rescan if possible.
                    bc_MapLocation_x_set(temp_ml, testPos.x);
                    bc_MapLocation_y_set(temp_ml, testPos.y);
                    debug_int = 4;
                    //has unit is bugged
                    if(testPos.x == start.x && testPos.y == start.y){
                        pathExists = 1;
                        directionMap[testPos.x][testPos.y] = counter;
                        delete_bc_MapLocation(temp_ml);
                        return EXIT_SUCCESS;
                    }
                    debug_int = 5;
                    if(bc_GameController_can_sense_location(gc, temp_ml) && !bc_GameController_is_occupiable(gc, temp_ml)){
                        continue;
                    }                    
                    pathExists = 1;
                    directionMap[testPos.x][testPos.y] = counter;
                    nextWave[nextWaveLen] = testPos;
                    nextWaveLen++;
                }
            }
        }
        if(!pathExists){
            delete_bc_MapLocation(temp_ml);
            return EXIT_FAILURE;
        }
        pathExists = 0;
        memcpy(waveFront, nextWave, sizeof(Pos2D) * nextWaveLen);
        waveFrontLen = nextWaveLen;
//        print_map(directionMap, map,  width, height);
        nextWaveLen = 0;
        
#ifdef DEBUG
        debugMap(directionMap, map);
        refresh();
        getch();
#endif
    }
}

Pos2D Path::getNextMove(int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], Pos2D point){
    Pos2D minPoint = {-1, -1};
    int min = 1000000;
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            if(i == 0 && j == 0) continue;
            if(point.x + i < 0 || point.x + i >= width ||
               point.y + j < 0 || point.y + j >= height) continue;
            if( map[point.x + i][point.y + j] >= 0 && map[point.x + i][point.y + j] < min){
                min = map[point.x + i][point.y + j];
                minPoint.x = point.x + i;
                minPoint.y = point.y + j;
            }
        }
    }
    if(min == 1000000){printf("Fucked up over here\n"); fflush(stdout);}
    return minPoint;
}

//get the direction from start to adjacent
bc_Direction Path::getDirection(Pos2D start, Pos2D adjacent){
    int dx = start.x - adjacent.x + 1; //either 0, 1, 2
    int dy = start.y - adjacent.y + 1;
    int combined = dx + (dy << 2);
    static bc_Direction table[11] = {Northeast, North, Northwest, Center, East, Center, West, Center, Southeast, South, Southwest};
    /* dx dy combined
       0 0 0
       0 1 4
       0 2 8
       1 0 1
       1 1 5
       1 2 9
       2 0 2
       2 1 6
       2 2 10 */
    return table[combined];    
}

int Path::getPath(std::vector<bc_Direction> &moveList, int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], bc_MapLocation *ml_start, bc_MapLocation *ml_stop){
    static int directionMap[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y]; //will it be faster if this is static?
//    printf("from %s, to %s\n", bc_MapLocation_debug(ml_start), bc_MapLocation_debug(ml_stop)); fflush(stdout);
    if(bc_MapLocation_eq(ml_start, ml_stop)) return EXIT_SUCCESS;
    if(getAllPaths2(directionMap, map, ml_start, ml_stop))
        return EXIT_FAILURE;
    
    Pos2D start;
    Pos2D stop;
    start.x = bc_MapLocation_x_get(ml_start);
    start.y = bc_MapLocation_y_get(ml_start);
    stop.x = bc_MapLocation_x_get(ml_stop);
    stop.y = bc_MapLocation_y_get(ml_stop);

    Pos2D temp;
    Pos2D temp2;
    temp.x = start.x;
    temp.y = start.y;
    //there has to be a path otherwise it would have returned earlier.
    moveList.clear();
    
    for(int i = 0; i < 50 && (temp.x != stop.x || temp.y != stop.y); i++){
        temp2 = getNextMove(directionMap, temp);
        moveList.push_back(getDirection(temp, temp2));
        temp = temp2;
    }
    return EXIT_SUCCESS;
}

