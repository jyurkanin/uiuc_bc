#include "path.hpp"
#include "global.hpp"
#include "stdlib.h"
#include <string>

//fun variables I want all of main to know about
int terrain_map[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y];
int map_width;
int map_height;
bc_GameController *gc;
bc_Planet planet;
bc_Team myTeam;
std::vector<bc_MapLocation*> initial_karbonite;


typedef struct{
    std::vector<int> workers;
    std::vector<int> knights;
    std::vector<int> rangers;
    std::vector<int> mages;
    std::vector<int> healers;
    std::vector<int> factories;
    std::vector<int> rockets;
} Team;

int robot_init();
int start_phase();
Team get_robots(bc_VecUnit *units); //by type

bc_MapLocation* getClosestKarbonite(bc_MapLocation *ml);
int canReplicate(bc_Unit *unit, bc_Direction &d);
int canRepair(bc_Unit *unit,  int &id);
int canBuild(bc_Unit *unit,  int &id);
bc_MapLocation *findClosestSite(bc_Unit *unit);
int canBlueprint(bc_Unit *unit, bc_UnitType unit_type, bc_Direction &dir);




int main(){
    //this will be used for pathfinding. It will hold information about where you can move.
    std::vector<bc_Direction> moveList;
    bc_MapLocation *target = new_bc_MapLocation(Earth, map_width/2, map_height/2);
    uint32_t round;
    bc_VecUnit *units;
    bc_Direction direction;
    bc_Location *unit_loc;
    bc_MapLocation *unit_ml;
    int numUnits;
    
    robot_init();

    
    while(bc_GameController_round(gc) < 1000){
        //check for interrupt conditions.
        //implement strategy.
        
        switch(gs.major_mode){
        case START:
            //WHEREAMI(bc_GameController_round(gc));
            start_phase();            
            break;
        }
        
        //WHEREAMI(0);
        bc_GameController_next_turn(gc);
        //WHEREAMI(0);
    }
}


//this just figures some basic shit out about the map
int robot_init(){
    printf("jplayer startin up\n");
    srand(0xDEADBEEF);
    gc = new_bc_GameController();

    //all this to find out what planet im on.
    bc_VecUnit *units = bc_GameController_my_units(gc);
    bc_Unit *unit = bc_VecUnit_index(units, 0);
    bc_Location *loc = bc_Unit_location(unit);
    bc_MapLocation *ml = bc_Location_map_location(loc);
    planet = bc_MapLocation_planet_get(ml);
    myTeam = bc_Unit_team(unit);
    delete_bc_Unit(unit);
    delete_bc_VecUnit(units);
    delete_bc_Location(loc);
    delete_bc_MapLocation(ml);

    if(planet == Mars){
        while(bc_GameController_round(gc) < 1000) bc_GameController_next_turn(gc);
    }
    bc_PlanetMap *planetMap = bc_GameController_starting_map(gc, planet);
    
    map_height = bc_PlanetMap_height_get(planetMap);
    map_width = bc_PlanetMap_width_get(planetMap);
    
    Path::path_init(gc, map_width, map_height);
    
    bc_MapLocation *temp_ml = new_bc_MapLocation(planet, 0, 0);
    for(int y = 0; y < map_height; y++){
        for(int x = 0; x < map_width; x++){
            bc_MapLocation_x_set(temp_ml, x);
            bc_MapLocation_y_set(temp_ml, y);

            if(bc_PlanetMap_initial_karbonite_at(planetMap, temp_ml)){
                initial_karbonite.push_back(new_bc_MapLocation(planet, x, y));
                printf(" ");
            }
            else if(bc_PlanetMap_is_passable_terrain_at(planetMap, temp_ml)){
                terrain_map[x][y] = PASSABLE;
                printf(" ");
            }
            else{
                printf("0");
                terrain_map[x][y] = TERRAIN;
            }
        }
        printf("\n");
    }
//    print_map(terrain_map, map_width, map_height);
    //WHEREAMI(0);
    delete_bc_MapLocation(temp_ml);  
    delete_bc_PlanetMap(planetMap);
    return EXIT_SUCCESS;
}

Team get_robots(){
    bc_VecUnit *units = bc_GameController_my_units(gc);
    Team t;
    int len = bc_VecUnit_len(units);
    bc_Unit *unit;
    int type;
    uint16_t id;
    for(int i = 0; i < len; i++){
        unit = bc_VecUnit_index(units, i);
        type = bc_Unit_unit_type(unit);
        id = bc_Unit_id(unit);
        switch(type){
        case Worker:
            t.workers.push_back(id);
            break;
        case Knight:
            t.knights.push_back(id);
            break;
        case Ranger:
            t.rangers.push_back(id);
            break;
        case Mage:
            t.mages.push_back(id);
            break;
        case Healer:
            t.healers.push_back(id);
            break;
        case Factory:
            t.factories.push_back(id);
            break;
        case Rocket:
            t.rockets.push_back(id);
            break;
        }
    }
    return t;
}

bc_MapLocation* getClosestKarbonite(bc_MapLocation *ml){
    bc_MapLocation *min_ml = initial_karbonite[0];
    int min_dist = bc_MapLocation_distance_squared_to(min_ml, ml);
    int temp;
    for(int i = 1; i < initial_karbonite.size(); i++){
        temp = bc_MapLocation_distance_squared_to(initial_karbonite[i], ml);
        if(temp < min_dist){
            min_dist = temp;
            min_ml = initial_karbonite[i];
        }
    }
    return min_ml;
}

int canBlueprint(bc_Unit *unit, bc_UnitType unit_type, bc_Direction &dir){
    //for fuck sake
    bc_Location *unit_l = bc_Unit_location(unit);
    bc_MapLocation *unit_ml = bc_Location_map_location(unit_l);
    bc_MapLocation *temp_ml;
    int worker_id = bc_Unit_id(unit);
    delete_bc_Location(unit_l);
    
    for(int i = 0; i < 8; i++){
        temp_ml = bc_MapLocation_add(unit_ml, (bc_Direction) i);
        if(!bc_GameController_karbonite_at(gc, temp_ml) && bc_GameController_can_blueprint(gc, worker_id, unit_type, (bc_Direction) i)){
            dir = (bc_Direction) i;
            delete_bc_MapLocation(temp_ml);
            delete_bc_MapLocation(unit_ml);
            return true;
        }
        delete_bc_MapLocation(temp_ml);
    }
    delete_bc_MapLocation(unit_ml);
    return false;
}

//finds the nearest place that a factory can be built.
bc_MapLocation *findClosestSite(bc_Unit *unit){
    //WHEREAMI(0);
    int viewRadius = bc_Unit_vision_range(unit);
    bc_Location *temp_l = bc_Unit_location(unit);
    bc_MapLocation *unit_ml = bc_Location_map_location(temp_l);
    bc_VecMapLocation *viewableLocations = bc_GameController_all_locations_within(gc,unit_ml, viewRadius);
    int len = bc_VecMapLocation_len(viewableLocations);    

    delete_bc_Location(temp_l);

    int minRadius = 10000000;
    int temp;
    bc_MapLocation *closestSite = 0;
    bc_MapLocation *temp_ml;

    for(int i = 0; i < len; i++){
        temp_ml = bc_VecMapLocation_index(viewableLocations, i);
        temp = bc_MapLocation_distance_squared_to(temp_ml, unit_ml);
        if(temp < minRadius && bc_GameController_is_occupiable(gc, temp_ml) && !bc_GameController_karbonite_at(gc, temp_ml)){
            minRadius = temp;
            closestSite = temp_ml;
        }
        else delete_bc_MapLocation(temp_ml);
    }
    
    delete_bc_VecMapLocation(viewableLocations);
    delete_bc_MapLocation(unit_ml);
    
//    printf("Closest site: %s\n", bc_MapLocation_debug(closestSite));
    return closestSite;
}

//robot will go in this general direction
bc_Direction walk(){
    
}

int canBuild(bc_Unit *unit,  int &id){
    //WHEREAMI(0);
    bc_Unit *temp_unit;
    bc_Location *unit_l = bc_Unit_location(unit);
    bc_MapLocation *unit_ml = bc_Location_map_location(unit_l);
    bc_Location *temp_l;
    bc_MapLocation *temp_ml;

    bc_VecUnit *units = bc_GameController_sense_nearby_units_by_type(gc, unit_ml, 2, Factory);
    int units_len = bc_VecUnit_len(units);
    
    for(int i = 0; i < units_len; i++){
        temp_unit = bc_VecUnit_index(units, i);
        temp_l = bc_Unit_location(temp_unit);
        temp_ml = bc_Location_map_location(temp_l);
        if(!bc_Unit_structure_is_built(temp_unit) && bc_Unit_team(temp_unit) == myTeam){
            id = bc_Unit_id(temp_unit);
            
            delete_bc_Unit(temp_unit);
            delete_bc_Location(temp_l);
            delete_bc_MapLocation(temp_ml);
            delete_bc_VecUnit(units); //this is just awful.
            delete_bc_Location(unit_l);
            delete_bc_MapLocation(unit_ml);         
            return bc_GameController_can_build(gc, bc_Unit_id(unit), id);
        }
        delete_bc_Unit(temp_unit);
        delete_bc_Location(temp_l);
        delete_bc_MapLocation(temp_ml);
    }
    delete_bc_VecUnit(units);
    delete_bc_Location(unit_l);
    delete_bc_MapLocation(unit_ml);
    return false;
}

//get the minimum health adjacent factory
int canRepair(bc_Unit *unit,  int &id){
    //WHEREAMI(0);
    bc_Unit *temp_unit;
    bc_Location *unit_l = bc_Unit_location(unit);
    bc_MapLocation *unit_ml = bc_Location_map_location(unit_l);
    
    bc_VecUnit *units = bc_GameController_sense_nearby_units_by_type(gc, unit_ml, 2, Factory);
    int units_len = bc_VecUnit_len(units);
    int min_id;
    int min_health = 1000000000;
    int temp;
    
    for(int i = 0; i < units_len; i++){
        temp_unit = bc_VecUnit_index(units, i);
        temp = bc_Unit_health(temp_unit);
        if(temp < min_health){
            min_health = temp;
            min_id = bc_Unit_id(temp_unit);
        }
        delete_bc_Unit(temp_unit);
    }
    if(min_health != 1000000000){
        id = min_id;
    }
    delete_bc_VecUnit(units);
    delete_bc_Location(unit_l);
    delete_bc_MapLocation(unit_ml);
    return (min_health < 300) && bc_GameController_can_repair(gc, bc_Unit_id(unit), id);
}

int canReplicate(bc_Unit *unit, bc_Direction &d){
    //WHEREAMI(0);
    for(int i = 0; i < 8; i++){
        if(bc_GameController_can_replicate(gc, bc_Unit_id(unit), (bc_Direction) i)){
            d = (bc_Direction) i;
            return true;            
        }
    }
    return false;
}

int canHarvest(bc_Unit *unit, bc_Direction &d){
    //WHEREAMI(0);
    for(int i = 0; i < 8; i++){
        if(bc_GameController_can_harvest(gc, bc_Unit_id(unit), (bc_Direction) i)){
            d = (bc_Direction) i;
            return true;            
        }
    }
    return false;
}



int start_phase(){
    Team t = get_robots();
    bc_Unit *unit;
    bc_Location *unit_l;
    bc_MapLocation *unit_ml;
    bc_MapLocation *temp_ml;
    bc_Location *temp_l;
    std::vector<bc_Direction> moves;
    int flag;
    int id;
    bc_Direction dir;//ection
    
    int karbonite = bc_GameController_karbonite(gc);
    int num_workers = t.workers.size(); //add the workers replicated this round
    int num_factories = t.factories.size();
    
    //worker logic
    //WHEREAMI(0);
    for(int i = 0; i < t.workers.size(); i++){
        unit = bc_GameController_unit(gc, t.workers[i]);
        unit_l = bc_Unit_location(unit);
        unit_ml = bc_Location_map_location(unit_l);

        //WHEREAMI(0);
        if(canBuild(unit, id)){ //if there is an adjacent blueprint, build it.
            WHEREAMI(1);
            bc_GameController_build(gc, t.workers[i], id);
        }
        else if(canRepair(unit, id)){ //if there is an adjacent factory that can be repaired, do it.
            WHEREAMI(2);
            bc_GameController_repair(gc, t.workers[i], id);
        }
        else if(num_workers < NUM_WORKERS_GOAL && canReplicate(unit, dir)){
            WHEREAMI(3);
            bc_GameController_replicate(gc, t.workers[i], dir);
            num_workers++;
        }
        else if(karbonite < bc_UnitType_blueprint_cost(Factory)){ //if we don't have enough karbonite to build a factory then find more            
            if(canHarvest(unit, dir)){
                WHEREAMI(41);
                bc_GameController_harvest(gc, t.workers[i], dir);
            }
            else{
                WHEREAMI(42);
                temp_ml = getClosestKarbonite(unit_ml);
                Path::getPath(moves, terrain_map, unit_ml, temp_ml);
                if ((moves.size() > 0) && bc_GameController_can_move(gc, t.workers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.workers[i])) {
                    WHEREAMI(43);
                    bc_GameController_move_robot(gc, t.workers[i], moves[0]);
                    check_errors();
                }
            }
        }
        else if(canBlueprint(unit, Factory, dir)){ //can Blueprint in any direction
            WHEREAMI(5);
            bc_GameController_blueprint(gc, bc_Unit_id(unit), Factory, dir);
            num_factories++;
        }
        else{ //find a spot to blueprint
            WHEREAMI(6);
            temp_ml = findClosestSite(unit);
            Path::getPath(moves, terrain_map, unit_ml, temp_ml);
            if ((moves.size() > 0) && bc_GameController_can_move(gc, t.workers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.workers[i])) {
                bc_GameController_move_robot(gc, t.workers[i], moves[0]);
                check_errors();
            }
        }

        delete_bc_Unit(unit);
        delete_bc_Location(unit_l);
        delete_bc_MapLocation(unit_ml);
    }
    
    //factories
    num_factories = 0;
    for(unsigned int i = 0; i < t.factories.size(); i++){
        unit = bc_GameController_unit(gc, t.factories[i]);
        num_factories += bc_Unit_structure_is_built(unit);
    }
    WHEREAMI(num_factories);
    return EXIT_SUCCESS;
}
