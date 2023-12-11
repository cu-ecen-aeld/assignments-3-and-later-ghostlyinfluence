#!/bin/sh

filesdir=$1
searchstr=$2

# Check to see that 2 parameters were passed into the script
if [ $# -ne 2 ] ; then
    echo "ERROR: Invalid number of arguments, Expected 2"
    exit 1
elif [ ! -d ${filesdir} ] ; then
    echo "ERROR: Provided directory does not exist"
    exit 1
else
    X=$(find ${filesdir} -type f | wc -l)
    Y=$(grep -or ${searchstr} ${filesdir} | wc -l)
    echo "The number of files are ${X} and the number of matching lines are ${Y}"
fi