#include <vector>
#include <stdio.h>
#include <string>
#include <string.h>
#include <cstdlib>
#include "bc.hpp"
#include "params.hpp"

#pragma once

#ifdef DEBUG

#include <ncurses.h>
int pathTest(int argc, char *argv[]);
#endif

#define PASSABLE 1
#define BLOCKED 0
#define TERRAIN -1

typedef struct{
    int x; int y;
} Pos2D;

class Path{
private:
    static bc_GameController *gc;
    static int height;
    static int width;
public:
    static void path_init(bc_GameController *g, int w, int h){gc = g; width = w; height = h;}
    static bc_Direction getDirection(Pos2D start, Pos2D adjacent);
    static Pos2D getNextMove(int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], Pos2D start);
    static int getAllPaths(int (&directionMap)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], bc_MapLocation *start, bc_MapLocation *stop);
    static int getPath(std::vector<bc_Direction> &moveList, int (&map)[MAX_MAP_SIZE_X][MAX_MAP_SIZE_Y], bc_MapLocation *start, bc_MapLocation *stop);
};
