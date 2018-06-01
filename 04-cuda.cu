///////////////////////////////////////////////////////////////////////
// Author: Jeff Comer <jeffcomer at gmail>

#include <ctime>
#include "useful.H"
#include "PiecewiseCubic.H"
#include "Scatter.H"
#include "RandomGsl.H"
#include "BaseGrid.H"
#include "TrajectoryWriter.H"
#include "Pure.H"

#define TBP 32

__global__
void hello(char *a, int *b)
{
  a[threadIdx.x] += b[threadIdx.x];
}

int main(int argc, char* argv[]) {
  if (argc != 12) {
    printf("Usage: %s energyDxFile diffuseDxFile interactEnergyFile initFile dt kT steps seed0 outputFormat outputPeriod outputPrefix\n", argv[0]);
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
  //long int seed0 = atol(argv[8]);
  const char* outputFormat = argv[9];
  int outputPeriod = atoi(argv[10]);
  const char* outputPrefix = argv[argc-1];
  const int stripSize = 16;

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
  for (int i = 0; i < n; i++) {
    pos[i] = initCoord.get(i);
    type[i] = 0;
  }

  // The names of the particle types (we just have one type right now).
  String* typeName = new String[1];
  typeName[0] = "N";

  // Prepare to write the trajectory.
  TrajectoryWriter writer(outputPrefix, outputFormat, sysEnergy.getBox(), n, dt, outputPeriod, typeName);
  writer.newFile(pos, type, 0.0, n);

  // Parameters to make functions pure
  const Matrix3 basis = sysEnergy.getBasis();
  const Matrix3 basisInv = sysEnergy.getInverseBasis();
  const int nx = sysEnergy.getNx();
  const int ny = sysEnergy.getNy();
  const int nz = sysEnergy.getNz();

  const bool periodic = interactEnergy.getPeriodic();
  const double r0 = interactEnergy.getR0();
  const double dl = interactEnergy.getDl();
  const double dr = interactEnergy.getDr();
  const int interact_n = interactEnergy.length();
  double* v1 = interactEnergy.getV1();
  double* v2 = interactEnergy.getV2();
  double* v3 = interactEnergy.getV3();
  // Parameters to make functions pure


  // Cuda hello world
  char a[16] = "Hello \0\0\0\0\0\0";
  int b[16] = {15, 10, 6, 0, -11, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char *ad;
  int *bd;
  const int csize = 16*sizeof(char);
  const int isize = 16*sizeof(int);
  printf("%s cake\n", a);

  cudaMalloc( (void**)&ad, csize );
  cudaMalloc( (void**)&bd, isize );
  cudaMemcpy( ad, a, csize, cudaMemcpyHostToDevice );
  cudaMemcpy( bd, b, isize, cudaMemcpyHostToDevice );

  hello<<<1, 16>>>(ad, bd);
  cudaMemcpy( a, ad, csize, cudaMemcpyDeviceToHost );
  cudaFree( ad );
  cudaFree( bd );
  printf("%s pizza\n", a);
  // Cuda hello world

  printf("Number of particles: %d\n", n);



  // Start cuda setup
  int numSMs;
  cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, 0);
  // end cuda setup


  long int s;
  for (s = 1; s <= steps; s++) {
    // Get the force of the environment.
    for (int i = 0; i < n; i++) force[i] = sysEnergy.interpolateForce(pos[i]);

    // Particle-particle interactions.
    dim3 grid(numSMs, 32, 1);
    dim3 block(16, 16, 1);
    doComputeCuda<<<grid,block>>>(s, n);
    cudaDeviceSynchronize();

    doCompute(
            force,
            pos,
            basis,
            basisInv,
            nx,
            ny,
            nz,
            periodic,
            r0,
            dl,
            dr,
            interact_n,
            v1,
            v2,
            v3,
            n
    );


    // Update position.
    for (int i = 0; i < n; i++) {
      double diffuse = sysDiffuse.interpolatePotential(pos[i]);
      Vector3 diffGrad = -sysDiffuse.interpolateForce(pos[i]);

      // Get the random kick.
      Vector3 dr = rando.gaussian_vector();

      // Perform the Brownian Dynamics step.
      Vector3 r = pos[i] + beta*force[i]*diffuse*dt + diffGrad*dt + sqrt(2*diffuse*dt)*dr;

      // Wrap boundaries.
      pos[i] = sysEnergy.wrap(r);
    }

    if (s % outputPeriod == 0) {
      fprintf(stdout, "STEP %ld TIME %.15g\n", s, dt*s);
      writer.append(pos, type, dt*s, n);
    }
  }

  // start cuda teardown
  cudaDeviceSynchronize();
  // end cuda teardown

  printf("Number of SMs: %d\n", numSMs);

  delete[] pos;
  delete[] force;
  delete[] type;
  delete[] typeName;

  return 0;
}
