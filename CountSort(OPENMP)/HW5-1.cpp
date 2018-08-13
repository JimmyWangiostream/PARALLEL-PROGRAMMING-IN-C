#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#define Range 100  //range of random num
#define N 20       //size of matrix
#define threadnum 4

void gene_Matrix(int * a, int n) //generate a size n a[n] matrix with each a[] <Range and >0
{
    int i;
    for (i = 0; i < n; ++i) 
    {
	int randnum;    
	randnum = random() % Range;    
        a[i] = randnum;
    }
}

void print_Matrix(int * a, int n) //print the Matrix
{
    int i;
    for (i = 0; i < n; ++i) 
    {
        printf("%d ", a[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) 
{
    int i, j,  count;
    srandom(0);
    int * a =(int*) malloc(N* sizeof(int)); //malloc matrix a[N]
    gene_Matrix(a, N);
    printf("=====================New Matrix a=============================\n");
    print_Matrix(a, N);
    int * temp = (int*)malloc(N* sizeof(int));
    double startwtime,endwtime;
    startwtime = omp_get_wtime();	//starttime
    #pragma omp parallel for num_threads(threadnum) \
    default(none) private(i, j, count) shared(a, temp)
    for (i = 0; i < N; i++) 
    {
        count = 0;
        for (j = 0; j < N; j++)
            if (a[j] < a[i])
                count++;
            else if (a[j] == a[i] && j < i)
                count++;
        temp[count] = a[i];
    }
    memcpy ( a , temp, N * sizeof(int));
    endwtime = omp_get_wtime();	//endtime
    free(temp);	//free temp matrix
    printf("=====================Count_sort Matix a========================\n");
    print_Matrix(a,N);
    printf("=====================Spending time=============================\n");
    printf("Spending time: %e sec.\n", endwtime - startwtime);
    return 0;
} 
