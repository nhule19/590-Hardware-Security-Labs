#!/bin/bash

if [ -z "$*" ]; then 
    echo "Executable argument required!"
    exit 0
fi


mkdir -p log
rm -f log/${1}.log log/${1}.error log/${1}.out

if ! condor_submit log=log/${1}.log error=log/${1}.error output=log/${1}.out \
                 executable=bin/${1} launch.condor; then
    echo "condor_submit failed"
    exit 1
fi

condor_wait log/${1}.log