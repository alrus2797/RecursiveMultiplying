# Recursive Multiplying MPI
MPI Implementation from Generalisation of recursive doubling for allreduce from https://www.sciencedirect.com/science/article/pii/S0167819117301199

## Requeriments
You only need [MPICH](https://www.mpich.org/) to run this code
## Run
You just need to compile the file using `mpic++`
```sh
$ mpic++ --std=c++11 all_reduce.cpp
```
And then use `mpirun`
```sh
$  mpirun --oversubscribe -np 28 ./a.out  
```
The `np` argument is the nnumber of nodes, and `oversubscribe` use oversubscribing. Oversusbscription is the case where you run more processes on a node than there are cores on the node.
You should set *schedule* in `all_reduce.cpp` file when you change the `np` argument.
