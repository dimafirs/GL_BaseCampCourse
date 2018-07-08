#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

int main(int argc, char *argv[]){

    /* Parse terminal argument*/
    if(argc == 1) goto noarg_exc;

    /* Calculate matrix dimension and allocate memory*/
    int n = ceil((sqrt(argc-1)));
    printf("Matrix dimension is %d \n", n);
    double **matrix = malloc(n*sizeof(*matrix));
    for(int i=0; i<n; i++){
        matrix[i] = malloc(n*sizeof(matrix));
    }

    /* Read terminal arguments*/
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            /* +1 to start after program name in argv[]*/
            matrix[i][j] = atof(argv[i*n + j + 1]);
        }
    }

    /* Calculate determinant for input matrix*/
    double determinant = calc_determ(matrix, n);

    printf("Determinant of this matrix equals %f \n", determinant);

    return 0;

    noarg_exc:
    printf("No program arguments!");
    exit(1);
    
}


