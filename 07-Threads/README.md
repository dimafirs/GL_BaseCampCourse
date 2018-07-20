# Very simple OpenMP number-crunching program.

## This program calculates scalar product of two dynamically alocated random vectors.

To build program use gcc command in terminal: <br>
> **gcc -O0 omp_crunch.c -o omp -fopenmp** (fopenmp flag enable openmp usage)
 
Then just run compiled program and add arguments - first is number of threads, second - is array size:
>** ./omp 4 100000 ** <br>

Then you will see result:

>*n=100000, thread_num=2 <br>
>Thread 0 start, from = 0, to = 50000 <br> 
>Thread 1 start, from = 50000, to = 100000 <br>
>Scalar product is: 81059720839 <br>
>Passed time is: 14850.026852 ms <br>*

## Enjoy concurrency!
