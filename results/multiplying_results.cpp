#include <iostream> //Código para Generar tablas
#include <map>
#include <vector>
#include <math.h>
#include"timer.c"
#include"output.h"
#include<memory.h>

#include "mpi.h"

typedef std::vector<std::vector<int>> mat_sch;

int mod(int a, int b) {
	int q = a/b;
	return (a - b * (a < 0? q - 1 : q));
}


int stage_type (std::vector<int> stage){
	return stage[0];
}


int stage_value (std::vector<int> stage){
	return stage[1];
}

int all_reduce(int rank, int* com, mat_sch schedule, int* global){
	int* value		= com;
	int stage_mask	= 1;
	int pthres		= 0;
	int pbase		= 1;
	int wid			= rank;

	int sfactor, sbase, mask, offset, peer, rpeer, block;

	MPI_Request request;

	void *rbuf;


	//std::cout<<"Execution by: "<< rank <<std::endl;

	for (auto stage : schedule){
		//std::cout<<"Rank: "<<rank << " -- Stage: "<<stage[2]<<std::endl;
		if ( stage_type(stage) == 1 ){  //Factor Stage
			sfactor	= stage_value(stage);
			sbase	= sfactor * stage_mask;
			int peers[sfactor-1];
			MPI_Status status;
			if ( wid != -1 ){
				for (size_t index = 0; index < sfactor-1; index++)
				{
					mask	= (index + 1) * stage_mask;
					block	= floor( wid / sbase ) * sbase;
					offset	= ( wid + mask ) % sbase;
					peer	= block +  offset;

					if (peer < pthres){
						rpeer	= peer * pbase + pbase - 1;
					}
					else{
						//Always go here
						rpeer	= peer + (pthres/pbase) * (pbase - 1);
						peers[index] = rpeer;
						//std::cout<< "See if dangerous operation is float: " << pthres/pbase<<std::endl;
					}
					// Send non blocking value to rpeer
					MPI_Request request;
					MPI_Isend(value, 1, MPI_INT, rpeer, 0, MPI_COMM_WORLD,&request);
				}

				for (size_t i = 0; i < sfactor - 1; i++)
				{
					// Recv value from peer
					MPI_Request request;
					MPI_Irecv(global, 1, MPI_INT, peers[i], 0, MPI_COMM_WORLD, &request);
					// Reduce value bfur
					MPI_Wait(&request,&status);					MPI_Reduce_local(global, value, 1, MPI_INT, MPI_SUM);
					//std::cout<< "Res local reduce: "<< *(value)<<" Rank: "<<  wid << " From: " << peers[i] <<" Stage: " << stage[2] << " Global: "<< *(global) <<std::endl;
				}
				//std::cout<<"Llego"<<std::endl;
				// Wait on sends
				MPI_Barrier(MPI_COMM_WORLD);
			}
			stage_mask = stage_mask *  sfactor;
		}
	}
}


int main(int argc, char *argv[])
{
	MPI_Init(&argc,&argv);
	mat_sch schedule = {
		{1,4,1}//,
		//{1,6,2}//,
		//{1,3,3}//,
		//{1,2,4},
	};
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	int global;
	int world_size;
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int com =  rank;
	//std::cout<<"Node Value: "<< com << " - Rank: "<<rank<<std::endl;

	//MPI_Allreduce(&com, &global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	struct time_object start, end;
	t_time dataset_raw[BLOCKS];
	for(int j = 0; j < BLOCKS; ++j){
		t_time total_time = 0;
		for(int i  = 0; i < SAMPLES ; ++i){
			com = rank + i;
			start = get_time();
			all_reduce(rank, &com, schedule, &global);
			MPI_Barrier(MPI_COMM_WORLD);
			end = get_time();
			total_time += diff_time(end,start);
		}
		dataset_raw[j] = total_time;
	}
	t_time dataset[BLOCKS];
	MPI_Reduce(dataset_raw,dataset,BLOCKS,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
	if(rank == 0){
                char* str1;
                char* str2;
                char str4[10];
                sprintf(str4, "%d", world_size);
                str1 = "dataset_";
                str2 = "_rm.csv";
                char * str3 = (char *) malloc(1 + strlen(str1)+strlen(str4)+ strlen(str2) );
                strcpy(str3, str1);
                strcat(str3, str4);
                strcat(str3, str2);
		for(int i = 0; i < BLOCKS; ++i)
			dataset_raw[i] /= SAMPLES*1000;
		write_to_csv(dataset_raw,str3);
	}
	std::cout<<std::endl;
	std::cout<<"Res: "<<global<<" ; "<<com<<" -- RANK: "<<rank<<std::endl;
	MPI_Finalize();
	return 0;
}
