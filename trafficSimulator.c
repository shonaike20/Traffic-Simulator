#include "trafficSimulator.h"

/* printNames
 * input: none
 * output: none
 *
 * Prints names of the students who worked on this solution
 */
void printNames( )
{
    /* TODO : Fill in you and your partner's names (or N/A if you worked individually) */
    printf("This solution was completed by:\n");
    printf("Garreth Shonaike \n");
    printf("N/A>\n");
}

/* trafficSimulator
 * input: char* name of file to read
 * output: N/A
 *
 * Simulate the road network in the given file
 */
void trafficSimulator( char* filename )
{
    /* Read in the traffic data.  You may want to create a separate function to do this. */
    int i;
    int o = 10;
    int eventCycle;
    TrafficSimData *simData = readTrafficData(filename);
    simData -> maxSteps = 0;
    int *commandStepNum = simData -> printComSteps;
    
    double trackAverage = 0.0;
    /* Loop until all events processed and either all cars reached destination or gridlock has occurred */
    for(eventCycle =0;; eventCycle++){
    
        /* Loop on events associated with this time step */
        while(!isEmptyPQ(simData->events) && eventCycle == getFrontPriority(simData -> events)) {
            //from - to
            Event * event = dequeuePQ(simData -> events);
            RoadData * roadData = getEdgeData(simData -> trafficGraph, event -> start, event -> end);
            Car * car = malloc(sizeof(Car));
            car -> start = event->start;
            car -> end = event->end;
            car -> destinationVertex = event -> destinationVertex;
            car -> cycleCarJoined = eventCycle;
            enqueue(roadData->waitingToEnter, car);
            
            if(roadData->lastTime != eventCycle) {
                printf("CYCLE %d - ADD_CAR_EVENT - Cars enqueued on road from %d to %d\n", eventCycle, roadData -> from, roadData->to);
                roadData->lastTime = eventCycle;
            }
        }
        int j;

        //print evnts
        if(eventCycle == *commandStepNum) {
            commandStepNum++;
            printf("\nCYCLE %d - PRINT_ROADS_EVENT - Current contents of the roads:\n", eventCycle); 
            for(i = 0; i < simData -> numRoads; i++) {
                RoadData * currentRoad = simData -> roads[i];
                printf("Cars on the road from %d to %d:\n", currentRoad->from, currentRoad->to);
                for(j = 0; j < currentRoad -> roadLength; j++) {
                    if(currentRoad -> carsRoad[j] == NULL) {
                        printf("- ");
                    }
                    else {
                        printf("%d ", currentRoad -> carsRoad[j]->destinationVertex);
                    }
                }
                printf("\n");
            }
            printf("\n");
        }

        /* First try to move through the intersection */
        bool carReachedDest = false;
        for(i = 0; i < simData -> numRoads; i++) {
            RoadData * currentRoad = simData -> roads[i];
            Car * carToMove = currentRoad -> carsRoad[0];

            if(carToMove == NULL)
                continue;

            
            if(currentRoad->currentCycle >= currentRoad->greenOn && currentRoad->currentCycle < currentRoad->greenOff) {
                // shortest route/destination
                if(currentRoad->to == carToMove->destinationVertex) {
                    currentRoad -> carsRoad[0] = NULL;
                    int moved = eventCycle - carToMove->cycleCarJoined;
                    // arrived at destination
                    printf("CYCLE %d - Car successfully traveled from %d to %d in %d time steps.\n", eventCycle, carToMove->start, carToMove->destinationVertex, moved);

                    simData -> averageCount += (double)moved;
                    trackAverage++;
                    if(moved > simData -> maxSteps)
                        simData -> maxSteps = moved;
                    simData -> carsActive--;
                    carReachedDest = true;
                }
                else {
                    int * pNext = malloc(sizeof(int));
                    if(!getNextOnShortestPath(simData -> trafficGraph, currentRoad->to, carToMove->destinationVertex, pNext)) {
                        break;
                    }

                    RoadData * nextRoad = getEdgeData(simData -> trafficGraph, currentRoad->to, *pNext);

                    if(nextRoad -> carsRoad[nextRoad-> roadLength - 1] == NULL) {
                        carToMove->hasMoved = true;
                        nextRoad -> carsRoad[nextRoad-> roadLength - 1] = carToMove;
                        currentRoad -> carsRoad[0] = NULL;
                    }
                }
            }
        }
        
        /* Second move cars forward on every road */
        for(i = 0; i < simData -> numRoads; i++) {
            RoadData * currentRoad = simData -> roads[i];
            Car ** carsRoad = currentRoad -> carsRoad;

            for(j = currentRoad -> roadLength - 1; j > 0; j--) {
                if(carsRoad[j] != NULL && carsRoad[j - 1] == NULL) {
                    carsRoad[j - 1] = carsRoad[j];
                    carsRoad[j] = NULL;
                    carsRoad[j - 1]->hasMoved = true;
                    j--;
                }
            }
        }

        /* Third move cars onto road if possible */
        for(i = 0; i < simData -> numRoads; i++) {
            RoadData * currentRoad = simData -> roads[i];
            Car ** cars = currentRoad -> carsRoad;
            int roadLength = currentRoad -> roadLength;

            if(cars[roadLength - 1] == NULL && !isEmpty(currentRoad->waitingToEnter)) {
                // move waiting car onto road
                Car * car = dequeue(currentRoad->waitingToEnter);
                currentRoad -> carsRoad[roadLength-1] = car;
                car->hasMoved = true;
            }
        }

        // light cycles
        for(i = 0; i < simData -> numRoads; i++) {
            RoadData * currentRoad = simData -> roads[i];

            currentRoad->currentCycle++;
            
            if(currentRoad->currentCycle == currentRoad->resetCycle)
                currentRoad->currentCycle = 0;
        }

        //Check for Zero
        if(simData -> carsActive <= 0) {
            printf("\nAverage number of time steps to the reach their destination is %.2lf.\n", simData -> averageCount /= trackAverage);
            printf("Maximum number of time steps to the reach their destination is %d.\n", simData -> maxSteps);
            printf("\n");

            break;
        }

        // Grid Lock
        int cycledDetectedOn;
        if(eventCycle % simData -> longestLightCycles == 0) {
            bool haveAnyCarsMoved = carReachedDest;

            for(i = 0; i < simData -> numRoads; i++) {
                RoadData * currentRoad = simData -> roads[i];
                Car ** car = currentRoad -> carsRoad;

                for(j = 0; j < currentRoad-> roadLength; j++) {
                    if(car[j] == NULL)
                        continue;
                    haveAnyCarsMoved = haveAnyCarsMoved || car[j]->hasMoved;
                    // Grid Detection
                    if(haveAnyCarsMoved == true)
                        cycledDetectedOn = eventCycle;
                    car[j]->hasMoved = false;
                }
            }
            if(haveAnyCarsMoved == false) {
                printf("CYCLE %d - Gridlock detected.\n", cycledDetectedOn + 1);
                break;
            }
        }
    }

   
}

TrafficSimData * readTrafficData(char * filename) {
    TrafficSimData * simData = malloc(sizeof(TrafficSimData));

    FILE * fp;
    fp = fopen(filename, "r");

    int i;
    int j;
    int k;
    int verticies, edges;
    fscanf(fp, "%d", &verticies);
    fscanf(fp, "%d", &edges);
    simData -> trafficGraph = createGraph(verticies);
    simData -> events = createPQ();
    simData -> roads = malloc(sizeof(RoadData) * edges);
    simData -> numRoads = edges;
    int roadCounter = 0;

    for(i = 0; i < verticies; i++) {
        int toVertex, incomingRoads;
        fscanf(fp, "%d", &toVertex);
        fscanf(fp, "%d", &incomingRoads);

        if(!isVertex(simData -> trafficGraph, toVertex))
            addVertex(simData -> trafficGraph, toVertex);

        for(j = 0; j < incomingRoads; j++) {
            int fromVertex, roadLength, greenOn, greenOff, resetCycle;
            // scan in fromVertex & lengthOfEdge
            fscanf(fp, "%d", &fromVertex);
            fscanf(fp, "%d", &roadLength);
            // Green On
            fscanf(fp, "%d", &greenOn);
            // Green Off
            fscanf(fp, "%d", &greenOff);
            // Reset
            fscanf(fp, "%d", &resetCycle);
            RoadData *roadDataStruct = malloc(sizeof(RoadData));
            roadDataStruct -> to = toVertex;
            roadDataStruct -> from = fromVertex;
            roadDataStruct -> roadLength = roadLength;
            roadDataStruct -> greenOn = greenOn;
            roadDataStruct -> greenOff = greenOff;
            roadDataStruct -> resetCycle = resetCycle;
            roadDataStruct -> lastTime = -1;
            roadDataStruct -> carsRoad = malloc(sizeof(Car) * roadLength);
            for(k = 0; k < roadLength; ++k)
                roadDataStruct -> carsRoad[k] = NULL;
            roadDataStruct->waitingToEnter = createQueue();
            setEdge(simData -> trafficGraph, fromVertex, toVertex, roadLength);
            setEdgeData(simData -> trafficGraph, fromVertex, toVertex, roadDataStruct);
            simData -> roads[roadCounter++] = roadDataStruct;
            if(simData -> longestLightCycles < resetCycle)
               simData -> longestLightCycles = resetCycle;
        }
    }

    int addCarCommands;
    fscanf(fp, "%d", &addCarCommands);
    for(i = 0; i < addCarCommands; i++) {
        int start, end, whenAddCar, howManyToAdd;  
        fscanf(fp, "%d", &start);
        fscanf(fp, "%d", &end);
        fscanf(fp, "%d", &whenAddCar);
        fscanf(fp, "%d", &howManyToAdd);

        for(j = 0; j < howManyToAdd; j++) {
            int destinationVertex;
            fscanf(fp, "%d", &destinationVertex);

            Event * event = malloc(sizeof(Event));
            event->start = start;
            event->end = end;
            event->whenAddCar = whenAddCar;
            event->destinationVertex = destinationVertex;

            enqueueByPriority(simData -> events, event, whenAddCar);
            simData -> carsActive++;
        }
    }
    int printRoadCommands;
    fscanf(fp, "%d", &printRoadCommands);
    simData -> printComSteps = malloc(sizeof(int) * printRoadCommands);
    for(i = 0; i < printRoadCommands; i++) {
        int cycleToPrintRoads;
        fscanf(fp, "%d", &cycleToPrintRoads);
        simData -> printComSteps[i] = cycleToPrintRoads;
    }
    fclose(fp);
    return simData;
}
