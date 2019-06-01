#include <iostream>
#include <map>
#include <vector>
#include <math.h>

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
	MPI_Status status;

	void *rbuf;

	std::cout<<"Execution by: "<< rank <<std::endl;

	for (auto stage : schedule){
		std::cout<<"Rank: "<<rank << " -- Stage: "<<stage[2]<<std::endl;
		if ( stage_type(stage) == 1 ){  //Factor Stage
			sfactor	= stage_value(stage);
			sbase	= sfactor * stage_mask;
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
						rpeer	= peer + (pthres/pbase) * (pbase - 1);
						//std::cout<< "See if dangerous operation is float: " << pthres/pbase<<std::endl;
					}
					// Send non blocking value to rpeer
					MPI_Isend(value, 1, MPI_INT, rpeer, 0, MPI_COMM_WORLD, &request);
				}
				
				for (size_t i = 0; i < sfactor; i++)
				{
					// Recv value from peer
					MPI_Recv(value, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
					// Reduce value bfur
					MPI_Wait(&request, &status);
					MPI_Reduce_local(value, global, 1, MPI_INT, MPI_SUM);
					std::cout<< "Res local reduce: "<< *(global)<<std::endl;
				}
				//std::cout<<"Llego"<<std::endl;
				// Wait on sends
				//MPI_Barrier(MPI_COMM_WORLD);
			}
			stage_mask = stage_mask *  sfactor;
		}
	}
}

int main(int argc, char *argv[])
{
	MPI::Init(argc,argv);
	mat_sch schedule = {
		{1,2,1},
		{1,2,2},
	};
	int rank = MPI::COMM_WORLD.Get_rank();
	int global;
	int com = 5;

	//MPI_Allreduce(&com, &global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	all_reduce(rank, &com, schedule, &global);
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout<<std::endl;
	std::cout<<"Res: "<<global<<" -- RANK: "<<rank<<std::endl;
	return 0;
}
