#include <stdlib.h>
#include <stdio.h>

#include "trafficSimulator.h"

int main( int argc, char *argv[] )
{
    int i;
    int numFiles = 6;
    char *defaultFilenames[] = {"data-Trivial.txt", "data-Simple.txt", "data-Gridlock.txt", "data-NearGridlock.txt", "data-Complex1.txt", "data-Complex2.txt"};

    if( argc==2 ) /* Usage: ./driver <filename> */
        trafficSimulator( argv[1] );
    else{
        printNames( );
        printf("\nRunning default test files: \n\n");

        for( i=0; i<numFiles; i++ ){
            printf("\n\n--------------- START OF OUTPUT FOR %s -----------------\n\n", defaultFilenames[i]);
            trafficSimulator( defaultFilenames[i] );
            printf("\n--------------- END OF OUTPUT FOR %s -----------------\n\n", defaultFilenames[i]);
        };
    }

    return 0;
}
