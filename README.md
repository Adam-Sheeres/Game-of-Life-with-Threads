# Game-of-Life-with-Threads
This is conway's game of life, with threading! 

To compile:
make

To run: 
./gol_data X X X -d
Where X X X represents the number of threads, the grid size, and the number of iterations. 
So, if I wanted to use 2 threads on a 10x10 grid and iterate 100 times I would put "./gol_data 2 10 100"
The -d flag allows displaying the grid. 

./gol_task X X
Where X X represents the grid size and number of iteraions. See above for more information. 
