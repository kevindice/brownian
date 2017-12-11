default: cuda serial

serial: add.cpp
	g++ add.cpp -o add_serial

cuda: add.cu
	nvcc -I  /opt/cuda/sdk/common/inc -arch sm_30 add.cu -o add_cuda

clean:
	rm -f $(ROOT) $(ROOT)
