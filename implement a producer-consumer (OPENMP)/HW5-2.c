#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<string.h>
#include<dirent.h>
#define Max_Filenum 200 //max file num of dir
#define Max_Charnum 200 //max char num of one line
#define Pro_cnt 2 //procedurer num
#define Con_cnt 3  //consumer num

struct list_nodes{         //linequeuestructure
	char* data;
	struct list_nodes* next;
};
int key_cnt = 0;
char keyword[200][200];
int findnum[200]={0};	//each keyword findnum

void Tokenize(char* data,int myrank);	//tokenize linequeue
void Enqueue(char*,struct list_nodes**,struct list_nodes**);	//insert line to linequeue
struct list_nodes* Dequeue(struct list_nodes**,struct list_nodes**,int);	//delete line from linequeue
void prod_cons(int pro_cnt,int con_cnt,FILE* files[],int file_cnt);		//define procedure and consumer Work
void Read_file(FILE*,struct list_nodes**,struct list_nodes**,int);     //producer will enter it to Insert new line to linequeue

			
int main(int argc,char* argv[])
{
	FILE* files[Max_Charnum];
	int file_cnt = 0;
	double startwtime,endwtime;
	int i;
	startwtime = omp_get_wtime(); //start time
	DIR *dir;    //open dir
	{
		char dirname[100];
		sprintf(dirname,"/home/C14041141/txtdata");
		dir = opendir(dirname);
	}
	
	if(dir)		//if open dir successfully
	{
		struct dirent *entry;
		while((entry = readdir(dir))!=NULL)
		{
			if(entry->d_name[0] == '.')continue;
			char txt[]="txtdata/";
			strcat(txt,entry->d_name);
			files[file_cnt] = fopen(txt,"r");
			if(files[file_cnt++] == NULL)
			{
				perror("opening file is Null");
				return(-1);
			}
		}
	}
	
	printf("=================keyword=====================\n");	//print the keywords from key.txt
	FILE* fp;char line[100],*token;
	fp = fopen("key.txt","r");
	while(fgets(line,100,fp)!=NULL)
	{
		token = strtok(line," ,.-\n\r");
		while(token != NULL)
		{
			strcpy(keyword[key_cnt++],token);
			printf("%s ",keyword[key_cnt-1]);
			token = strtok(NULL," ,.-\n\r");
		}
	}
	printf("\n\n");
	fclose(fp);
	printf("=========Producer_readline===================\n");
	prod_cons(Pro_cnt,Con_cnt,files,file_cnt);
	
#pragma omp barrier	 //wait all threads
	endwtime = omp_get_wtime();  //get finish time
	printf("===========Result of Finding=================\n");
	for(i=0;i<key_cnt;i++)
	{
		printf("%s:%d\n",keyword[i],findnum[i]);
	}
	printf("===========Spending Time====================\n");
	printf("Spending time = %lf sec.\n",endwtime-startwtime);

	return 0;
}

void Tokenize(char* data,int myrank)	//consumer tokenize the line from linequeue
{
	int i;
	char *token;
	token = strtok(data," ,.-\n\r");
	while(token != NULL)
	{
		for(i=0;i<key_cnt;i++)
		{
			if(strcasecmp(token,keyword[i])==0)
			{
#pragma omp atomic
				findnum[i]++;
			}
		}
		token = strtok(NULL," ,.-\n\r");
	}
}

void prod_cons(int pro_cnt,int con_cnt,FILE* files[],int file_cnt)
{
	int thread_cnt = Pro_cnt + Con_cnt; //total threads num
	struct list_nodes* head_of_queue = NULL;
	struct list_nodes* tail_of_queue = NULL;
	int prod_done_cnt = 0;

#pragma omp parallel num_threads(thread_cnt) default(none) shared(file_cnt,head_of_queue,tail_of_queue,files,prod_done_cnt)
	{
		int myrank = omp_get_thread_num();
		int j;
		int i;
		if(myrank<Pro_cnt)    //producer will enter
		{
			for(j=myrank;j<file_cnt;j+=Pro_cnt)
			{
				Read_file(files[j],&head_of_queue,&tail_of_queue,myrank);
			}
#pragma omp atomic //only one thread can enter
			prod_done_cnt++;
		}
		else    //consumer wille nter
		{
			for(i=0;i<1000000;i++);
			struct list_nodes* tmp_node;
			
			while(prod_done_cnt<Pro_cnt)
			{
				tmp_node = Dequeue(&head_of_queue,&tail_of_queue,myrank);
				if(tmp_node!=NULL)
				{
#pragma omp critical
					Tokenize(tmp_node->data,myrank);
					free(tmp_node);
				}
			}
			while(head_of_queue!=NULL)
			{
				tmp_node = Dequeue(&head_of_queue,&tail_of_queue,myrank);
				if(tmp_node!=NULL)
				{
					Tokenize(tmp_node->data,myrank);
					free(tmp_node);
				}
			}
		}
	}
}

void Enqueue(char* line,struct list_nodes** head_of_queue,struct list_nodes** tail_of_queue)
{
	struct list_nodes* tmp_node = NULL;

	tmp_node = malloc(sizeof(struct list_nodes));
	tmp_node->next = NULL;
	tmp_node->data = line;

#pragma omp critical
	if(*tail_of_queue == NULL)
	{
		*head_of_queue = tmp_node;
		*tail_of_queue = tmp_node;
	}
	else
	{
		(*tail_of_queue)->next = tmp_node;
		*tail_of_queue = tmp_node;
	}
}

struct list_nodes* Dequeue(struct list_nodes** head_of_queue,struct list_nodes** tail_of_queue,int myrank)
{
	struct list_nodes* tmp_node = NULL;

	if(*head_of_queue == NULL)
		return NULL;

#pragma omp critical
	{
		if(*head_of_queue == *tail_of_queue)
			*tail_of_queue = (*tail_of_queue)->next;
		tmp_node = *head_of_queue;
		*head_of_queue = (*head_of_queue)->next;
	}
	return tmp_node;
}

void Read_file(FILE* file,struct list_nodes** head_of_queue,struct list_nodes** tail_of_queue,int myrank)
{
	char* line = malloc(Max_Charnum*sizeof(char));
	while(fgets(line,Max_Charnum,file)!=NULL)
	{
		printf("Thread %d read line: %s",myrank,line);
		Enqueue(line,head_of_queue,tail_of_queue); //insert the line to linequeue
		line = malloc(Max_Charnum*sizeof(char)); //create a line line for fgets
	}
	fclose(file);
}

