#!/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"

usage() {
cat <<EOF
Usage: ./perf-test.sh <example> <bin>

    example: should be the directory containing the example files (big/small)
    bin: the binary name - assume relative to the bin directory

EOF
}

if [ $# -ne 2 ]; then
	usage
	exit
elif [ ! -f ../bin/$2 ]; then
	echo "Error: Binary file to be tested does not exist"
	usage
	exit
elif [ ! -d $1 ]; then
	echo "Error: Example directory does not exist"
	usage
	exit
fi

pwd

fmt="DATA\t$2\t$1\t'$(date)'\t$(whoami)\t$(hostname)\tuser:\t%U\tsystem:\t%S\telapsed:\t%e\t$NSLOTS"

/usr/bin/time -f "$fmt" $1/do-example.sh "$parent_path/../bin/$2"
