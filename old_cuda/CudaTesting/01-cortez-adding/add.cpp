#include <iostream>
#include <math.h>
#include <time.h>

// function to add the elements of two arrays
void add(int n, float *x, float *y)
{
  for (int i = 0; i < n; i++)
      y[i] = x[i] + y[i];
}

double myclock() 
{
   static time_t t_start = 0;  // Save and subtract off each time

   struct timespec ts;
   clock_gettime(CLOCK_REALTIME, &ts);
   if( t_start == 0 ) t_start = ts.tv_sec;

   return (double) (ts.tv_sec - t_start) + ts.tv_nsec * 1.0e-9;
}

int main(void)
{
  int N = 3200000; // 1M elements

  float *x = new float[N];
  float *y = new float[N];
  
  double tstart, ttotal;
  
  
  // initialize x and y arrays on the host
  for (int i = 0; i < N; i++) 
  {
    x[i] = 1.0f;
    y[i] = 2.0f;
  }
  
  //timing
  tstart = myclock();  // Set the zero time
  tstart = myclock();  // Start the clock
  
  // Run kernel on 1M elements on the CPU
  add(N, x, y);
  
  //end time
  ttotal = myclock() - tstart;
  
  // Check for errors (all values should be 3.0f)
  float maxError = 0.0f;
  for (int i = 0; i < N; i++)
    maxError = fmax(maxError, fabs(y[i]-3.0f));
  std::cout << "Max error: " << maxError << std::endl;
  std::cout << "Sum: " << y[1] << std::endl;
  std::cout << "Time: " << ttotal << std::endl;
  
  // Free memory
  delete [] x;
  delete [] y;

  return 0;
}