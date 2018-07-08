#include "functions.h"

void printMatrix(double **matrix, int n){
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++)
            printf("%f ", matrix[i][j]);
        printf("\n");
    }
    printf("\n");
}

int max_row(double **matrix, int pos, int n){
    int result = pos;
    for(int i=pos+1; i<n; i++)
        if(matrix[result][pos]<matrix[i][pos]) result = i;
    return result;
}

void swap_row(int a, int b, double **matrix, int n){
    double buf = 0;
    for(int i=0; i<n; i++){
        buf = matrix[a][i];
        matrix[a][i] = matrix[b][i];
        matrix[b][i] = buf;
    }
}

double calc_determ(double **matrix, int n){
    for(int i=0; i<n; i++){
        /* Search row in column i with maximum value,
            then put this at i row position*/
        unsigned int i_max = max_row(matrix, i, n);
        if(i!=i_max)
            swap_row(i, i_max, matrix, n);
        for(int j=i+1; j<n; j++){
            /* Calculate coefficient*/
            double buf = matrix[j][i]/matrix[i][i];
            for(int k=i; k<n; k++){
                matrix[j][k] -= matrix[i][k]*buf;
            }
        }
    }

    double determinant = 1;

    for (int i = 0; i < n; i++)
        determinant *= matrix[i][i];

    return determinant;
}

