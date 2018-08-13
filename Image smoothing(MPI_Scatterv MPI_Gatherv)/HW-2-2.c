#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
int Randrange=150;/*set the random number range*/

int cmp(const void* ap, const void* bp) {
        int a = *((const int*)ap);
        int b = *((const int*)bp);
        if(a < b)return -1;
        else if (a > b)return 1;
        else return 0;
}

/* find the index of the smallest item in an array */
int min_index(int* data, int n) {
        int i, min = data[0], mini = 0;
        for (i = 1; i < n; i++) {
                if (data[i] < min) {
                        min = data[i];
                        mini = i;
                }
        }
        return mini;
}

/* find the index of the largest item in an array */
int max_index(int* data, int n) {
        int i, max = data[0], maxi = 0;
        for (i = 1; i < n; i++) {
                if (data[i] > max) {
                        max = data[i];
                        maxi = i;
                }
        }
        return maxi;
}

void parallel_sort(int* data, int rank, int size, int n) {
        int i;
        /* the array we use for reading from partner */
        int other[n];

        /* we need to apply P phases where P is the number of processes */
        for (i = 0; i < size; i++) {
        /* find our partner on this phase */
        int partener;
        /* if it's an even phase */
        if (i % 2 == 0) {
        /* if we are an even process */
                if (rank % 2 == 0) {
                partener = rank + 1;
                }
                else {
                partener = rank - 1;
                }
        }
        else {
        /* it's an odd phase - do the opposite */
                if (rank % 2 == 0) {
                        partener = rank - 1;
                }
                else {
                        partener = rank + 1;
                }
        }

        /* if the partener is invalid, we should simply move on to the next iteration */
        if (partener < 0 || partener >= size) {
                continue;
        }
        /* do the exchange - even processes send first and odd processes receive first
        * *      * this avoids possible deadlock of two processes working together both sending */
        if (rank % 2 == 0) {
                MPI_Send(data, n, MPI_INT, partener, 0, MPI_COMM_WORLD);
                MPI_Recv(other, n, MPI_INT, partener, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else {
                MPI_Recv(other, n, MPI_INT, partener, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(data, n, MPI_INT, partener, 0, MPI_COMM_WORLD);
        }

        /* now we need to merge data and other based on if we want smaller or larger ones */
        if (rank < partener) {
        /* keep smaller keys */
                while (1) {
                /* find the smallest one in the other array */
                        int mini = min_index(other, n);
                        /* find the largest one in out array */
                        int maxi = max_index(data, n);
                        /* if the smallest one in the other array is less than the largest in ours, swap them */
                        if (other[mini] < data[maxi]) {
                                int temp = other[mini];
                                other[mini] = data[maxi];
                                data[maxi] = temp;
                        }
                        else {
                        /* else stop because the smallest are now in data */
                                break;
                        }
                }
        }
        else {
        /* keep larger keys */
                while (1) {
                /* find the largest one in the other array */
                        int maxi = max_index(other, n);
                        /* find the largest one in out array */
                        int mini = min_index(data, n);
                        /* if the largest one in the other array is bigger than the smallest in ours, swap them */
                        if (other[maxi] > data[mini]) {
                                int temp = other[maxi];
                                other[maxi] = data[mini];
                                data[mini] = temp;
                        }
                        else {
                        /* else stop because the largest are now in data */
                                break;
                        }
                }
        }
}
        qsort(data, n, sizeof(int), &cmp);
}


int main(int argc, char* argv[]) {
        /* our rank and size */
        int myid,pronum;
        /*number of data*/
        int num,much;

        /* initialize MPI */
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);
        MPI_Comm_size(MPI_COMM_WORLD, &pronum);

        if(myid == 0)
        {
                printf("The number you want to sort :");
                scanf("%d",&num);
                much = num % pronum;
                num = (num / pronum)+1;
        }
        MPI_Bcast(&num,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&much,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        /* processor's data*/
        int data[num];
        /* initial data */
        int i,j;
        srand(59487+myid);
        for(i = 0;i < num-1;i++)data[i] = rand()% Randrange;
        if(myid < much)data[num-1] = rand()% Randrange;
        if(myid >= much)data[num-1] = Randrange+100;
        /*sort locallists*/
        qsort(data, num, sizeof(int), &cmp);
        /*gather locallists into process 0*/
        int* newdata;
        if(myid == 0)
                newdata = (int*)malloc((pronum * num + much) * sizeof(int));
        MPI_Gather(data, num, MPI_INT, newdata, num, MPI_INT, 0, MPI_COMM_WORLD);

        /*process 0 show locallists*/
        if(myid == 0)
        {
                for(i = 0;i < pronum;i++)
                {
                        printf("locallist[%d] :",i);
                                for(j = i*num;j < (i+1)*num;j++)
                                {
                                        if(i >= much)
                                                if(j == (i+1)*num-1)
                                                        continue;
                                                printf("%d ",newdata[j]);
                                }
                        printf("\n");
                }
        }

        /*parallel odd/even sort */
        parallel_sort(data, myid, pronum, num);
        MPI_Barrier(MPI_COMM_WORLD);
        /*gather into process 0 become globallist*/
        MPI_Gather(data,num ,MPI_INT, newdata, num, MPI_INT, 0, MPI_COMM_WORLD);
        /*process 0 show sorted globallist*/
        if(myid == 0)
        {
                printf("globallist:\n");
                for(i = 0;i < pronum * (num-1) + much;i++)
                {
                        printf("%d ",newdata[i]);
                        if((i + 1) % (num-1) == 0)
                                printf("\n");
                }
        }


        MPI_Finalize( );
        return 0;
}
