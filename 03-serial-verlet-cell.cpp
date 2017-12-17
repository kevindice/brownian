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

#define MAX_INTERACTION_RADIUS 12
#define RADIUS_BUFFER_PER_STEP 2
#define VERLET_REBUILD_INT 2

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

    // printf("Simulation Space MIN - x: %f, y: %f, z: %f\n", minX, minY, minZ);
    // printf("Simulation Space MAX - x: %f, y: %f, z: %f\n", maxX, maxY, maxZ);

    // Divide up the space and create the cell lists

    // printf("Simulation Size - x: %d, y: %d, z: %d\n", sysEnergy.getNx(), sysEnergy.getNy(), sysEnergy.getNz());

    double cellSize = MAX_INTERACTION_RADIUS + RADIUS_BUFFER_PER_STEP * (VERLET_REBUILD_INT - 1);

    int numCellsX = ceil(sysEnergy.getNx() / cellSize);
    int numCellsY = ceil(sysEnergy.getNy() / cellSize);
    int numCellsZ = ceil(sysEnergy.getNz() / cellSize);

    // each cell will be cellSize big, except for the last cell

    // printf("Cell Size: %f, NumCellsX: %d, NumCellsY: %d, NumCellsZ: %d\n", cellSize, numCellsX, numCellsY, numCellsZ);

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
        thisCellIndex = findCellIndex(pos[i].x, pos[i].y, pos[i].z,numCellsX, numCellsY, numCellsZ, sysEnergy.getNx(), sysEnergy.getNy(), sysEnergy.getNz());

        //printf("x: %f, y: %f, z: %f, thisCellIndex: %d\n", pos[i].x + (sysEnergy.getNx()/2), pos[i].y + (sysEnergy.getNy()/2), pos[i].z + (sysEnergy.getNz()/2), thisCellIndex);

        // Link to any other atoms in that cell
        cellList[i] = head[thisCellIndex];

        // Put the last atom in the head of its cell
        head[thisCellIndex] = i;
    }

    long int s;

    int dx, dy, dz, x, y, z, currentNeighborIndex, currentAtomInMyCell, currentAtomInNeigborCell;
    for (s = 1; s <= steps; s++) {
        // Get the force of the environment.
        for (int i = 0; i < n; i++) force[i] = sysEnergy.interpolateForce(pos[i]);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
        for (int i = 0; i < numCells; i++) { // For each cell

            // If this cell contains no atoms, skip
            if (head[i] == -1) {
                continue;
            }

            // Get the neighbors for this calculation
            for (int n = 0; n < 14; n++) {

                dx = n/9 % 3 - 1;
                dy = n/3 % 3 - 1;
                dz = n % 3 - 1;

                //printf("%d, %d, %d\n", dx, dy, dz);

                x = i % numCellsX;
                y = (((i - x)) / numCellsX) % numCellsY;
                z = (i - y * numCellsX - x) / (numCellsX * numCellsY);

                currentNeighborIndex = ((dx + x + numCellsX) % numCellsX) + ((dy + y + numCellsY) % numCellsY) * numCellsX + ((dz + z + numCellsZ) % numCellsZ) * numCellsX * numCellsY;

                // If there are no atoms in the current neighbor cell, skip
                if (head[currentNeighborIndex] == -1) {
                    continue;
                }

                currentAtomInMyCell = head[i];
                currentAtomInNeigborCell = head[currentNeighborIndex];

                while (cellList[currentAtomInMyCell] != -1) { // While there are still atoms in this cell
                    while (cellList[currentAtomInNeigborCell] != -1) { // While there are still atoms in the neighbor cell
                        // Do stuff
                        Vector3 d = sysEnergy.wrapDiff(pos[currentAtomInMyCell] - pos[currentAtomInNeigborCell]);
                        double dist = d.length();
                        double fMag = -interactEnergy.computeGrad(dist);
                        Vector3 f = fMag/dist*d;
                        force[currentAtomInMyCell] += f;
                        force[currentAtomInNeigborCell] -= f;

                        // Go to next atom in neighbor cell
                        currentAtomInNeigborCell = cellList[currentAtomInNeigborCell];
                    }
                    // Go to next atom in my cell
                    currentAtomInMyCell = cellList[currentAtomInMyCell];
                }
            }
        }
/////////////////////////////////////////////////////////////////////////////////////////////////////////

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
