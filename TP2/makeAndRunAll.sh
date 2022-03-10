#!/bin/bash

readonly MAT_SIZE=4096
readonly VEC_NUMBER=12

currentDate=`date +"%D %T"`
fileName="data-${currentDate}.csv"
fileName=${fileName//"/"/"-"}
fileName=${fileName/":"/"h"}
fileName=${fileName/":"/"m"}
fileName=${fileName/" "/"_"}
touch "${fileName}"

for i in {0..1}
do
    # MatVec
    (
        printf "Running MatVec...\n"
        cd MatVec/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 3 --oversubscribe ./main $MAT_SIZE $VEC_NUMBER 0 $fileName
    )
    
    # MatVecRMA
    (
        printf "Running MatVecRMA...\n"
        cd MatVecRMA/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 3 --oversubscribe ./main $MAT_SIZE $VEC_NUMBER 0 $fileName
    )
    
    # MatVecMasterSlave
    (
        printf "Running MatVecMasterSlave...\n"
        cd MatVecMasterSlave/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 1 --oversubscribe ./maitre 3 $MAT_SIZE $VEC_NUMBER 0 $fileName
    )
    
    # MatVecMasterSlaveRMA
    (
        printf "Running MatVecMasterSlaveRMA...\n"
        cd MatVecMasterSlaveRMA/
        if [ $i == 0 ]; then make; fi;
        mpirun -np 1 --oversubscribe ./maitre 3 $MAT_SIZE $VEC_NUMBER 0 $fileName
    )
done