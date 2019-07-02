#include <iostream> //Ez
#include <map>
#include <vector>
#include <math.h>

#include "mpi.h"
#include <memory.h>
#include"timer.c"
#include"output.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	MPI_Init(&argc,&argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	int global;
	int com =  rank;
	int world_size;
    	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	struct time_object start, end;
	t_time dataset_raw[BLOCKS];
	for(int j = 0; j < BLOCKS; ++j){
		t_time total_time = 0;
		for(int i  = 0; i < SAMPLES ; ++i){
			com = rank + i;
			start = get_time();
			MPI_Allreduce(&com, &com,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			end = get_time();
			total_time += diff_time(end,start);
		}
		dataset_raw[j] = total_time;
	}
	t_time dataset[BLOCKS];
	MPI_Reduce(dataset_raw,dataset,BLOCKS,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	if(rank == 0){
		for(int i = 0; i < BLOCKS; ++i)
			dataset_raw[i] /= SAMPLES*1000;
		char* str1;
      		char* str2;
  		char str4[10];
  		sprintf(str4, "%d", world_size);
      		str1 = "dataset_";
     		str2 = "_rd.csv";
      		char * str3 = (char *) malloc(1 + strlen(str1)+strlen(str4)+ strlen(str2) );
		strcpy(str3, str1);
      		strcat(str3, str4);
      		strcat(str3, str2);
		write_to_csv(dataset_raw,str3);
	}
	std::cout<<std::endl;
	std::cout<<"Res: "<<global<<" ; "<<com<<" -- RANK: "<<rank<<std::endl;
	MPI_Finalize();
	return 0;
}
