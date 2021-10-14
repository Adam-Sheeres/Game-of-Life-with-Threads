CC = gcc
CFLAGS = -Wall
threads = -lpthread

main : gol_data.c
	$(CC) $(CFLAGS) $(threads) -o gol_data gol_data.c
	$(CC) $(CFLAGS) $(threads) -o gol_task gol_task.c
run : main
	./gol_task 100 10000 -d

gol_task_test :
	$(CC) $(CFLAGS) $(threads) -o gol_task_test gol_task_test.c

test: gol_task_test
	./gol_task_test

clean :
	-rm gol_data
	-rm gol_task
	-rm gol_task_test
	-rm *.csv
	-rm *.out
	clear
