#!/bin/bash

writefile=$1
writestr=$2

# Determine the file directory
filedir=$(dirname ${writefile})

# Check to see that 2 parameters were passed into the script
if [ $# -ne 2 ] ; then
    echo "ERROR: Invalid number of arguments, Expected 2"
    exit 1
fi

if [ ! -d ${filedir} ] ; then
    mkdir -p ${filedir} || { echo "ERROR: Directory could not be created"; exit 1; }
fi

echo "${writestr}" > "${writefile}" || { echo "ERROR: File could not be created"; exit 1; }