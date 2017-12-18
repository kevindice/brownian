#!/bin/bash 

qsub -l killable -l h_rt=00:10:00 -l mem=16G -l cuda=TRUE -q \*@@dwarves /homes/cgray6/Project-Brownian/performance-testing/perf-test.sh