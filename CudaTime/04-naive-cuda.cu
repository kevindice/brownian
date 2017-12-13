///////////////////////////////////////////////////////////////////////
// Author: Jeff Comer <jeffcomer at gmail>

#include <ctime>
#include "useful.H"
#include "PiecewiseCubic.H"
#include "Scatter.H"
#include "RandomGsl.H"
#include "BaseGrid.H"
#include "TrajectoryWriter.H"
#include <math.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
 
__global__
void particleInteraction(int n, Vector3* pos, Vector3* force, char* arg1, char* arg3)
{
  //threadIdx.x is the index of the current thread within its block
  //blockDim.x is the number of threads in the block
  //gridDim.x is the number of blocks in the grid
  //blockIdx.x is the index of the current thread block in the grid
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  int stride = blockDim.x * gridDim.x;
  /*
  for (int i = index; i < n; i += stride)
      y[i] = x[i] + y[i];
    */  
  // Particle-particle interactions.
  PiecewiseCubic interactEnergy(arg3, false);
  BaseGrid sysEnergy(arg1); 
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
      Vector3 d = sysEnergy.wrapDiff(pos[i] - pos[j]);
    	double dist = d.length();
    	double fMag = -interactEnergy.computeGrad(dist);
    	Vector3 f = fMag/dist*d;
    	force[i] += f;
    	force[j] -= f;
    }
  }
  
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
    size_t pitch;
    fprintf(stdout, "YeaP: ", sizeof(Vector3) );
    int numSMs;
    int devId = 0;
    //grab the amount of SMs 
    cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, devId);
    int blockSize = 32;
    
    //test
    //__device__ PiecewiseCubic interactPointer(argv[3], false);
    //PiecewiseCubic hostInteract;
   // cudaMemcpyFromSymbol(&hostInteract, interactPointer, sizeof(PiecewiseCubic) );
    //device_vector<double> pT = pos;
   //device_vector<double> fT = force;
   // double* pPoint = thrust::raw_pointer_cast (pT.data() );
   // double* fPoint = thrust::raw_pointer_cast (fT.data() );
    //test
    /*
    cudaMallocPitch(&pos, &pitch, n * sizeof(double) * 3, N);
    cudaMallocPitch(&force, &pitch, n * sizeof(double) * 3, N);
    */
    //interactEnergy
    //(int n, Vector3* pos, Vector3* force)
    Vector3* tempForce = new Vector3[n];
    Vector3* tempPos = new Vector3[n];
    
    cudaMalloc(&tempForce, n*sizeof(Vector3));
    cudaMemcpy(tempForce, force, n*sizeof(Vector3), cudaMemcpyHostToDevice);
    cudaMalloc(&tempPos, n*sizeof(Vector3));
    cudaMemcpy(tempPos, pos, n*sizeof(Vector3), cudaMemcpyHostToDevice);
    particleInteraction<<<blockSize*numSMs, 256>>>(n, pos, force, argv[1], argv[3]);
    
    // Wait for GPU to finish before accessing on host
    cudaDeviceSynchronize();
    
    cudaMemcpy(force, tempForce, n*sizeof(Vector3), cudaMemcpyDeviceToHost);
    /*
    cudaFree
    cudaFree
    */
    //end particle to paticle interactions
    
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
