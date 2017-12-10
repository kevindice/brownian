#!/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"
pwd

usage() {
cat <<EOF
Usage: ./perf-test.sh <example> <bin> <verlet refresh interval>

    example: should be the directory containing the example files (big/small)
    bin: the binary name - assume relative to the bin directory
    verlet refresh interval: The interval with which the verlet list rebuilds
EOF
}

if [ $# -ne 3 ]; then
	usage
	exit
elif [ ! -f ../../bin/$2 ]; then
	echo "Error: Binary file to be tested does not exist"
	usage
	exit
elif [ ! -d $1 ]; then
	echo "Error: Example directory does not exist"
	usage
	exit
fi


fmt="DATA\t$(printf "%03d" $3)\t$2\t'$(date)'\t$(whoami)\t$(hostname)\tuser:\t%U\tsystem:\t%S\telapsed:\t%e"

/usr/bin/time -f "$fmt" $1/do-example.sh "$parent_path/../../bin/$2" $3
