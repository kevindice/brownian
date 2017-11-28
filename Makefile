CFLAGS = -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse
MATHFLAGS = -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

default: naive verlet cell

naive: 00-serial-naive.cpp
	g++ $(CFLAGS) 00-serial-naive.cpp -o bin/00-serial-naive $(MATHFLAGS)

verlet: 01-serial-verlet.cpp
	g++ $(CFLAGS) 01-serial-verlet.cpp -o bin/01-serial-verlet $(MATHFLAGS)

cell: 02-serial-verlet-cell.cpp
	g++ $(CFLAGS) 02-serial-verlet-cell.cpp -o bin/02-serial-verlet-cell $(MATHFLAGS)

clean:
	rm -f $(ROOT) $(ROOT)
