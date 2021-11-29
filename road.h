#ifndef _road_h
#define _road_h
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "car.h"
#include "queue.h"

typedef struct RoadData
{
    /* TODO - Add data associated with road.  Suggested data: */
    /* length of this road */
    int roadLength;

    /* information used to record/update whether the light at the end of this road is green or red */
    int greenOn;
    int greenOff;
    int resetCycle;
    int currentCycle;
    /* intersections this road starts from and moves to */
    int from;
    int to;
    
    /* array of cars associated with this road */
    Car **carsRoad;
    /* queue of cars waiting to enter this road */
    Queue *waitingToEnter;
    int lastTime;
}  RoadData;

void printCar();

#endif

