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
    //nothing fancy. Place holder for actual strategy
    bc_GameController_queue_research(gc, Rocket);
    bc_GameController_queue_research(gc, Ranger);
    bc_GameController_queue_research(gc, Worker);
    bc_GameController_queue_research(gc, Ranger);
    bc_GameController_queue_research(gc, Worker);
    bc_GameController_queue_research(gc, Ranger);
    bc_GameController_queue_research(gc, Worker);
    
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



int canUnload(bc_Unit *unit, bc_Direction &dir){
    for(int i = 0; i < 8; i++){
        if(bc_GameController_can_unload(gc, bc_Unit_id(unit), (bc_Direction) i)){
            dir = (bc_Direction) i;
            return true;            
        }
    }
    return false;
}

int canLoad(bc_Unit *unit, int &id){
    bc_Location *unit_l = bc_Unit_location(unit);
    bc_MapLocation *unit_ml = bc_Location_map_location(unit_l);
    bc_VecUnit *adjacent = bc_GameController_sense_nearby_units_by_team(gc, unit_ml, 2, myTeam);
    bc_Unit *temp_unit;
    int len = bc_VecUnit_len(adjacent);
    
    delete_bc_MapLocation(unit_ml);
    delete_bc_Location(unit_l);
    
    for(int i = 0; i < len; i++){
        temp_unit = bc_VecUnit_index(adjacent, i);
        if(bc_GameController_can_load(gc, bc_Unit_id(unit), bc_Unit_id(temp_unit))){
            id = bc_Unit_id(temp_unit);
            delete_bc_Unit(temp_unit);
            delete_bc_VecUnit(adjacent);
            return true;
        }
        delete_bc_Unit(temp_unit);
         
    }
    delete_bc_VecUnit(adjacent);
    return false;
}

int canAttack(bc_Unit *unit, int &id){
    bc_Location *unit_l = bc_Unit_location(unit);
    bc_MapLocation *unit_ml = bc_Location_map_location(unit_l);
    bc_VecUnit *units = bc_GameController_sense_nearby_units_by_team(gc, unit_ml, bc_Unit_attack_range(unit), OTHER_TEAM(myTeam));
    bc_Unit *temp_unit;
    bc_Location *temp_l;
    bc_MapLocation *temp_ml;
    int len = bc_VecUnit_len(units);
    int min = 100000000;
    int min_id;
    int temp;
    int can = 0;
    
    for(int i = 0; i < len; i++){
        temp_unit = bc_VecUnit_index(units, i);
        temp_l = bc_Unit_location(temp_unit);
        temp_ml = bc_Location_map_location(temp_l);
        
        temp = bc_MapLocation_distance_squared_to(temp_ml, unit_ml);
        if(temp < min){
            min = temp;
            can = 1;
            min_id = bc_Unit_id(temp_unit);
        }
        
        delete_bc_Unit(temp_unit);
        delete_bc_Location(temp_l);
        delete_bc_MapLocation(temp_ml);
    }
    
    delete_bc_VecUnit(units);
    delete_bc_MapLocation(unit_ml);
    delete_bc_Location(unit_l);
    if(can) id = min_id;
    return can;
}

int findLandingSite(bc_MapLocation **ml){ //scans a mars' starting map to find a suitable spot to land. 
    bc_PlanetMap *planetMap = bc_GameController_starting_map(gc, Mars);
    bc_MapLocation *temp_ml = new_bc_MapLocation(Mars, 0, 0);
    int height = bc_PlanetMap_height_get(planetMap);
    int width = bc_PlanetMap_width_get(planetMap);
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            bc_MapLocation_x_set(temp_ml, x);
            bc_MapLocation_y_set(temp_ml, y);
            if(bc_PlanetMap_is_passable_terrain_at(planetMap, temp_ml)){
                *ml = temp_ml;
                delete_bc_PlanetMap(planetMap);
                return EXIT_SUCCESS;
            }
        }        
    }

    delete_bc_MapLocation(temp_ml);
    delete_bc_PlanetMap(planetMap);
    return EXIT_FAILURE;
}

//find an enemy that is too close to a factory
int getTarget(const std::vector<int> &factories, int &target){
    int min_distance = 1000000;
    bc_Unit *fact;
    bc_MapLocation *fact_ml;
    bc_Location *fact_l;
    bc_MapLocation *unit_ml;
    bc_Location *unit_l;
    int min_id;
    bc_Unit *unit;
    bc_VecUnit *units;
    int len;
    int temp;
    int found = 0;
    
    for(int i = 0; i < factories.size(); i++){
        fact = bc_GameController_unit(gc, factories[i]);
        fact_l = bc_Unit_location(fact);
        fact_ml = bc_Location_map_location(fact_l);
        units = bc_GameController_sense_nearby_units_by_team(gc, fact_ml, 16, OTHER_TEAM(myTeam));

        len = bc_VecUnit_len(units);        
        for(int i = 0; i < len; i++){
            unit = bc_VecUnit_index(units, i);
            unit_l = bc_Unit_location(unit);
            unit_ml = bc_Location_map_location(unit_l);

            temp = bc_MapLocation_distance_squared_to(unit_ml, fact_ml);
            if(temp < min_distance){
                min_distance = temp;
                min_id = bc_Unit_id(unit);
                found = 1;
            }

            delete_bc_Location(unit_l);
            delete_bc_MapLocation(unit_ml);
            delete_bc_Unit(unit);
        }
        
        delete_bc_Unit(fact);
        delete_bc_Location(fact_l);
        delete_bc_MapLocation(fact_ml);
        delete_bc_VecUnit(units);
    }
    
    if(found){
        target = min_id;
    }
    return !found;
}

int countMyRobotsOnMars(){
    bc_VecUnit *units = bc_GameController_units(gc);
    bc_Unit *unit;
    bc_Location *unit_l;
    int len = bc_VecUnit_len(units);
    int count = 0;
    for(int i = 0; i < len; i++){
        unit = bc_VecUnit_index(units, i);
        unit_l = bc_Unit_location(unit);
        if(bc_Unit_team(unit) == myTeam && bc_Location_is_on_planet(unit_l, Mars))
            count++;
        delete_bc_Unit(unit);
        delete_bc_Location(unit_l);
    }
    delete_bc_VecUnit(units);
    return count;
}

int countMyRobotsInSpace(){
    bc_VecUnit *units = bc_GameController_units_in_space(gc);
    int len = bc_VecUnit_len(units);
    delete_bc_VecUnit(units);
    return len;
}

int start_phase(){
    Team t = get_robots();
    bc_Unit *loading_rocket;
    bc_Unit *unit;
    bc_Unit *enemy;
    bc_Unit *temp_unit;
    bc_Location *unit_l;
    bc_MapLocation *unit_ml;
    bc_MapLocation *temp_ml;
    bc_Location *temp_l;
    std::vector<bc_Direction> moves;
    int id = 0;
    int hasRocketry = 0;
    int num_blocked_workers = 0;
    int num_blocked_rangers = 0;
    bc_Direction dir;//ection
    bc_VecUnitID *unit_ids;    
    bc_ResearchInfo *r_info = bc_GameController_research_info(gc); 

    hasRocketry = bc_ResearchInfo_get_level(r_info, Rocket);
    delete_bc_ResearchInfo(r_info);
    
    int karbonite = bc_GameController_karbonite(gc);
    int num_workers = t.workers.size(); //add the workers replicated this round
    int num_factories = t.factories.size();
    int num_rangers = t.rangers.size();
    int num_rockets = t.rockets.size();
    
    
    //worker logic
    //WHEREAMI(0);
    for(int i = 0; i < t.workers.size(); i++){
        unit = bc_GameController_unit(gc, t.workers[i]);
        unit_l = bc_Unit_location(unit);
        unit_ml = bc_Location_map_location(unit_l);

        //WHEREAMI(0);
        if(bc_Location_is_in_garrison(unit_l)){
            WHEREAMI(0);
        }
        else if(canBuild(unit, id)){ //if there is an adjacent blueprint, build it.
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
        else if(i < (4 + num_blocked_workers) && t.rockets.size() > 0){ //we have got a rocket that can be boarded. head over to it.
            loading_rocket = bc_GameController_unit(gc, t.rockets[0]); //should only be one rocket.
            temp_l = bc_Unit_location(loading_rocket);
            temp_ml = bc_Location_map_location(temp_l);
            
            if(Path::getPath(moves, terrain_map, unit_ml, temp_ml)) num_blocked_workers++;
            if ((moves.size() > 0) && bc_GameController_can_move(gc, t.workers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.workers[i])) {
                WHEREAMI(43);
                bc_GameController_move_robot(gc, t.workers[i], moves[0]);
                check_errors();
            }
            
            delete_bc_Location(temp_l);
            delete_bc_MapLocation(temp_ml);
            delete_bc_Unit(loading_rocket);            
        }
        else if(num_factories < NUM_FACTORIES_GOAL){
            if(karbonite < bc_UnitType_blueprint_cost(Factory)){ //if we don't have enough karbonite to build a factory then find more            
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
                    delete_bc_MapLocation(temp_ml);
                }
            }
            else{ //do have enough karbonite so try to build
                if(canBlueprint(unit, Factory, dir)){ //can Blueprint in any direction
                    WHEREAMI(51);
                    bc_GameController_blueprint(gc, bc_Unit_id(unit), Factory, dir);
                    num_factories++;
                }
                else{ //find a spot to blueprint
                    WHEREAMI(52);
                    temp_ml = findClosestSite(unit);
                    Path::getPath(moves, terrain_map, unit_ml, temp_ml);
                    if ((moves.size() > 0) && bc_GameController_can_move(gc, t.workers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.workers[i])) {
                        bc_GameController_move_robot(gc, t.workers[i], moves[0]);
                        check_errors();
                    }
                    delete_bc_MapLocation(temp_ml);
                }
            }
        } //if we got no one on mars and we have no rockets that are loading.
        else if(hasRocketry && (countMyRobotsInSpace() + countMyRobotsOnMars()) < 1 && num_rockets == 0){ //put a bitch on mars.
            if(canBlueprint(unit, Rocket, dir)){ //can Blueprint in any direction
                WHEREAMI(71);
                bc_GameController_blueprint(gc, bc_Unit_id(unit), Rocket, dir);
                num_rockets++;
            }
            else{ //find a spot to blueprint
                WHEREAMI(72);
                temp_ml = findClosestSite(unit);
                Path::getPath(moves, terrain_map, unit_ml, temp_ml);
                if ((moves.size() > 0) && bc_GameController_can_move(gc, t.workers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.workers[i])) {
                    bc_GameController_move_robot(gc, t.workers[i], moves[0]);
                    check_errors();
                }
                delete_bc_MapLocation(temp_ml);
            }
        }
        else{
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
                delete_bc_MapLocation(temp_ml);
            }
        }
    
        delete_bc_Unit(unit);
        delete_bc_Location(unit_l);
        delete_bc_MapLocation(unit_ml);
    }

    //rangers. Seek and destroy. As a group
    static int target_id = 0; //todo makes every unit their own class. Make this a class variable
    static int has_target = 0;
    if(has_target && !bc_GameController_can_sense_unit(gc, target_id)){ //how to check if enemy unit is dead?
        has_target = 0;
    }
    if(!has_target){
        getTarget(t.factories, target_id); //defend factories
    }
    for(int i = 0; i < t.rangers.size(); i++){
        unit = bc_GameController_unit(gc, t.rangers[i]);
        unit_l = bc_Unit_location(unit);
        unit_ml = bc_Location_map_location(unit_l);
        
        //WHEREAMI(0);
        if(bc_Location_is_in_garrison(unit_l)){
            WHEREAMI(0);
        }
        else if(i < (4 + num_blocked_rangers) && t.rockets.size() > 0){ //we have got a rocket that can be boarded. head over to it.
            loading_rocket = bc_GameController_unit(gc, t.rockets[0]); //should only be one rocket.
            temp_l = bc_Unit_location(loading_rocket);
            temp_ml = bc_Location_map_location(temp_l);
            
            if(Path::getPath(moves, terrain_map, unit_ml, temp_ml)) num_blocked_rangers++;
            if ((moves.size() > 0) && bc_GameController_can_move(gc, t.rangers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.rangers[i])) {
                WHEREAMI(43);
                bc_GameController_move_robot(gc, t.rangers[i], moves[0]);
                check_errors();
            }
            
            delete_bc_Location(temp_l);
            delete_bc_MapLocation(temp_ml);
            delete_bc_Unit(loading_rocket);            
        }
        else if(has_target && bc_GameController_can_attack(gc, t.rangers[i], target_id) && bc_GameController_is_attack_ready(gc, t.rangers[i])){
            bc_GameController_attack(gc, t.rangers[i], target_id);
        }
        else if(canAttack(unit, id) && bc_GameController_is_attack_ready(gc, t.rangers[i])){
            bc_GameController_attack(gc, t.rangers[i], id);
        }
        else if(has_target){ //seek
            enemy = bc_GameController_unit(gc, target_id);
            temp_l = bc_Unit_location(enemy);
            temp_ml = getClosestKarbonite(unit_ml);
            Path::getPath(moves, terrain_map, unit_ml, temp_ml);
            if ((moves.size() > 0) && bc_GameController_can_move(gc, t.rangers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.rangers[i])) {
                WHEREAMI(43);
                bc_GameController_move_robot(gc, t.rangers[i], moves[0]);
                check_errors();
            }
            delete_bc_Unit(enemy);
            delete_bc_Location(temp_l);
            delete_bc_MapLocation(temp_ml);
        }
        else if(t.factories.size() > 0){ //fortrify a position
            temp_unit = bc_GameController_unit(gc, t.factories[0]);
            temp_l = bc_Unit_location(temp_unit);
            temp_ml = getClosestKarbonite(unit_ml);
            Path::getPath(moves, terrain_map, unit_ml, temp_ml);
            if ((moves.size() > 0) && bc_GameController_can_move(gc, t.rangers[i], moves[0]) && bc_GameController_is_move_ready(gc, t.rangers[i])) {
                WHEREAMI(43);
                bc_GameController_move_robot(gc, t.rangers[i], moves[0]);
                check_errors();
            }
            delete_bc_Unit(temp_unit);
            delete_bc_Location(temp_l);
            delete_bc_MapLocation(temp_ml);
        }
    }
    
    
    
    //factories. Dont do much right now.
    for(unsigned int i = 0; i < t.factories.size(); i++){
        unit = bc_GameController_unit(gc, t.factories[i]);
        if(!bc_Unit_structure_is_built(unit)){
            delete_bc_Unit(unit);
            continue;
        }
        //try to unload units in garrison

//        unit_ids = bc_Unit_structure_garrison(unit);
//        len = bc_VecUnitID_len(unit_ids);
        while(canUnload(unit, dir)){ //indiscriminately unload all units
            bc_GameController_can_unload(gc, bc_Unit_id(unit), dir);
        }
        
        if(num_rangers < NUM_RANGERS_GOAL && bc_GameController_can_produce_robot(gc, t.factories[i], Ranger)){
            bc_GameController_produce_robot(gc, t.factories[i], Ranger);
            num_rangers++;
        }

        delete_bc_Unit(unit);
//        delete_bc_VecUnit(units);
    }

    //rockets. Launch when full.
    for(int i = 0; i < t.rockets.size(); i++){
        unit = bc_GameController_unit(gc, t.rockets[i]);
        if(!bc_Unit_structure_is_built(unit)){
            delete_bc_Unit(unit);
            continue;
        }

        unit_ids = bc_Unit_structure_garrison(unit);
        if(bc_VecUnitID_len(unit_ids) == bc_Unit_structure_max_capacity(unit)){ //if ready to blast the fuck off
            findLandingSite(&temp_ml);
            if(bc_GameController_can_launch_rocket(gc, t.rockets[i], temp_ml)){
                bc_GameController_launch_rocket(gc, t.rockets[i], temp_ml);
            }
        }
        else{
            while(canLoad(unit, id)){ //load all adjacent robots
                bc_GameController_load(gc, bc_Unit_id(unit), id);
            }
        }
        
        delete_bc_Unit(unit);
        delete_bc_VecUnitID(unit_ids);
    }
    
    
    return EXIT_SUCCESS;
}
