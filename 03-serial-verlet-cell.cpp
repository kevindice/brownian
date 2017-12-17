///////////////////////////////////////////////////////////////////////
// Author: Jeff Comer <jeffcomer at gmail>
// Modified by: George Walker <walkerg at ksu dot edu>

#include <ctime>
#include "useful.H"
#include "PiecewiseCubic.H"
#include "Scatter.H"
#include "RandomGsl.H"
#include "BaseGrid.H"
#include "TrajectoryWriter.H"
#include <algorithm>

#define MAX_INTERACTION_RADIUS 12
#define RADIUS_BUFFER_PER_STEP 2
#define VERLET_REBUILD_INT 2

#define SURROUNDING_CELL_COUNT 27


int scalarIndexCompose(int x, int y, int z, int dx, int dy, int dz, int numCellsX, int numCellsY, int numCellsZ) {
    return ((dx + x) % numCellsX) + ((dy + y) % numCellsY) * numCellsX + ((dz + z) % numCellsZ) * numCellsX * numCellsY;
}

int* scalarIndexDecompose(int numCellsX, int numCellsY, int numCellsZ, int thisCellIndex) {
    static int xyz[3];
    int x, y, z;

    x = thisCellIndex % numCellsX;
    y = (((thisCellIndex - x)) / numCellsX) % numCellsY;
    z = (thisCellIndex - y * numCellsX - x) / (numCellsX * numCellsY);

    xyz[0] = x;
    xyz[1] = y;
    xyz[2] = z;
    return xyz;
}


int* findNeighborCells(int thisCellIndex, int numCellsX, int numCellsY, int numCellsZ) {
    static int neighborIndexes[SURROUNDING_CELL_COUNT];

    int neighborIndexesIndex = 0;

    int *xyz;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {

                xyz = scalarIndexDecompose(numCellsX, numCellsY, numCellsZ, thisCellIndex);

                neighborIndexes[neighborIndexesIndex] = scalarIndexCompose(xyz[0], xyz[1], xyz[2], i, j, k, numCellsX, numCellsY, numCellsZ);
                neighborIndexesIndex++;
            }
        }

    }

    return neighborIndexes;
}

int findCellIndex(int x, int y, int z, int numCellsX, int numCellsY, int numCellsZ, int sizeX, int sizeY, int sizeZ) {

    float fx, fy, fz, fnumCellsX, fnumCellsY, fnumCellsZ, fsizeX, fsizeY, fsizeZ;
    fx = (float) x;
    fy = (float) y;
    fz = (float) z;
    fnumCellsX = (float) numCellsX;
    fnumCellsY = (float) numCellsY;
    fnumCellsZ = (float) numCellsZ;
    fsizeX = (float) sizeX;
    fsizeY = (float) sizeY;
    fsizeZ = (float) sizeZ;

    return (int) floor((fx + (fsizeX/2)) / ceil(fsizeX/fnumCellsX)) + floor((fy + (fsizeY/2)) / ceil(fsizeY/fnumCellsY)) * fnumCellsX + floor((fz + (fsizeZ/2)) / ceil(fsizeZ/fnumCellsZ)) * fnumCellsX * fnumCellsY;
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

    printf("System 3D energy map: `%s' %d nodes\n", argv[1], sysEnergy.length());
    printf("System 3D diffusivity map: `%s' %d nodes\n", argv[2], sysDiffuse.length());
    printf("Interparticle radial interaction energy: `%s' %d nodes\n", argv[3], interactEnergy.length());
    printf("Initial coordinates: `%s'\n", argv[4]);
    printf("dt %g kT %g steps %ld outputPeriod %d\n", dt, kT, steps, outputPeriod);

    double beta = 1.0/kT;
    // long seed = (unsigned int)time((time_t *)NULL) + seed0*seed0*seed0;
    // CONSTANT SEED FOR TESTING: Random rando(seed);
    Random rando(0);

    // Number of particles.
    const int n = initCoord.length();
    Vector3* pos = new Vector3[n];
    Vector3* force = new Vector3[n];
    int verlet [n][n];
    int verlet_index [n];
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

    // Figure out min/maxes of simulation space
    double minX = 0;
    double minY = 0;
    double minZ = 0;
    double maxX = 0;
    double maxY = 0;
    double maxZ = 0;
    int sizeX, sizeY, sizeZ;

    for (int i = 0; i < n; i++) {
        if (pos[i].x < minX) {
            minX = pos[i].x;
        } else if (pos[i].x > maxX) {
            maxX = pos[i].x;
        }
        if (pos[i].y < minY) {
            minY = pos[i].y;
        } else if (pos[i].y > maxY) {
            maxY = pos[i].y;
        }
        if (pos[i].z < minZ) {
            minZ = pos[i].z;
        } else if (pos[i].z > maxZ) {
            maxZ = pos[i].z;
        }
    }

    minX = floor(minX);
    minY = floor(minY);
    minZ = floor(minZ);
    maxX = ceil(maxX);
    maxY = ceil(maxY);
    maxZ = ceil(maxZ);

    printf("Simulation Space MIN - x: %f, y: %f, z: %f\n", minX, minY, minZ);
    printf("Simulation Space MAX - x: %f, y: %f, z: %f\n", maxX, maxY, maxZ);

    // Divide up the space and create the cell lists
    sizeX = abs(minX) + abs(maxX);
    sizeY = abs(minY) + abs(maxY);
    sizeZ = abs(minZ) + abs(maxZ);

    printf("Simulation Size - x: %d, y: %d, z: %d\n", sizeX, sizeY, sizeZ);

    double cellSize = MAX_INTERACTION_RADIUS + RADIUS_BUFFER_PER_STEP * (VERLET_REBUILD_INT - 1);

    int numCellsX = ceil(sizeX / cellSize);
    int numCellsY = ceil(sizeY / cellSize);
    int numCellsZ = ceil(sizeZ / cellSize);

    // each cell will be cellSize big, except for the last cell

    printf("Cell Size: %f, NumCellsX: %d, NumCellsY: %d, NumCellsZ: %d\n", cellSize, numCellsX, numCellsY, numCellsZ);

    // Build the Linked List:
    int cellList[n]; // cellList[i] holds the atom index to which the ith atom points.
    int numCells = numCellsX * numCellsY * numCellsZ;
    int head[numCells]; // head[c] holds the index of the first atom in the c-th cell, or head[c] = −1 if there is no atom in the cell.
    int thisCellIndex;

    // Set all heads to -1
    for (int i = 0; i < (numCellsX * numCellsY * numCellsZ); i++) {
        head[i] = -1;
    }

    // Put atoms in linked list
    for (int i = 0; i < n; i++) {
        // Compute the scalar cell index
        thisCellIndex = findCellIndex(pos[i].x, pos[i].y, pos[i].z,numCellsX, numCellsY, numCellsZ, sizeX, sizeY, sizeZ);

        printf("x: %f, y: %f, z: %f, thisCellIndex: %d\n", pos[i].x + (sizeX/2), pos[i].y + (sizeY/2), pos[i].z + (sizeZ/2), thisCellIndex);

        // Link to any other atoms in that cell
        cellList[i] = head[thisCellIndex];

        // Put the last atom in the head of its cell
        head[thisCellIndex] = i;
    }

    long int s;
    int *neighborIndexes;
    neighborIndexes = new int[SURROUNDING_CELL_COUNT];
    for (s = 1; s <= steps; s++) {
        // Get the force of the environment.
        for (int i = 0; i < n; i++) force[i] = sysEnergy.interpolateForce(pos[i]);

        if(s % VERLET_REBUILD_INT == 0) {
            memset(verlet, 0, sizeof(verlet)); // empty the verlet list
            memset(verlet_index, 0, sizeof(verlet_index)); // empty the verlet index
/////////////////////////////////////////////////////////////////////////////////////////////////////////
            for (int i = 0; i < n; i++) { // For each cell
                // Get this atom's cell index
                thisCellIndex = findCellIndex(pos[i].x, pos[i].y, pos[i].z,numCellsX, numCellsY, numCellsZ, sizeX, sizeY, sizeZ);

                // Get the neigbor cells based off of thisCellIndex
                printf("I am cellIndex: %d, my neighbors are: ", thisCellIndex);

                neighborIndexes = findNeighborCells(thisCellIndex, numCellsX, numCellsY, numCellsZ);
                for (int i = 0; i < SURROUNDING_CELL_COUNT; i++) {
                    printf("%d, ",neighborIndexes[i]);
                }
                printf("\n\n\n");

                // set j to the next atom's index
                for (int j = i+1; j < n; j++) {
                    Vector3 d = sysEnergy.wrapDiff(pos[i] - pos[j]); // Set d to the distance of the two atoms
                    double dist = d.length(); // get the size of the distance

                    if(dist <= MAX_INTERACTION_RADIUS) { // If the distance is less than or equal to the MAX_INTERACTION_RADIUS
                        verlet[i][verlet_index[i]] = j; // Put the index of the j cell into this cell's verlet list
                        verlet_index[i]++;
                    }
                }
            }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
        }

        // Particle-particle interactions.
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < verlet_index[i]; j++) {
                Vector3 d = sysEnergy.wrapDiff(pos[i] - pos[verlet[i][j]]);
                double dist = d.length();
                double fMag = -interactEnergy.computeGrad(dist);
                Vector3 f = fMag/dist*d;
                force[i] += f;
                force[verlet[i][j]] -= f;
            }
        }

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

    delete[] pos;
    delete[] force;
    delete[] type;
    delete[] typeName;

    return 0;
}
