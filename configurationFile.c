

/* Testdata
2
5 6 8 9
0 0 3 4

*/

#include <stdio.h>
#include <stdlib.h>

int size = 0;

int *xIntervals = NULL;
int *yIntervals = NULL;

void readConfigurations();
int isPixelValid(int row, int col);

void readConfigurations(){
    int numOfRectRegions;
    puts("Enter amount of regions");
    scanf("%d", &numOfRectRegions);

    xIntervals = (int*) malloc((numOfRectRegions * 2) * sizeof(int));
    yIntervals = (int*) malloc((numOfRectRegions * 2) * sizeof(int));

    int xL = 0, yL = 0;
    int x1, y1, x2, y2;
    puts("Enter x1, y1, x2, y2");
    for (int i = 0; i < numOfRectRegions; i++){
        scanf("%d %d %d %d", &x1, &y1, &x2, &y2);
        xIntervals[xL++] = x1;
        xIntervals[xL++] = x2;
        yIntervals[yL++] = y1;
        yIntervals[yL++] = y2;
        size++;
        size++;
    }
}

int isPixelValid(int row, int col){

    for (int i = 0; i < size; i+=2){
        int boolX = xIntervals[i] <= row && row <= xIntervals[i+1];
        int boolY = yIntervals[i] <= col && col <= yIntervals[i+1];
        if (boolX && boolY){
            return 0;
        }
    }

    return 1;
}
