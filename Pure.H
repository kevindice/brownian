#ifndef PURE_H
#define PURE_H

#ifdef __CUDACC__
#define __HOSTDEVICE__ __host__ __device__
#else
#define __HOSTDEVICE__
#endif


__HOSTDEVICE__
double pureWrapDiff(double x, double l) {
  int image = int(floor(x/l));
  x -= image*l;
  if (x >= 0.5*l) x -= l;
  return x;
}

__HOSTDEVICE__
Vector3 pureWrapDiff(Vector3 r, Matrix3 basis, Matrix3 basisInv, int nx, int ny, int nz) {
  Vector3 l = basisInv.transform(r);
  l.x = pureWrapDiff(l.x, nx);
  l.y = pureWrapDiff(l.y, ny);
  l.z = pureWrapDiff(l.z, nz);
  return basis.transform(l);
}


// ------------------------

__HOSTDEVICE__
inline double pureWrapReal(double x, double l) {
  int image = int(floor(x/l));
  return x - image*l;
}

__HOSTDEVICE__
double pureWrap(double x, double r0, double dl) {
  return pureWrapReal(x-r0, dl) + r0;
}

__HOSTDEVICE__
double pureComputeGrad(double d, bool periodic, double r0, double dl, double dr, int n, double v1[], double v2[], double v3[]) {
  int home;

  if (periodic) {
    d = pureWrap(d, r0, dl);
    home = int(floor((d - r0)/dr));
  } else {
    home = int(floor((d - r0)/dr));

    if (home < 0) return 0.0;
    if (home >= n) return 0.0;
  }

  double homeR = home*dr + r0;
  double w = (d - homeR)/dr;
  double w2 = w*w;

  // Interpolate.
  return (3.0*v3[home]*w2 + 2.0*v2[home]*w + v1[home])/dr;
}



#ifdef __CUDACC__

void doCompute(
        Vector3 *force,
        Vector3 *pos,
        Matrix3 basis,
        Matrix3 basisInv,
        int nx,
        int ny,
        int nz,
        bool periodic,
        double r0,
        double dl,
        double dr,
        int interact_n,
        double *v1,
        double *v2,
        double *v3,
        int n
)
{

  for (int i = 0; i < n; i++) {
    for (int j = i+1; j < n; j++) {
      Vector3 d = pureWrapDiff(pos[i] - pos[j], basis, basisInv, nx, ny, nz);
      double dist = d.length();
      double fMag = -pureComputeGrad(dist, periodic, r0, dl, dr, interact_n, v1, v2, v3);
      Vector3 f = fMag / dist * d;
      force[i] += f;
      force[j] -= f;
    }
  }

}


template <typename T>
T *CopyArrayToGPU(T *HostArray, int NumElements)
{
  int bytes = sizeof(T) * NumElements;
  void *DeviceArray;

  if (cudaMalloc(&DeviceArray, bytes) != cudaSuccess)
  {
    printf("CopyArrayToGPU(): Couldn't allocate mem for array on GPU.");
    return NULL;
  }

  if (cudaMemcpy(DeviceArray, HostArray, bytes, cudaMemcpyHostToDevice) != cudaSuccess)
  {
    printf("CopyArrayToGPU(): Couldn't copy host array to GPU.");
    cudaFree(DeviceArray);
    return NULL;
  }

  return (T *) DeviceArray;
}

__global__ void doComputeCuda(
        Vector3 *force,
        Vector3 *pos,
        Matrix3 basis,
        Matrix3 basisInv,
        int nx,
        int ny,
        int nz,
        bool periodic,
        double r0,
        double dl,
        double dr,
        int interact_n,
        double *v1,
        double *v2,
        double *v3,
        int n,
        int step
)
{
  int x_index = blockIdx.x * blockDim.x + threadIdx.x;
  int y_index = blockIdx.y * blockDim.y + threadIdx.y;

  /* 2 Dimensional grid stride loop //
     1D Case discussed here:  https://devblogs.nvidia.com/cuda-pro-tip-write-flexible-kernels-grid-stride-loops
     2D example here:  https://stackoverflow.com/questions/22593936/cuda-grid-stride-loops-over-2d-arrays */

  for (int i = x_index; i < n; i += blockDim.x * gridDim.x)
  {
    for (int j = y_index; j < n; j += blockDim.y * gridDim.y)
    {
      if (i != j)
      {
        //double length = (pos[i] - pos[j]).length();
        //printf("Step: %d\tblockIdx: %d,%d\ttheadIdx: %d,%d,%d\tIndex: %d, %d\tDistance: %f\n",
        //    step, blockIdx.x, blockIdx.y, threadIdx.x, threadIdx.y, threadIdx.z, j, i, length );

        Vector3 d = pureWrapDiff(pos[i] - pos[j], basis, basisInv, nx, ny, nz);
        double dist = d.length();
        double fMag = -pureComputeGrad(dist, periodic, r0, dl, dr, interact_n, v1, v2, v3);
        Vector3 f = fMag / dist * d;
        force[i] += f;
        force[j] -= f;
      }
    }
  }
}

#endif

#endif