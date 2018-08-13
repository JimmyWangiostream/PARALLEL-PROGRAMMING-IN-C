#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#define SEED 35792468 //srand��SEED(int type)

int main(int argc, char* argv[])
{
	long long tossnum; //���Y����
    long long pertoss;
    long long totalcnt;
    long long tmp;     

    scanf("%lld",&tossnum); //��J���Y����

    int myid; /* process id */
    int half_check; //half_check�ΨӰϤ��ӽ�processor�nSend�٬ORecv,tmp�Ψӱ�processor��Send�X�Ӫ�count 
    double x,y; //random double between-1 and 1
    long long i, count=0; //count=num_in_circuit
    double z; //�ΨӦs�H��x y ��sqrt(x*x+y*y);
    double pi; //���p��X��pi
    int pronum; //�ΨӦsprocessor���ƥ�	

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid); //���oprocessor id
    MPI_Comm_size(MPI_COMM_WORLD, &pronum); //���oprocessor���ƥ�

    pertoss=tossnum/pronum; //�C��processor�Ҥ��t�쪺���Y��

    MPI_Bcast(&pertoss,1,MPI_LONG_LONG,0,MPI_COMM_WORLD);//�N��J��tossnum�e��C��processor

    srand(SEED);  //�H���ܼ�

    double Stime=0.0,Ptime=0.0;//��l��startTime��totalTime
    Stime=MPI_Wtime();         //����startTime


	for (i=(myid)*pertoss; i<(myid+1)*pertoss; ++i) //�C��processor������t�즸�Ʀ���Monte Carlo�ç�num_in_circle�����bcount
	{
		x= ((double)rand())/RAND_MAX;
        y =((double)rand())/RAND_MAX;
        z = sqrt(x*x+y*y);

        if(z<=1)
		{
			count++;
		}
    }

    for(half_check = pronum/2;half_check>=1;half_check=half_check/2)  //��Tree-structure(�ΨC�����P��half_check�Ӱ��P�_)�N�Ҧ�processor��count���M���processor 0��count
	{
		if(myid<2*half_check)          //�T�{��processor�O�_�wSend�L,�Y��Send�L�h�i�Jif�����i��Send��Recv
		{
			if(myid>=half_check)       //�Yprocessor id >=half_check �h��Send��
			{
				MPI_Send(&count,1,MPI_INT,myid-half_check,0,MPI_COMM_WORLD); //�Ncount�e�� processor id=myid-half_check��processor
			}
			else                      //�_�h��Recv��
			{
				MPI_Recv(&tmp,1,MPI_INT,myid+half_check,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //��tmp����processor id=myid+half_check��processor��count
                count +=tmp;         //��s�ثecount��
            }
		}
    }

    if(myid==0)  //��processor 0�Ӻ�XEstimate_pi�æL�X���G�Ϊ�O�ɶ�
	{
		pi=4*(double)count/tossnum;
        printf("Estimated PI : %f\n",pi);

        Ptime=MPI_Wtime()-Stime;
		printf("time:%f secs",Ptime);
    }
    MPI_Finalize();
    return 0;
}
