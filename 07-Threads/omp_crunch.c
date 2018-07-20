#include <omp.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

enum err {
	E_OK = 0,
	E_NOARG,
	E_WRONGARG,
	E_FOPEN,
	E_FREAD,
	E_NOMEM
};

static const char* const _err_msg[] = {
	[E_OK] = "Successful\n",
	[E_NOARG] = "No command argumets found\n",
	[E_WRONGARG] = "Invalid command argumets\n",
 	[E_FOPEN] = "Unable to open file '/dev/random'\n",
	[E_FREAD] = "Unable to read file '/dev/random'\n",
 	[E_NOMEM] = "No memory\n"
};

/**
 * timespec_diff() - returns time difference in milliseconds for two timespecs.
 * @stop:	time after event.
 * @start:	time before event.
 *
 * Uses difftime() for time_t seconds calcultation.
 */
static double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}

long calc_scalar(int *A, int *B, unsigned long from, unsigned long to){
	long result = 0;
	for(unsigned long i = from; i<to; i++){
		/* Add empty cycle to increase theard calc time*/
		for(unsigned long j=from; j<to; j++) {}
		/* Calculate product */
		result += A[i]*B[i];
	}
	return result;
}
	
int main(int argc, char *argv[]){

	enum err err_lvl = E_OK;	
	if(argc<2){
		err_lvl = E_NOARG;
		goto err_handler;
	}
	
	if(argc!=3){
		err_lvl = E_WRONGARG;
		goto err_handler;
	}
	
	/* Read command line arguments */	
	int num_th = 0;
	unsigned long n = 0;
	
	num_th = atoi(argv[1]);
	n = atol(argv[2]);
	printf("n=%lu, thread_num=%d\n", n, num_th);
	if( (n<=0) | (num_th <=0) ){
		err_lvl = E_WRONGARG;
		goto err_handler;
	}
	
	/* Setup random*/ 
	FILE *fp_rand = fopen("/dev/random", "rb");
	if (NULL == fp_rand) {
		err_lvl = E_FOPEN;
		goto err_handler;
	}
	unsigned int seed;
	fread(&seed, sizeof(seed), 1, fp_rand);
	if (ferror(fp_rand)) {
		err_lvl = E_FREAD;
		goto err_fread;
	}
	srand(seed);
	
	/* Allocate memory for vectors */
	int *A = malloc(n * sizeof(*A));
	int *B = malloc(n * sizeof(*B));
	if( (A == NULL) | (B == NULL) ){
		err_lvl = E_NOMEM;
		goto err_handler;
	}
	
	/*Fill vectors with random values */
	for(unsigned long i = 0; i<n; i++){
		A[i] = rand();
		B[i] = rand();
	}
	
	int h = n / num_th;
	long result = 0;
	struct timespec time_now, time_after;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_now);
	omp_lock_t sum_lock;
	omp_init_lock(&sum_lock);// init lock

	#pragma omp parallel num_threads(num_th)
	{
		const int tid = omp_get_thread_num();
		unsigned long left = tid * h;
		unsigned long right = (tid + 1) * h;
		printf("Thread %d start, from = %lu, to = %lu \n", tid, left, right);
		
		long scalar = calc_scalar(A, B, left, right);

		omp_set_lock(&sum_lock);
		result += scalar;
		omp_unset_lock(&sum_lock);

		#pragma omp barrier // wait finish computing
	}
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time_after);
	double took_global = timespec_diff(&time_after, &time_now);

	printf("Scalar product is: %ld\nPassed time is: %f ms\n ", result, took_global);

	omp_destroy_lock(&sum_lock);
	free(A);
	free(B);
	exit(0);
	
	err_fread:
		fclose(fp_rand);

	err_handler:
	printf("%s", _err_msg[err_lvl]);
    	exit(err_lvl);
}
