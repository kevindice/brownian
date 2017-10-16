ROOT=brownTown3d
ME := $(shell whoami)
default: gnu

debug:
	g++ -ggdb -Wall $(ROOT).C -o $(ROOT) -lm -lgsl -lgslcblas

gnu:
	g++ -O3 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse $(ROOT).C -o $(ROOT) -lm -lgsl -lgslcblas -I/homes/$(ME)/gsl/include/ -L/homes/$(ME)/gsl/lib

clean:
	rm -f $(ROOT) $(ROOT)
