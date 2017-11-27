default: naive

debug:
	g++ -ggdb -Wall $(ROOT).C -o $(ROOT) -lm -lgsl -lgslcblas

naive: 00-serial-naive.c
	g++ -O3 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse 00-serial-naive.c -o bin-00-serial-naive -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

clean:
	rm -f $(ROOT) $(ROOT)
