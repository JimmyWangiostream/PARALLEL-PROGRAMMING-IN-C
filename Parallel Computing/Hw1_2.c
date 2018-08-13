#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <math.h>
#define SEED 35792468 //srand之SEED(int type)

int main(int argc, char* argv[])
{
	long long tossnum; //投擲次數
    long long pertoss;
    long long totalcnt;
    long long tmp;     

    scanf("%lld",&tossnum); //輸入投擲次數

    int myid; /* process id */
    int half_check; //half_check用來區分該輪processor要Send還是Recv,tmp用來接processor所Send出來的count 
    double x,y; //random double between-1 and 1
    long long i, count=0; //count=num_in_circuit
    double z; //用來存隨機x y 之sqrt(x*x+y*y);
    double pi; //估計算出的pi
    int pronum; //用來存processor的數目	

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid); //取得processor id
    MPI_Comm_size(MPI_COMM_WORLD, &pronum); //取得processor的數目

    pertoss=tossnum/pronum; //每個processor所分配到的投擲數

    MPI_Bcast(&pertoss,1,MPI_LONG_LONG,0,MPI_COMM_WORLD);//將輸入的tossnum送到每個processor

    srand(SEED);  //隨機變數

    double Stime=0.0,Ptime=0.0;//初始化startTime及totalTime
    Stime=MPI_Wtime();         //紀錄startTime


	for (i=(myid)*pertoss; i<(myid+1)*pertoss; ++i) //每個processor執行分配到次數次的Monte Carlo並把num_in_circle紀錄在count
	{
		x= ((double)rand())/RAND_MAX;
        y =((double)rand())/RAND_MAX;
        z = sqrt(x*x+y*y);

        if(z<=1)
		{
			count++;
		}
    }

    for(half_check = pronum/2;half_check>=1;half_check=half_check/2)  //用Tree-structure(用每輪不同之half_check來做判斷)將所有processor之count的和放到processor 0之count
	{
		if(myid<2*half_check)          //確認該processor是否已Send過,若未Send過則進入if之內進行Send及Recv
		{
			if(myid>=half_check)       //若processor id >=half_check 則為Send端
			{
				MPI_Send(&count,1,MPI_INT,myid-half_check,0,MPI_COMM_WORLD); //將count送給 processor id=myid-half_check之processor
			}
			else                      //否則為Recv端
			{
				MPI_Recv(&tmp,1,MPI_INT,myid+half_check,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //用tmp接收processor id=myid+half_check之processor之count
                count +=tmp;         //更新目前count數
            }
		}
    }

    if(myid==0)  //用processor 0來算出Estimate_pi並印出結果及花費時間
	{
		pi=4*(double)count/tossnum;
        printf("Estimated PI : %f\n",pi);

        Ptime=MPI_Wtime()-Stime;
		printf("time:%f secs",Ptime);
    }
    MPI_Finalize();
    return 0;
}
