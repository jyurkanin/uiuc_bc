#include "path.hpp"
#include "params.hpp"
#include "stdlib.h"
#include "ncurses.h"
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


