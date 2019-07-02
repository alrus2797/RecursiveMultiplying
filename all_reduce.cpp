#include <iostream> //Ez
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
					MPI_Send(value, 1, MPI_INT, rpeer, 0, MPI_COMM_WORLD);
				}

				for (size_t i = 0; i < sfactor - 1; i++)
				{
					// Recv value from peer
					MPI_Recv(global, 1, MPI_INT, peers[i], 0, MPI_COMM_WORLD, &status);
					// Reduce value bfur
					MPI_Reduce_local(global, value, 1, MPI_INT, MPI_SUM);
					std::cout<< "Res local reduce: "<< *(value)<<" Rank: "<<  wid << " From: " << peers[i] <<" Stage: " << stage[2] << " Global: "<< *(global) <<std::endl;
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
		{1,2,1},
		{1,2,2},
		{1,2,3},
		{1,2,4},
	};
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	int global;
	int com =  rank;
	//std::cout<<"Node Value: "<< com << " - Rank: "<<rank<<std::endl;

	//MPI_Allreduce(&com, &global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	all_reduce(rank, &com, schedule, &global);
	MPI_Barrier(MPI_COMM_WORLD);
	std::cout<<std::endl;
	std::cout<<"Res: "<<global<<" ; "<<com<<" -- RANK: "<<rank<<std::endl;

	MPI_Finalize();
	return 0;
}
