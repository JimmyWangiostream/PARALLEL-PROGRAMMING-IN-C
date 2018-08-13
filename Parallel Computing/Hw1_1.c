#include <stdio.h>     // printf()
#include <limits.h>    // UINT_MAX
#include <mpi.h>
int checkCircuit (int, long);

int main (int argc, char *argv[])
{
	long i,parti;       
    int myid;           /* process id */
    int pronum;			//�ΨӦsprocessor���ƥ�	
    int half_check,tmp;	//half_check�ΨӰϤ��ӽ�processor�nSend�٬ORecv,tmp�Ψӱ�processor��Send�X�Ӫ�count 
    int count = 0;        /* number of solutions */

    MPI_Init(&argc , &argv);				
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);	//���oprocessor id
    MPI_Comm_size(MPI_COMM_WORLD,&pronum);  //���oprocessor���ƥ�


    double startTime = 0.0 , totalTime = 0.0; //��l��startTime��totalTime
    startTime = MPI_Wtime();				  //����startTime					

    parti=UINT_MAX/pronum;	                //�C��processor�Ҥ��t�쪺�����

    for (i = myid*parti; i < (myid+1)*parti; i++)   //�C��processor������t�즸�Ʀ���checkCircuit�ç⵲�G�O���bcount
	{
		count+= checkCircuit (myid, i);
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

    if(myid==0)       //��processor 0�ӦL�X���G�Ϊ�O�ɶ�
	{
		printf ("Process  finished.\n");
        fflush (stdout);
        printf("\n%d solutions.\n", count);

        totalTime = MPI_Wtime() - startTime; 
        printf("finished in time %f secs.\n" , totalTime);
    }
    MPI_Finalize();  


    return 0;
}

#define EXTRACT_BIT(n,i) ( (n & (1<<i) ) ? 1 : 0)


/* checkCircuit() checks the circuit for a given input.
 *  * parameters: id, the id of the process checking;
 *   *             bits, the (long) rep. of the input being checked.
 *    *
 *     * output: the binary rep. of bits if the circuit outputs 1
 *      * return: 1 if the circuit outputs 1; 0 otherwise.
 *       */

#define SIZE 32

int checkCircuit (int myid, long bits) 
{
	int v[SIZE];        /* Each element is a bit of bits */
	int i;

	for (i = 0; i < SIZE; i++)
		v[i] = EXTRACT_BIT(bits,i);

	if ( ( (v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
       && (!v[3] || !v[4]) && (v[4] || !v[5])
       && (v[5] || !v[6]) && (v[5] || v[6])
       && (v[6] || !v[15]) && (v[7] || !v[8])
       && (!v[7] || !v[13]) && (v[8] || v[9])
       && (v[8] || !v[9]) && (!v[9] || !v[10])
       && (v[9] || v[11]) && (v[10] || v[11])
       && (v[12] || v[13]) && (v[13] || !v[14])
       && (v[14] || v[15]) )
       ||
	  ( (v[16] || v[17]) && (!v[17] || !v[19]) && (v[18] || v[19])
       && (!v[19] || !v[20]) && (v[20] || !v[21])
       && (v[21] || !v[22]) && (v[21] || v[22])
       && (v[22] || !v[31]) && (v[23] || !v[24])
       && (!v[23] || !v[29]) && (v[24] || v[25])
       && (v[24] || !v[25]) && (!v[25] || !v[26])
       && (v[25] || v[27]) && (v[26] || v[27])
       && (v[28] || v[29]) && (v[29] || !v[30])
       && (v[30] || v[31]) ) )
    {
         printf ("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", id,
              v[31],v[30],v[29],v[28],v[27],v[26],v[25],v[24],v[23],v[22],
              v[21],v[20],v[19],v[18],v[17],v[16],v[15],v[14],v[13],v[12],
              v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
         fflush (stdout);
         return 1;
    } 
	else 
	{
	return 0;
    }
}