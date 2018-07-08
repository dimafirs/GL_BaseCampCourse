#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


/**
 * timespec_diff() - returns time difference in milliseconds for two timespecs.
 * @stop:	time after event.
 * @start:	time before event.
 *
 * Uses difftime() for time_t seconds calcultation.
 */
double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}


/** 
 * insertion_sort() - sorts array into increasing order.
 * @array:	The array to sort.
 * @len:	Number of elements in array.
 *
 * Uses optimized insertion sort (Jon Bentley, 2000).
 * See https://en.wikipedia.org/wiki/Insertion_sort for algorithm.
 */
void insertion_sort(int *array, int len)
{
	int	i, j;
	int	tmp;

	for (i = 1; i < len; i++) {
		j = i;
		tmp = array[j];
		while (j > 0 && array[j-1] > tmp) {
			array[j] = array[j-1];
			j--;
		}
		array[j] = tmp;
	}
}


int main(int argc, char *argv[])
{
	
	/* Initialize pseudo-random generator with (not) truly random number */
	FILE *fp_rand;
	if ((fp_rand = fopen("/dev/urandom", "rb")) == NULL) goto exc_fopen;
	unsigned int seed;
	fread(&seed, sizeof(seed), 1, fp_rand);
	if (ferror(fp_rand)) goto exc_fread;
	fclose(fp_rand);
	srand(seed);


    /* Initialize step amount and count of elements in unsorted array*/
    unsigned int steps = 20;
    unsigned int *count_array = malloc(steps * sizeof(*count_array));
    if (count_array == NULL) goto exc_malloc;
    
    /* Linearly fill count_array*/
    unsigned int step_value = 25000; // 20s step will be 500 000    
    unsigned int cnt;
    for(cnt=0; cnt<steps; cnt++){
        count_array[cnt] = (cnt+1) * step_value;
    }
    
    int *array;
    unsigned int steps_cnt, num_items;
    /* Calculating cycle */
    for(steps_cnt=0; steps_cnt<20; steps_cnt++){
        num_items = count_array[steps_cnt];
	
        /* Fill array with random numbers */
	    array = malloc(num_items * sizeof(*array));
	    if (array == NULL) goto exc_malloc;
	    for (cnt = 0; cnt < num_items; cnt++) {
		    array[cnt] = rand();
	    }

	    /* Sort array */
	    struct timespec time_now, time_after;

	    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_now);
	    insertion_sort(array, num_items);
	    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_after);

	    /* Print measured sorting time */
	    printf("Sorting %u elements took %g ms\n", 
	           num_items, timespec_diff(&time_after, &time_now));
	    free(array);
    }

	return 0;


	/* Exception handling */
	exc_noargs:
	printf("Error: no arguments provided\n");
	goto exc_usage;

	exc_wrongnum:
	printf("Error: num_items should be > 0\n");
	goto exc_usage;

	exc_argunknown:
	printf("Error: unknown argument (%s)\n", argv[2]);
	goto exc_usage;

	exc_argtoomuch:
	printf("Error: too much arguments\n");
	/* Notice: no goto here */

	exc_usage:
	printf("Usage:\n%s <num_items> [--verbose]\n", argv[0]);
	exit(1);

	exc_fopen:
	printf("Error: could not open /dev/random\n");
	exit(1);

	exc_fread:
	printf("Error: read I/O mismatch\n");
	fclose(fp_rand);
	exit(1);

	exc_malloc:
	printf("Error: could not allocate array of size %u\n", num_items);
	free(array);
	exit(1);
}
