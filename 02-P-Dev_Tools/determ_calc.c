#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

int main(int argc, char *argv[]){

    /* Parse terminal argument*/
    if(argc == 1) goto noarg_exc;

    /* Calculate matrix dimension and allocate memory*/
    int n = (int) round((sqrt(argc-1)));
    if( n*n != argc-1 ) goto argcount_exc;
    printf("Matrix dimension is %d \n", n);
    int k=0;
    double **matrix = malloc(n*sizeof(*matrix));
    if(matrix == NULL) goto malloc_exc;
    for(k=0; k<n; k++){
        matrix[k] = malloc(n*sizeof(matrix));
        if(matrix[k] == NULL) goto malloc_exc;
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
    
    /* Free used memory */
    for(int i=0; i<n; i++)
        free(matrix[i]);
    free(matrix);

    return 0;

    /* Exceptions handling */
    noarg_exc:
    printf("No program arguments! \n");
    exit(1);
    
    argcount_exc:
    printf("Invalid arguments count! Input matrix isn't squared.\n");
    exit(1);

    malloc_exc:
    printf("No memory! (malloc exception) \n");
    /* Free allocated memory */    
    if(k) 
        for(int j=0; j<k; j++)
            free(matrix[j]);
    free(matrix);
    exit(1);
}


