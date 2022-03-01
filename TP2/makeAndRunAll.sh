#!/bin/bash

readonly MAT_SIZE=20
readonly VEC_NUMBER=12

for i in {0..2}
do
    # MatVec
    (
        printf "Running MatVec...\n"
        cd Matvec/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 3 --oversubscribe ./main $MAT_SIZE $VEC_NUMBER 0
    )
    
    # MatVecRMA
    (
        printf "Running MatVecRMA...\n"
        cd MatVecRMA/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 3 --oversubscribe ./main $MAT_SIZE $VEC_NUMBER 0
    )
    
    # MatVecMasterSlave
    (
        printf "Running MatVecMasterSlave...\n"
        cd MatVecMasterSlave/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 1 --oversubscribe ./maitre 3 $MAT_SIZE $VEC_NUMBER 0
    )
    
    # MatVecMasterSlaveRMA
    (
        printf "Running MatVecMasterSlaveRMA...\n"
        cd MatVecMasterSlaveRMA/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 1 --oversubscribe ./maitre 3 $MAT_SIZE $VEC_NUMBER 0
    )
done