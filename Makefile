default: naive verlet cell

naive: 00-serial-naive.cpp
	g++ -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse 00-serial-naive.cpp -o bin-00-serial-naive -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

verlet: 01-serial-verlet.cpp
	g++ -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse 01-serial-verlet.cpp -o bin-01-serial-verlet -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

cell: 02-serial-verlet-cell.cpp
	g++ -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse 02-serial-verlet-cell.cpp -o bin-02-serial-verlet-cell -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

clean:
	rm -f $(ROOT) $(ROOT)
