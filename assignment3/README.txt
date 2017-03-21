Darwin Ding
Parallel Programming Assignment 3

Usage:
mpicc -Wall assignment3.c clcg4.c
mpirun -np <number of processes> ./a.out <threshold (float, e.g. 0.5)>
output: <x> s elapsed, where x is the elapsed time

Works for 1, 2, 4, 8 and 16 cores. Any other odd number of cores will not work.
