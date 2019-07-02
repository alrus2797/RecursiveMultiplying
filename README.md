# Recursive Multiplying MPI
Implementation from Generalisation of recursive doubling for allreduce from https://www.sciencedirect.com/science/article/pii/S0167819117301199

## Requeriments
You only need [MPICH](https://www.mpich.org/) to run this code
## Run
You just need to compile the file using `mpic++`
```sh
$ mpic++ --std=c++11 all_reduce.cpp
```
And then use `mpirun`
```sh
$  mpirun -np 28 ./a.out  
```
The `np` argument is the nnumber of nodes, and `oversubscribe` use oversubscribing. Oversusbscription is the case where you run more processes on a node than there are cores on the node.
You should set *schedule* in `all_reduce.cpp` file when you change the `np` argument.
**Note**
The structure of each stage in schedule {a,b,c,d} is:
- a is the type of stage. 0 is Collapse, 1 is Factorization, 2 is Expand
- b in case of stage is Collapse or Expand is the rank from where we take the nodes away. In case is Factorization stage is the factor.
- c in case of Collapse or Expand is the number of nodes from *b* wich is going to take away. Otherwyse doesn't matter.
- d Doesn't matter.
