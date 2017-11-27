default: naive verlet

naive: 00-serial-naive.cpp
	g++ -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse 00-serial-naive.cpp -o bin-00-serial-naive -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

verlet: 01-serial-verlet.cpp
	g++ -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse 01-serial-verlet.cpp -o bin-01-serial-verlet -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

clean:
	rm -f $(ROOT) $(ROOT)
