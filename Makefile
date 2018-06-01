CFLAGS = -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse
CUDAFLAGS = -O2
MATHFLAGS = -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

default: naive naive-omp tiled tiled-omp pure pure-omp pure-tiled pure-tiled-omp cuda verlet verlet-omp verlet-interval

naive: 00-serial-naive.cpp
	g++ $(CFLAGS) 00-serial-naive.cpp -o bin/00-serial-naive $(MATHFLAGS)

naive-omp: 00-omp-naive.cpp
	g++ $(CFLAGS) 00-omp-naive.cpp -fopenmp -o bin/00-omp-naive $(MATHFLAGS)

tiled: 01-serial-tiled.cpp
	g++ $(CFLAGS) 01-serial-tiled.cpp -o bin/01-serial-tiled $(MATHFLAGS)

tiled-omp: 01-omp-tiled.cpp
	g++ $(CFLAGS) 01-omp-tiled.cpp -fopenmp -o bin/01-omp-tiled $(MATHFLAGS)

pure: 02-serial-pure.cpp
	g++ $(CFLAGS) 02-serial-pure.cpp -o bin/02-serial-pure $(MATHFLAGS)

pure-omp: 02-omp-pure.cpp
	g++ $(CFLAGS) 02-omp-pure.cpp -fopenmp -o bin/02-omp-pure $(MATHFLAGS)

pure-tiled: 03-serial-pure-tiled.cpp
	g++ $(CFLAGS) 03-serial-pure-tiled.cpp -o bin/03-serial-pure-tiled $(MATHFLAGS)

pure-tiled-omp: 03-omp-pure-tiled.cpp
	g++ $(CFLAGS) 03-omp-pure-tiled.cpp -fopenmp -o bin/03-omp-pure-tiled $(MATHFLAGS)

cuda: 04-cuda.cu
	nvcc $(CUDAFLAGS) 04-cuda.cu -o bin/04-cuda $(MATHFLAGS)


verlet: 05-serial-verlet.cpp
	g++ $(CFLAGS) 05-serial-verlet.cpp -o bin/05-serial-verlet $(MATHFLAGS)

verlet-omp: 05-omp-verlet.cpp
	g++ $(CFLAGS) 05-omp-verlet.cpp -fopenmp -o bin/05-omp-verlet $(MATHFLAGS)

verlet-interval: 06-serial-verlet-interval.cpp
	g++ $(CFLAGS) 06-serial-verlet-interval.cpp -o bin/06-serial-verlet-interval $(MATHFLAGS)

verlet-interval-omp: 06-omp-verlet-interval.cpp
	g++ $(CFLAGS) 06-omp-verlet-interval.cpp -fopenmp -o bin/06-omp-verlet-interval $(MATHFLAGS)

clean:
	rm -f bin/*
