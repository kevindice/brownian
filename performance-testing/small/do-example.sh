#!/bin/bash

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parent_path"

if [ $# -ne 1 ]; then
    echo "You must pass a binary to be tested."
    exit
elif [ ! -x $1 ]; then
    echo "Binary provided does not exist"
    exit
fi

$1 energy_graphene_toluene.dx diffuse_graphene_toluene.dx energy_toluene-toluene.dat init_coords.dat 4e-5 0.59616124 100000 0 pdb 100 small_sim_graph-tol
