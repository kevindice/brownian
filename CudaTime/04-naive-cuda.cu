///////////////////////////////////////////////////////////////////////
// Author: Jeff Comer <jeffcomer at gmail>

#include <ctime>
#include <iostream>
#include "useful.H"
#include "PiecewiseCubic.H"
#include "Scatter.H"
#include "RandomGsl.H"
#include "BaseGrid.H"
#include "TrajectoryWriter.H"
#include <math.h>
#include <math_functions.h>

__device__ __host__
static double cuda_wrapDiffDeep(double x, double l) 
{
    int image = int(floor(x/l));
    x -= image*l;
    if (x >= 0.5*l) x -= l;
    return x;
}

__device__ __host__
Vector3 cuda_wrapDiff(Vector3 r, Matrix3 basis, Matrix3 basisInv, int nx, int ny, int nz) 
{
  Vector3 l = basisInv.transform(r);
  l.x = cuda_wrapDiffDeep(l.x, nx);
  l.y = cuda_wrapDiffDeep(l.y, ny);
  l.z = cuda_wrapDiffDeep(l.z, nz);
  return basis.transform(l);
}

__device__ __host__
double cuda_computeGrad(double d, double dl, double r0, double dr, int n, double* v1, double* v2, double* v3) 
{
  n = 201;
  int home;
  //SEND CUDA FALSE FOR PERIODIC
  home = int(floor((d - r0)/dr));
  if (home < 0) return 0.0;
  if (home > n) return 0.0;
  
  double homeR = home*dr + r0;
  double w = (d - homeR)/dr;
  double w2 = w*w;
 
  // Interpolate.
  return (3.0*v3[home]*w2 + 2.0*v2[home]*w + v1[home])/dr;
}

__global__
void particleInteraction(int n, Matrix3 basis, Matrix3 basisInv, Vector3* pos, Vector3* force, int nx, int ny, int nz, double dl, double r0, double dr, double* v1, double* v2, double* v3)
{
  //threadIdx.x is the index of the current thread within its block
  //blockDim.x is the number of threads in the block
  //gridDim.x is the number of blocks in the grid
  //blockIdx.x is the index of the current thread block in the grid
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;

  // Particle-particle interactions.
  
  //PiecewiseCubic interactEnergy(arg3, false);
  //BaseGrid sysEnergy(arg1); 
  /*
  for (int i = 0; i < n; i++) 
  {
    for (int j = i+1; j < n; j++) 
    {
    	Vector3 d = sysEnergy.wrapDiff(pos[i] - pos[j]);
    	double dist = d.length();
    	double fMag = -interactEnergy.computeGrad(dist);
    	Vector3 f = fMag/dist*d;
    	force[i] += f;
    	force[j] -= f;
    }
  }
  */
  for(int i = index; i < n; i += stride)
  {
    for(int j = i+1; j < n; j++)
    {
      Vector3 d = cuda_wrapDiff(pos[i] - pos[j], basis, basisInv, nx, ny, nz);
    	double dist = d.length();
    	double fMag = -cuda_computeGrad(dist, dl, r0, dr, n, v1, v2, v3);
    	Vector3 f = fMag/dist*d;
     //SOMETHING TO DO WITH FORCE CAUSING ILLEGAL ACCESS ERRORS
 	force[i] += f;
 /*     	force[j] -= f;
*/
    }
  }
  
  //end kernel
}

int main(int argc, char* argv[]) 
{
  if (argc != 12) 
  {
    printf("Usage: %s energyDxFile diffuseDxFile interactEnergyFile initFile dt kT steps seed0 outputFormat outputPeriod outputPrefix\n",     argv[0]);
    printf("outputFormat can be 'traj' or 'pdb'\n");
    exit(0);
  }
  BaseGrid sysEnergy(argv[1]);  
  BaseGrid sysDiffuse(argv[2]);  
  PiecewiseCubic interactEnergy(argv[3], false);
  Scatter initCoord(argv[4]);
  double dt = strtod(argv[5],NULL);
  double kT = strtod(argv[6],NULL);
  long int steps = atol(argv[7]);
  long int seed0 = atol(argv[8]);
  const char* outputFormat = argv[9];
  int outputPeriod = atoi(argv[10]);
  const char* outputPrefix = argv[argc-1];

  /* GRAB THIS SYSENERGY SHIT BEFORE LOOPING AND THEN SEND TO GPU
    int getNx() const {return nx;}
    int getNy() const {return ny;}
    int getNz() const {return nz;}
    
    GRAB THIS PIECEWISECUBIC SHIT
    double getDl() const { return dl; }
  double getR0() const { return r0; }
  double getDr() const { return dr; }
  */
  
  //begin cuda time
  int cuda_Nx = sysEnergy.getNx();
  int cuda_Ny = sysEnergy.getNy();
  int cuda_Nz = sysEnergy.getNz();
  
  double cuda_Dl = interactEnergy.getDl();
  double cuda_R0 = interactEnergy.getR0();
  double cuda_Dr = interactEnergy.getDr();
  
  Matrix3 host_basis =  sysEnergy.getBasis();
  Matrix3 host_basisInv = host_basis.inverse();
  
  double* cuda_v1 = interactEnergy.getV1();
  double* cuda_v2 = interactEnergy.getV2();
  double* cuda_v3 = interactEnergy.getV3();
  std::cout<<"stiff"<<cuda_v1[200] << std::endl;
  std::cout<<cuda_v2[200] << std::endl;
  std::cout<<cuda_v3[200]<< std::endl;
  std::cout<<"stiff2"<<cuda_v1[202] << std::endl;
  std::cout<<cuda_v2[201] << std::endl;
  std::cout<<cuda_v3[201]<< std::endl;
  //end cuda time
  printf("System 3D energy map: `%s' %d nodes\n", argv[1], sysEnergy.length());
  printf("System 3D diffusivity map: `%s' %d nodes\n", argv[2], sysDiffuse.length());
  printf("Interparticle radial interaction energy: `%s' %d nodes\n", argv[3], interactEnergy.length());
  printf("Initial coordinates: `%s'\n", argv[4]);
  printf("dt %g kT %g steps %ld outputPeriod %d\n", dt, kT, steps, outputPeriod);

  double beta = 1.0/kT;
  //long seed = (unsigned int)time((time_t *)NULL) + seed0*seed0*seed0;
  // REMOVING SEED FOR TESTING: Random rando(seed);
  Random rando(0);

  // Number of particles.
  const int n = initCoord.length();
  Vector3* pos = new Vector3[n];
  Vector3* force = new Vector3[n];
  int* type = new int[n];
  // Initialize positions.
  for (int i = 0; i < n; i++) 
  {
    pos[i] = initCoord.get(i);
    type[i] = 0;
  }
  
  //more cuda
  Vector3* tempForce = NULL;
  Vector3* tempPos = NULL;
    
  cudaMalloc(&tempForce, n*sizeof(Vector3));
  std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;
  cudaMalloc(&tempPos, n*sizeof(Vector3));
  std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;
          
  int numSMs;
  int devId = 0;
  //grab the amount of SMs 
  cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, devId);
  std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;
  int blockSize = 32;
  
  // The names of the particle types (we just have one type right now).
  String* typeName = new String[1];
  typeName[0] = "N";

  // Prepare to write the trajectory.
  TrajectoryWriter writer(outputPrefix, outputFormat, sysEnergy.getBox(), n, dt, outputPeriod, typeName);
  writer.newFile(pos, type, 0.0, n);

  long int s;
  for (s = 1; s <= steps; s++) 
  {
    // Get the force of the environment.
    for (int i = 0; i < n; i++) force[i] = sysEnergy.interpolateForce(pos[i]);

    // Particle-particle interactions.
    cudaMemcpy(tempForce, force, n*sizeof(Vector3), cudaMemcpyHostToDevice);
    std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;
    
    cudaMemcpy(tempPos, pos, n*sizeof(Vector3), cudaMemcpyHostToDevice);
    std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;

    particleInteraction<<<blockSize*numSMs, 512>>>(n, host_basis, host_basisInv, tempPos, tempForce, cuda_Nx, cuda_Ny, cuda_Nz, cuda_Dl, cuda_R0, cuda_Dr, cuda_v1, cuda_v2, cuda_v3 );  
  
  std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;
    // Wait for GPU to finish before accessing o n host
    cudaDeviceSynchronize();
    cudaMemcpy(force, tempForce, n*sizeof(Vector3), cudaMemcpyDeviceToHost );
    std::cout<<cudaGetErrorString(cudaGetLastError()) << std::endl;
    //end particle-particle interactions

    // Update position.
    for (int i = 0; i < n; i++) 
    {
      double diffuse = sysDiffuse.interpolatePotential(pos[i]);
      Vector3 diffGrad = -sysDiffuse.interpolateForce(pos[i]);

      // Get the random kick.
      Vector3 dr = rando.gaussian_vector();

      // Perform the Brownian Dynamics step.
      Vector3 r = pos[i] + beta*force[i]*diffuse*dt + diffGrad*dt + sqrt(2*diffuse*dt)*dr;

      // Wrap boundaries.
      pos[i] = sysEnergy.wrap(r);
    }

    if (s % outputPeriod == 0) 
    {
      fprintf(stdout, "STEP %ld TIME %.15g\n", s, dt*s);
      writer.append(pos, type, dt*s, n);
    }
  }

  delete[] pos;
  delete[] force;
  delete[] type;
  delete[] typeName;

  return 0;
}
/*

//everything ripped from BaseGrid
Matrix3 basis0
basis = basis0;
basisInv = basis.inverse();
nx = abs(nx0);
ny = abs(ny0);
nz = abs(nz0);
    
Vector3 wrapDiff(Vector3 r) const {
    Vector3 l = basisInv.transform(r);
    l.x = wrapDiff(l.x, nx);
    l.y = wrapDiff(l.y, ny);
    l.z = wrapDiff(l.z, nz);
    return basis.transform(l);
  }

static double wrapDiff(double x, double l) {
    int image = int(floor(x/l));
    x -= image*l;
    if (x >= 0.5*l) x -= l;
    return x;
  }

//end BaseGrid ripping

//everything we need from piecewisecubic
double computeGrad(double d) const {
    int home;
    //SEND CUDA FALSE FOR PERIODIC
    if (periodic) {
      d = wrap(d);
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
  
  //rippedfrom Piecewise1d
  double wrap(double x) const {
    return wrapReal(x-r0, dl) + r0;
  }
  
  double getDl() const { return dl; }
  double getR0() const { return r0; }
  double getDr() const { return dr; }
 
  //end piecewise1d rip
  
  
  //ripped from Field
  inline static double wrapReal(double x, double l) {
    int image = int(floor(x/l));
    return x - image*l;
  }
  //end Field rip
  
//end PiecewiseCubic ripping

*/





























