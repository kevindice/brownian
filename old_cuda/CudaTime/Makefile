CFLAGS = -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse
MATHFLAGS = -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

default: naive-CUDA

naive-CUDA: 04-naive-cuda.cu
	nvcc -lineinfo -arch sm_30 04-naive-cuda.cu -o /home/cgray6/bin/04-naive-cuda $(MATHFLAGS)
  
clean:
	rm -f bin/*
