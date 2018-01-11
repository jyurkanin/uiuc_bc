#include "path.hpp"
#include "params.hpp"
#include "stdlib.h"
#include <string>

/*
 * time ./bot 2
 * count 0
 * real	0m0.283s
 * user	0m0.280s
 * sys	0m0.000s
 * 
 * so pathing takes like .283 milliseconds. WHen compiled with -O3
 *
int main(int argc, char *argv[]){
    int count = 0;
    initscr();
    noecho();
    for(int i = 0; i < 1000; i++)
        count += pathTest(argc, argv);
    printf("count %d\n", count);
    endwin();
}*/



int main(){
    //this will be used for pathfinding. It will hold information about where you can move.
    int pathmap[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y];
    int map_width;
    int map_height;
    
    printf("jplayer startin up\n");
    srand(0xDEADBEEF);
    bc_GameController *gc = new_bc_GameController();
    bc_PlanetMap *planetMap = bc_GameController_starting_map(gc, Earth);
            
    map_height = bc_PlanetMap_height_get(planetMap);
    map_width = bc_PlanetMap_width_get(planetMap);

    bc_MapLocation *temp_ml = new_bc_MapLocation(Earth, 0, 0);
    for(int x = 0; x < map_width; x++){
        for(int y = 0; y < map_height; y++){
            bc_MapLocation_x_set(temp_ml, x);
            bc_MapLocation_y_set(temp_ml, y);
            if(bc_PlanetMap_is_passable_terrain_at(planetMap, temp_ml)){
                pathmap[x][y] = PASSABLE;
            }
            else pathmap[x][y] = TERRAIN;
                
        }
    }
    delete_bc_MapLocation(temp_ml);
    
    delete_bc_PlanetMap(planetMap);
    
    while(true){
        uint32_t round = bc_GameController_round(gc);
        planetMap = new_bc_PlanetMap();
        bc_VecUnit *units = bc_GameController_my_units(gc);
        int numUnits = bc_VecUnit_len(units);
        for(int i = 0; i < numUnits; i++){
            bc_Unit  *unit = bc_VecUnit_index(units, i);
            uint16_t id = bc_Unit_id(unit);
            
            delete_bc_Unit(unit);
        }
        delete_bc_PlanetMap(planetMap);
        delete_bc_VecUnit(units);
        bc_GameController_next_turn(gc);
    }
}
