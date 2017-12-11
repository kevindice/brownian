CFLAGS = -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse
MATHFLAGS = -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

default: naive naive-omp verlet verlet-omp verlet-interval verlet-interval-omp cell
experiment: verlet-interval-experiment

naive: 00-serial-naive.cpp
	g++ $(CFLAGS) 00-serial-naive.cpp -o bin/00-serial-naive $(MATHFLAGS)

naive-omp: 00-omp-naive.cpp
	g++ $(CFLAGS) 00-omp-naive.cpp -fopenmp -o bin/00-omp-naive $(MATHFLAGS)

verlet: 01-serial-verlet.cpp
	g++ $(CFLAGS) 01-serial-verlet.cpp -o bin/01-serial-verlet $(MATHFLAGS)

verlet-omp: 01-omp-verlet.cpp
	g++ $(CFLAGS) 01-omp-verlet.cpp -fopenmp -o bin/01-omp-verlet $(MATHFLAGS)

verlet-interval: 02-serial-verlet-interval.cpp
	g++ $(CFLAGS) 02-serial-verlet-interval.cpp -o bin/02-serial-verlet-interval $(MATHFLAGS)

verlet-interval-omp: 02-omp-verlet-interval.cpp
	g++ $(CFLAGS) 02-omp-verlet-interval.cpp -fopenmp -o bin/02-omp-verlet-interval $(MATHFLAGS)

verlet-interval-experiment: 02a-serial-verlet-interval-experiment.cpp
	g++ $(CFLAGS) 02a-serial-verlet-interval-experiment.cpp -o bin/02a-serial-verlet-interval-experiment $(MATHFLAGS)

cell: 03-serial-verlet-cell.cpp
	g++ $(CFLAGS) 03-serial-verlet-cell.cpp -o bin/03-serial-verlet-cell $(MATHFLAGS)

clean:
	rm -f bin/*
