#include <iostream>
#include <math.h>
// Kernel function to add the elements of two arrays
__global__
void add(int n, float *x, float *y)
{
  //threadIdx.x is the index of the current thread within its block
  //blockDim.x is the number of threads in the block
  //gridDim.x is the number of blocks in the grid
  //blockIdx.x is the index of the current thread block in the grid
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  for (int i = index; i < n; i += stride)
      y[i] = x[i] + y[i];
}

int main(void)
{
  int N = 3200000;
  float *x, *y;
  int devId = 0;
  
  // Allocate Unified Memory – accessible from CPU or GPU
  cudaMallocManaged(&x, N*sizeof(float));
  cudaMallocManaged(&y, N*sizeof(float));
  
  //timing code
  cudaEvent_t start, stop;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);

  // initialize x and y arrays on the host
  for (int i = 0; i < N; i++) {
    x[i] = 1.0f;
    y[i] = 2.0f;
  }

  
  
  int numSMs;
  //grab the amount of SMs 
  cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, devId);
  int blockSize = 32;
  
  cudaEventRecord(start);
  // Run kernel on 3.2M elements on the GPU
  // >>> dictates the number of threads in a thread block
    //note that cuda blocks threads by multiples of 32
      //beocat has two graphic cards 980ti and 1080ti
      //980ti has 2,816 cuda cores with 22 SMs
      //1080ti has 3,584 cuda cores with 28 SMs
  // <<< dictates the number of thread blocks
  add<<<blockSize*numSMs, 256>>>(N, x, y);
  cudaEventRecord(stop);


  // Wait for GPU to finish before accessing on host
  cudaDeviceSynchronize();
  
  //get time
  float milliseconds = 0;
  cudaEventElapsedTime(&milliseconds, start, stop);
  
  // Check for errors (all values should be 3.0f)
  float maxError = 0.0f;
  for (int i = 0; i < N; i++)
    maxError = fmax(maxError, fabs(y[i]-3.0f));
    
  std::cout << "Max error: " << maxError << std::endl;
  std::cout << "Sum: " << y[1] << std::endl;
  std::cout << "Time: " << milliseconds/1000 << std::endl;
  
  // Free memory
  cudaFree(x);
  cudaFree(y);
  
  return 0;
}