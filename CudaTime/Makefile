CFLAGS = -O2 -Wall -Wno-strict-overflow -Wno-unused-result -march=native -mfpmath=sse
MATHFLAGS = -lm -I$(HOME)/gsl/include/ -L$(HOME)/gsl/lib -lgsl -lgslcblas

default: naive-CUDA

naive-CUDA: 04-naive-cuda.cu
	nvcc -I  /opt/cuda/sdk/common/inc -arch sm_30  04-naive-cuda.cu -o bin/04-naive-cuda $(MATHFLAGS)
  
clean:
	rm -f bin/*
