#!/bin/bash

qsub serial-versions.qsub
qsub -pe single 2 -l mem=5G omp-versions.qsub
qsub -pe single 4 -l mem=3G omp-versions.qsub
qsub -pe single 8 -l mem=2G omp-versions.qsub
qsub -pe single 16 -l mem=1G omp-versions.qsub
qsub -pe single 20 -l mem=1G omp-versions.qsub
