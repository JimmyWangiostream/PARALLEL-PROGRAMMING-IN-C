#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

#define NSmooth 1000

BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;

int readBMP( char *fileName);
int saveBMP( char *fileName);
void swap(RGBTRIPLE *a, RGBTRIPLE *b);/*swap temp data and save data*/
RGBTRIPLE **alloc_memory( int Y, int X ); /*allocate the sizeof (y*x)RGBTRIPLE*/

int main(int argc,char *argv[])
{
        char *infileName = "input.bmp";
        char *outfileName = "output1.bmp";/*outputfilename*/
        double startwtime, endwtime;
        int pronum, myid, i;
        int much, new_hight, width,height;
        int *sendcount, *displs, sum = 0;
        MPI_Init(&argc,&argv);
        MPI_Comm_size(MPI_COMM_WORLD,&pronum);/*get process num*/
        MPI_Comm_rank(MPI_COMM_WORLD,&myid);/*get process id*/

        if(myid == 0)/*process 0 read BMP file*/
        {
                if ( readBMP( infileName) )
                        cout << "Read file successfully!!" << endl;
                else
                {
                        cout << "Read file fails!!" << endl;
                        MPI_Finalize();
                        return 0;
                }
                much=bmpInfo.biHeight%pronum;/*num of much height that cant avaragely distribute to each processor*/
                if(much!=0)
                        new_hight=(bmpInfo.biHeight / pronum+1);/*if there is much set newheight in another num*/
                else
                        new_hight = bmpInfo.biHeight / pronum;/*can averagely distribute*/
                        width = bmpInfo.biWidth;/*num of BMPFILE width*/
                        height=bmpInfo.biHeight;/*num of BMPFILE height*/
        }
        MPI_Barrier(MPI_COMM_WORLD);/*wait all processors*/
        MPI_Bcast(&much,1,MPI_INT,0,MPI_COMM_WORLD);/*pass data to orther processors form processor 0*/
        MPI_Bcast(&new_hight,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&width,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&height,1,MPI_INT,0,MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);

        if((myid==pronum-1)&&(much!=0))new_hight=height-(pronum-1)*new_hight;/*set the last processor's new_height if exist much*/

        startwtime = MPI_Wtime();/*set starttime*/

        sendcount = (int*)malloc(sizeof(RGBTRIPLE)*pronum*width*new_hight);/*malloc the array to set the number that each processor get from BMPSave array*/
        displs = (int*)malloc(sizeof(RGBTRIPLE)*pronum*width*new_hight);/*malloc the array to set the position that each processor get from BMPSava array*/

        for(i=0;i<pronum;i++)/*calculate each sentcount and displs by new_height and width*/
        {
                if(i==(pronum-1)&&much!=0)sendcount[i] = width*(height-(height/pronum+1)*(pronum-1))*sizeof(RGBTRIPLE);
                else sendcount[i] = width*new_hight*sizeof(RGBTRIPLE);
                displs[i] = sum;
                sum += sendcount[i];
        }
        RGBTRIPLE **BMPData = NULL;
        RGBTRIPLE **BMPRecv = NULL;
        RGBTRIPLE **BMPNew = NULL;
        BMPData = alloc_memory( new_hight, width);/*each processor's tem BMP data*/
        BMPRecv = alloc_memory( new_hight, width);/*each processor's BMP get from BMPSave Data*/
        BMPNew = alloc_memory(2,width);/*For Recv another processor's data to do Smooth*/
        if (myid == 0)
                MPI_Scatterv((BYTE*)BMPSaveData[0],sendcount,displs,MPI_BYTE,(BYTE*)BMPRecv[0],(new_hight * width * sizeof(RGBTRIPLE)),MPI_BYTE,0,MPI_COMM_WORLD);
                /*Scatterv data to each processor and RGBTRIPLE's data is 3 BYTE*/
        else
                MPI_Scatterv(NULL,sendcount,displs,MPI_BYTE,(BYTE*)BMPRecv[0],(new_hight * width * sizeof(RGBTRIPLE)),MPI_BYTE,0,MPI_COMM_WORLD);
        for(int count = 0; count < NSmooth ; count ++)
        {
                int bufferid1 = (myid == 0) ? pronum-1:myid-1;/*set num of buffer to know the get or send processor*/
                int bufferid2 = (myid == (pronum-1)) ? 0:myid+1;

            if(myid % 2 == 0)/*seperate 2 groups by processor id to do send and recv(because all processors cant all do only recv or send concurrently)*/
                {
                        MPI_Recv((BYTE*)BMPNew[0],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                        MPI_Send((BYTE*)BMPRecv[new_hight-1],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid2,0,MPI_COMM_WORLD);
                }
                else
                {
                        MPI_Send((BYTE*)BMPRecv[new_hight-1],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid2,0,MPI_COMM_WORLD);
                        MPI_Recv((BYTE*)BMPNew[0],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                }

                if(myid % 2 == 0)
                {
                        MPI_Recv((BYTE*)BMPNew[1],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                        MPI_Send((BYTE*)BMPRecv[0],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid1,0,MPI_COMM_WORLD);
                }
                else
                {
                        MPI_Send((BYTE*)BMPRecv[0],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid1,0,MPI_COMM_WORLD);
                        MPI_Recv((BYTE*)BMPNew[1],width*sizeof(RGBTRIPLE),MPI_BYTE,bufferid2,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                }

                MPI_Barrier(MPI_COMM_WORLD);
                swap(BMPRecv,BMPData);/*swap tem data and save data*/

                for(int k = 0; k<new_hight ; k++)
                        for(int j = 0; j<width ; j++)
                        {
                                int Top = k>0 ? k-1 : new_hight-1;/*set Top Down Left Right and to do Smooth*/
                                int Down = k<new_hight-1 ? k+1 : 0;
                                int Left = j>0 ? j-1 : width-1;
                                int Right = j<width-1 ? j+1 : 0;

                                if(k != 0 && k != (new_hight-1))
                                {
                                        BMPRecv[k][j].rgbBlue =  (double) ((double)BMPData[k][j].rgbBlue+(double)BMPData[Top][j].rgbBlue+(double)BMPData[Down][j].rgbBlue+(double)BMPData[k][Left].rgbBlue+(double)BMPData[k][Right].rgbBlue)/5+0.5;

                                        BMPRecv[k][j].rgbGreen =  (double) ((double)BMPData[k][j].rgbGreen+(double)BMPData[Top][j].rgbGreen+(double)BMPData[Down][j].rgbGreen+(double)BMPData[k][Left].rgbGreen+(double)BMPData[k][Right].rgbGreen)/5+0.5;

                                        BMPRecv[k][j].rgbRed =  (double) ((double)BMPData[k][j].rgbRed+(double)BMPData[Top][j].rgbRed+(double)BMPData[Down][j].rgbRed+(double)BMPData[k][Left].rgbRed+(double)BMPData[k][Right].rgbRed)/5+0.5;
                                }

                                else if(k == 0)
                                {
                                        BMPRecv[k][j].rgbBlue =  (double) ((double)BMPData[k][j].rgbBlue+(double)BMPNew[0][j].rgbBlue+(double)BMPData[Down][j].rgbBlue+(double)BMPData[k][Left].rgbBlue+(double)BMPData[k][Right].rgbBlue)/5+0.5;

                                        BMPRecv[k][j].rgbGreen =  (double) ((double)BMPData[k][j].rgbGreen+(double)BMPNew[0][j].rgbGreen+(double)BMPData[Down][j].rgbGreen+(double)BMPData[k][Left].rgbGreen+(double)BMPData[k][Right].rgbGreen)/5+0.5;

                                        BMPRecv[k][j].rgbRed =  (double) ((double)BMPData[k][j].rgbRed+(double)BMPNew[0][j].rgbRed+(double)BMPData[Down][j].rgbRed+(double)BMPData[k][Left].rgbRed+(double)BMPData[k][Right].rgbRed)/5+0.5;
                                }
                                else if(k == (new_hight-1))
                                {
                                        BMPRecv[k][j].rgbBlue =  (double) ((double)BMPData[k][j].rgbBlue+(double)BMPNew[1][j].rgbBlue+(double)BMPData[Top][j].rgbBlue+(double)BMPData[k][Left].rgbBlue+(double)BMPData[k][Right].rgbBlue)/5+0.5;

                                        BMPRecv[k][j].rgbGreen =  (double) ((double)BMPData[k][j].rgbGreen+(double)BMPNew[1][j].rgbGreen+(double)BMPData[Top][j].rgbGreen+(double)BMPData[k][Left].rgbGreen+(double)BMPData[k][Right].rgbGreen)/5+0.5;

                                        BMPRecv[k][j].rgbRed =  (double) ((double)BMPData[k][j].rgbRed+(double)BMPNew[1][j].rgbRed+(double)BMPData[Top][j].rgbRed+(double)BMPData[k][Left].rgbRed+(double)BMPData[k][Right].rgbRed)/5+0.5;
                                }
                        }

                }

                if(myid == 0)
                {
                  MPI_Gatherv((BYTE*)BMPRecv[0],(new_hight * width * sizeof(RGBTRIPLE)),MPI_BYTE,(char*)BMPSaveData[0],sendcount,displs,MPI_BYTE,0,MPI_COMM_WORLD);
                  /*Gatherv each proccesor's BMPRecv to BMPSaveData*/
                }
                else
                {
                        MPI_Gatherv((BYTE*)BMPRecv[0],(new_hight*width*sizeof(RGBTRIPLE)),MPI_BYTE,NULL,sendcount,displs,MPI_BYTE,0,MPI_COMM_WORLD);
                }

                MPI_Barrier(MPI_COMM_WORLD);

                if(myid == 0)
                {
                        endwtime = MPI_Wtime();/*set end time and printf exe time*/
                cout << "The execution time = "<< endwtime-startwtime <<" sec."<<endl ;
                        if ( saveBMP( outfileName ) )
                                cout << "Save file successfully!!" << endl;
                        else
                                cout << "Save file fails!!" << endl;
                }
                free(BMPData);
                free(BMPRecv);
                free(BMPSaveData);

                MPI_Finalize();

                return 0;
}
int readBMP(char *fileName)
{
        ifstream bmpFile( fileName, ios::in | ios::binary );

        if ( !bmpFile )
        {
                cout << "It can't open file!!" << endl;
                return 0;
        }

        bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );
        if( bmpHeader.bfType != 0x4d42 )
        {
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }

        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );

        if ( bmpInfo.biBitCount != 24 )
        {
                cout << "The file is not 24 bits!!" << endl;
                return 0;
        }

        while( bmpInfo.biWidth % 4 != 0 )
                bmpInfo.biWidth++;

        BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
        bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);

        bmpFile.close();

        return 1;

}
int saveBMP( char *fileName)
{
        if( bmpHeader.bfType != 0x4d42 )
        {
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }

        ofstream newFile( fileName,  ios:: out | ios::binary );

        if ( !newFile )
        {
                cout << "The File can't create!!" << endl;
                return 0;
        }

        newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );
        newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );
        newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );
        newFile.close();

        return 1;

}

RGBTRIPLE **alloc_memory(int Y, int X )
{
        RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
        RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
        memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
        memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

        for( int i = 0; i < Y; i++)
        {
                temp[ i ] = &temp2[i*X];
        }

        return temp;

}
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
                RGBTRIPLE *temp;
                        temp = a;
                                a = b;
                                        b = temp;
}
