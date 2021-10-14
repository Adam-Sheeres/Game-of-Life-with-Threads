/*
    Name: Adam Sheeres-Paulicpulle
    Class: CIS 3090, F21
    Student ID: 1036569
    Purpose: Game of life with threads
    gol_task
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int gridSize = 0;
bool display = false;
bool flag = false;

int testiterations = 0;

//macros to make cleaner code
#define for_x for(int x = 0 ; x < gridSize ; x++)
#define for_y for(int y = 0 ; y < gridSize ; y++)
#define for_xy for_x for_y


/*
Some of the below code was copied from 
examples given by Denis Nikitenko in CIS 3090
*/

//Message struct
typedef struct {
    int x;
    int y;
} Message;

//Message node
typedef struct message_node {
    Message msg;
    struct message_node* next;
} MessageNode;

//Message queue - a singly linked lists
typedef struct {
    MessageNode* head;
    MessageNode* tail;
    pthread_mutex_t mutex;
} MessageQueue;

typedef struct {
    MessageQueue *liveQ;
    MessageQueue *deadQ;
    void *gridCur;
} threadArgs;

void game(int);
int neighbourCount(int, int, void *);

//Create a queue and initilize its mutex
MessageQueue* createMessageQueue()
{
    MessageQueue* q = (MessageQueue*)malloc(sizeof(MessageQueue));
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    return q;
}

//"Send" a message - append it onto the queue
void sendMessage(MessageQueue* q, int x, int y) {

    MessageNode* node = (MessageNode*)malloc(sizeof(MessageNode));
    node->msg.x = x;
    node->msg.y = y;
    node->next = NULL;


    // critical section
    //Obtain lock
    pthread_mutex_lock(&q->mutex);
    if (q->tail != NULL) {
        q->tail->next = node;       // append after tail
        q->tail = node;
    } else {
        q->tail = q->head = node;   // first node
    }
    //Release lock
    pthread_mutex_unlock(&q->mutex);
}

//"Receive" a message - remove it from the queue
int getMessage(MessageQueue* q, Message* msg_out) {
    int success = 0;
    
    // critical section
    //Obtain lock
    pthread_mutex_lock(&q->mutex);
    if (q->head != NULL) {

        MessageNode* oldHead = q->head;
        *msg_out = oldHead->msg;    // copy out the message
        q->head = oldHead->next;
        if (q->head == NULL) {
            q->tail = NULL;         // last node removed
        }
        free(oldHead);
        success = 1;
    }
    //Release lock
    pthread_mutex_unlock(&q->mutex);
    
    return success;
}

/*
Some of the Above code was copied from 
examples given by Denis Nikitenko in CIS 3090
*/

void showGrid(void *grid ) {

    unsigned (*gridCur)[gridSize] = grid;

    for (int i = 0 ; i < gridSize ; i++) {
        for (int j = 0 ; j < gridSize ; j++) {

            //decided  to use these characters to make it more readable
            if (gridCur[i][j] == 1) {
                printf("██ ");
            } else {
                printf("░░ ");
            }
        }
        printf("\n\n");
    }

    for (int i = 0 ; i < gridSize*3 ; i++) {
        printf("-");
    }
    printf("\n\n");
}



int main(int argc, char **argv) {
    int iterations = 0;
    if (!argv[1] || !argv[2]) { //check for input
        printf("Did not get any input\n");
        exit(1);
    } 

    //collect the arguments
    gridSize =    atoi(argv[1]);
    iterations = atoi(argv[2]);
    if (argv[3]) {
        if (strcmp("-d", argv[3]) == 0) {
            display = true;
        }
    }
    //start the game
    game(iterations);

    return 0;
}

int neighbourCount(int x, int y, void *grid) {
    unsigned (*gridLocal)[gridSize] = grid;
    int neighbours = 0;

    //check neighbours to see if they are alive
    for (int i = x-1 ; i <= x+1 ; i++) {
        for (int j = y-1 ; j <= y+1 ; j++) {

            if ((i >= 0 && j >= 0) && (i < gridSize && j < gridSize)) { //bounds check
            
                if (gridLocal[i][j] == 1) {
                    neighbours++;
                }
            }
            
        }
    }

    if (gridLocal[x][y]) neighbours--; //if itself is occupied

    return neighbours;
}

void *threadFuncOne(void *args) {
    threadArgs * arg = (threadArgs *)args;
    unsigned (*grid)[gridSize] = arg->gridCur;
    int nNeighbours = 0;

    for_x {
        for_y {
            nNeighbours = neighbourCount(x, y, grid); //count number of neighbours

            if ((nNeighbours == 2 && grid[x][y] == 1) || 
                 nNeighbours == 3) { 
                //if 2 neighbours and alive, live. If 3 neighbours dead or alive, live
                sendMessage(arg->liveQ, x, y);

            } else {
                sendMessage(arg->deadQ, x, y);

            }
        }
    }
    flag = true;

    return NULL;
}

void *processLiveQ (void *args) {
    threadArgs * arg = (threadArgs *)args;
    unsigned (*grid)[gridSize] = arg->gridCur;
    MessageQueue *liveQ = arg->liveQ;
    Message msg;
    int writeFlag = 0;

    while (1) {
        writeFlag = getMessage(liveQ, &msg);
        if (flag && writeFlag == 0) {
            return NULL;
        } else if (writeFlag == 1) {
            grid[msg.x][msg.y] = 1;
        }
    } 
    return NULL;
}

void *processDeadQ (void *args) {
    threadArgs * arg = (threadArgs *)args;
    unsigned (*grid)[gridSize] = arg->gridCur;
    MessageQueue *deadQ = arg->deadQ;
    Message msg;
    int writeFlag = 0;

    while (1) {
        writeFlag = getMessage(deadQ, &msg);

        if (flag && writeFlag == 0) {
            return NULL;
        } else if (writeFlag == 1) {
            grid[msg.x][msg.y] = 0;
        }
    }

    return NULL;
}


//whole game
void game (int iterations) {

    int grid    [gridSize][gridSize];
    int gridNew [gridSize][gridSize];
    void *funcPointers[3] = {threadFuncOne, processLiveQ, processDeadQ};
    time_t t;

    pthread_t tid[3];
    threadArgs args[3];

    MessageQueue *liveQ = createMessageQueue();
    MessageQueue *deadQ = createMessageQueue();

    srand((unsigned) time(&t));

    //init the grid
    for_xy {
        if ((rand() % 10) > 5) {
            grid[x][y] = 1;
        } else {
            grid[x][y] = 0;
        }
        gridNew[x][y] = 0;
    }



    //make my three threads
    while (iterations > 0) {
            //establish thread-specific aruments
            args[0].gridCur = grid;
            args[1].gridCur = gridNew;
            args[2].gridCur = gridNew;

        //spawn the three threads 
        flag = false;
        for (int i = 0 ; i < 3 ; i ++) {
            args[i].deadQ = deadQ;
            args[i].liveQ = liveQ;
            pthread_create(&tid[i], NULL, funcPointers[i], &args[i]);
        }

        //collect the three threads
        for (int i = 0 ; i < 3 ; i ++) {
            pthread_join(tid[i], NULL);
        }



        //display board
        if (display) {
            showGrid(gridNew);
        }
        
        //switch over grids
        for_xy { 
            grid   [x][y] = gridNew[x][y]; 
            gridNew[x][y] = 0;
        }
           
        iterations --;
    }
    

    free(liveQ);
    free(deadQ);
}   
