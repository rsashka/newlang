#!/bin/bash

CLR_BOLD='\033[1;33m'
CLR_NO='\033[0m' # No Color
CLR_FAIL='\033[0;31m'
CLR_OK='\033[0;32m'

for filename in *.src; do
#    for ((i=0; i<=3; i++)); do
    echo -ne "Run file ${CLR_BOLD}${filename}${CLR_NO}"
    OUT=`../output/nlc --eval=$filename`
    if [[ "${OUT}" == "OK" ]]; then
	echo -e " - ${CLR_OK}OK${CLR_NO}"
    else
	echo -e " - ${CLR_FAIL}FALSE${CLR_NO}"
    fi;
#    done
done

