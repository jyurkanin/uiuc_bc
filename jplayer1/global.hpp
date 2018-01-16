#include <sys/time.h>
#include "bc.hpp"

#pragma once

#define MAX_MAP_SIZE_X 50
#define MAX_MAP_SIZE_Y 50

#define PASSABLE 0x0
#define TERRAIN 0x1

//Params
#define NUM_WORKERS_GOAL 10




//aint got no gdb
#define WHEREAMI(x) printf("[%s:%d] %d\n", __FILE__, __LINE__, x); fflush(stdout)
#define STOPWATCH(x) gettimeofday(&_time_start, NULL); x; gettimeofday(&_time_stop, NULL); printf("%s : %ld, %ld\n", #x, _time_stop.tv_sec - _time_start.tv_sec, _time_stop.tv_usec - _time_start.tv_usec); fflush(stdout)

inline bool check_errors() {
    /// Check if we have an error...
    if (bc_has_err()) {
        char *err;
        /// Note: this clears the current global error.
        int8_t code = bc_get_last_err(&err);
        //printf("Engine error code %d: %s\n", code, err);
        bc_free_string(err);
        return true;
    } else {
        return false;
    }
}

extern struct timeval _time_start;
extern struct timeval _time_stop;

typedef enum{
    START
} Major_Mode;

typedef enum{
    
} Minor_Mode;

typedef enum{
    
} Interrupt_Mode;

struct{
    Major_Mode major_mode;
    Minor_Mode minor_mode;
    Interrupt_Mode interrupt;
} gs; //global state
