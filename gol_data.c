/*
    Name: Adam Sheeres-Paulicpulle
    Class: CIS 3090, F21
    Student ID: 1036569
    Purpose: Game of life with threads
    gol_data
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int *grid;
    int *newGrid;
    int startX;
    int endX;
} threadArgs;

//global variables
int gridSize = 0;
int nIterations = 0;
int nThreads = 0;
bool display = false;

//func definitions
void evolve(void *, void *, int, int);
void game(int, int);
void showGrid(void *);
void* workerFunc(void *);

int main(int argc, char **argv) {

    if (!argv[1] || !argv[2] || !argv[3]) { //check for input
        printf("Did not get any input\n");
        exit(1);
    } 

    //collect the arguments
    nThreads =    atoi(argv[1]);
    gridSize =    atoi(argv[2]);
    nIterations = atoi(argv[3]);

    if (nThreads > gridSize) {
        nThreads = gridSize;
    }

    //if user wants to display
    if (argv[4]) {
        if (strcmp("-d", argv[4]) == 0) {
            display = true;
        }
    }

    game(nIterations, nThreads);

    return 0;
}

//display grid if selected
void showGrid(void *grid ) {

    unsigned (*gridCur)[gridSize] = grid;
    char *s[gridSize * gridSize];

    for (int i = 0 ; i < gridSize ; i++) {
        for (int j = 0 ; j < gridSize ; j++) {

            //decided  to use these characters to make it more readable
            if (gridCur[i][j] == 1) {
                strcat(s, "1");
            } else {
                strcat(s, "0");
            }


        }
        strcat(s, "\n");
    }
    strcat(s, "\0");

    printf("\r%s\n", s);
    fflush(stdout);
    sleep(1);
}

//used to evolve a set of cells from the grid
void evolve(void *grid, void *newGrid, int startX, int endX) {

    //cast grids to actual variables
    unsigned (*gridCur)[gridSize] = grid;
    unsigned (*gridNew)[gridSize] = newGrid;
    int neighbours = 0;

    //loop over the col and rows
    for (int i = startX ; i <= endX ; i++) {
        for (int j = 0 ; j < gridSize ; j++) {
            neighbours = 0;


            for (int x = i-1 ; x <= i+1 ; x++) {
                for (int y = j-1 ; y <= j+1 ; y++) {


                    //bounds check
                    if (x >= 0 && y >= 0) {
                        if (x < gridSize && y < gridSize) {


                            if (gridCur[x][y] == 1) { //if it is occupied

                                neighbours++;

                            }
                        }
                    }
                }
            }

        if (gridCur[i][j]) neighbours--;
      
        if ((gridCur[i][j] && neighbours == 2) || neighbours == 3) { //alive
            gridNew[i][j] = 1;   
        } else {
            gridNew[i][j] = 0; //dead
        }
        

        }
    }

}


//function the threads run
void* workerFunc(void *args) {
    threadArgs *arg = (threadArgs *)args;
    
    evolve(arg->grid, arg->newGrid, arg->startX, arg->endX);
    
    return NULL;
}

//responsible for spawning threads and calculating game data
void game(int iterations, int nThreads) {

    int grid[gridSize][gridSize];
    int newGrid[gridSize][gridSize];
    pthread_t tid[nThreads];
    threadArgs args[nThreads]; 
    time_t t;


    int startX, endX = 0;

    //fill the grid randomly with values, empty the next grid
    srand((unsigned) time(&t));
    for (int i = 0 ; i < gridSize ; i++) {
        for (int j = 0 ; j < gridSize ; j++) {
            if ((rand() % 10) > 5) {
                grid[i][j] =  1;
            } else {
                 grid[i][j] = 0;
            }
           
            
            newGrid[i][j] = 0;
        }
    }
    
    //repeat number of defined iterations
    while(iterations > 0) {

        for (int i = 0 ; i < nThreads ; i++) {

            startX = i * (gridSize/nThreads);
            endX = (i + 1)*(gridSize/nThreads) - 1;
            if (i == nThreads-1) {
                endX = gridSize-1;
            }

            //calculate thread aspects
            args[i].endX = endX;
            args[i].startX = startX;
            args[i].grid = *grid;
            args[i].newGrid = *newGrid;

            //spawn threads
            pthread_create(&tid[i], NULL, workerFunc, &args[i]);
        }

        //join threads when they are done
        for (int i = 0 ; i < nThreads ; i++) {
            pthread_join(tid[i], NULL);
        }
            
        //convert to new grid
        for (int i = 0 ; i < gridSize ; i++) {
                for (int j = 0 ; j < gridSize ; j++) {
                    grid[i][j] = newGrid[i][j];
                    newGrid[i][j] = 0;
                }
            }
        iterations --;

        //display the grid after each iteration, if
        if (display) {
            showGrid(grid);
        }
    }



    return;
}