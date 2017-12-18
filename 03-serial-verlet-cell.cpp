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

    double cellSize = MAX_INTERACTION_RADIUS + RADIUS_BUFFER_PER_STEP * (VERLET_REBUILD_INT - 1);

    Vector3 simulationSize = sysEnergy.getDestination();

    double simSizeX = simulationSize.x * 2;
    double simSizeY = simulationSize.y * 2;
    double simSizeZ = simulationSize.z * 2;

    // printf("Simulation size: %f, %f, %f\n", simSizeX, simSizeY, simSizeZ);

    int numCellsX = ceil(simSizeX / cellSize);
    int numCellsY = ceil(simSizeY / cellSize);
    int numCellsZ = ceil(simSizeZ / cellSize);

    int numCells = numCellsX * numCellsY * numCellsZ;

    // each cell will be cellSize big, except for the last cell

    // printf("Cell Size: %f, NumCellsX: %d, NumCellsY: %d, NumCellsZ: %d\n", cellSize, numCellsX, numCellsY, numCellsZ);

    long int s;

    int dx, dy, dz, x, y, z, currentNeighborIndex, currentAtomInMyCell, currentAtomInNeighborCell;
    int cellList[n]; // cellList[i] holds the atom index to which the ith atom points.
    int head[numCells]; // head[c] holds the index of the first atom in the c-th cell, or head[c] = −1 if there is no atom in the cell.
    int thisCellIndex;

    for (s = 1; s <= steps; s++) {

        // During each timestep rebuild the Linked List:
        // Set all heads to -1
        for (int i = 0; i < numCells; i++) {
            head[i] = -1;
        }

        thisCellIndex = 0;
        // printf("Binning particles into cells: \n");
        // Put atoms in linked list
        for (int i = 0; i < n; i++) {
            // printf("%d: ", i);
            // Compute the scalar cell index
            thisCellIndex = findCellIndex(pos[i].x, pos[i].y, pos[i].z, numCellsX, numCellsY, numCellsZ, simSizeX, simSizeY, simSizeZ);

            // printf("x: %f, y: %f, z: %f, thisCellIndex: %d\n", pos[i].x + (simSizeX/2), pos[i].y + (simSizeY/2), pos[i].z + (simSizeZ/2), thisCellIndex);

            // Link to any other atoms in that cell
            cellList[i] = head[thisCellIndex];

            // Put the last atom in the head of its cell
            head[thisCellIndex] = i;
        }


        // Get the force of the environment.
        for (int i = 0; i < n; i++) force[i] = sysEnergy.interpolateForce(pos[i]);

        for (int i = 0; i < numCells; i++) { // For each cell

            // If this cell contains no atoms, skip
            if (head[i] == -1) {
                // printf("Skipping cell %d as it is empty\n", i);
                continue;
            }

            // printf("\n\nCurrent cell: %d\n", i);

            // Get the neighbors for this calculation
            for (int n1 = 0; n1 < 14; n1++) {

                x = i % numCellsX;
                y = (((i - x)) / numCellsX) % numCellsY;
                z = (i - y * numCellsX - x) / (numCellsX * numCellsY);

                // if (n == 0) printf(" (%d, %d, %d)\n", x, y, z);

                dx = n1/9 % 3 - 1;
                dy = n1/3 % 3 - 1;
                dz = n1 % 3 - 1;

                // printf("%d, %d, %d\n", dx, dy, dz);

                currentNeighborIndex = ((dx + x + numCellsX) % numCellsX) + ((dy + y + numCellsY) % numCellsY) * numCellsX + ((dz + z + numCellsZ) % numCellsZ) * numCellsX * numCellsY;

                // printf("Calculating interaction with neighbor cell: %d\n", currentNeighborIndex);

                // If there are no atoms in the current neighbor cell, skip
                if (head[currentNeighborIndex] == -1) {
                    // printf("Skipping cell %d as it is empty\n", currentNeighborIndex);
                    continue;
                }

                currentAtomInMyCell = head[i];
                currentAtomInNeighborCell = head[currentNeighborIndex];

                while (cellList[currentAtomInMyCell] != -1) { // While there are still atoms in this cell
                    while (cellList[currentAtomInNeighborCell] != -1) { // While there are still atoms in the neighbor cell

                        // Don't interact particle with itself
                        if (cellList[currentAtomInMyCell] == cellList[currentAtomInNeighborCell]) {
                            currentAtomInNeighborCell = cellList[currentAtomInNeighborCell];
                            continue;
                        }
                        // printf("Calculating interaction between atom %d and %d\n", currentAtomInMyCell, currentAtomInNeighborCell);
                        // Do stuff
                        Vector3 d = sysEnergy.wrapDiff(pos[currentAtomInMyCell] - pos[currentAtomInNeighborCell]);
                        // printf("d: (%f, %f, %f)\n", d.x, d.y, d.z);
                        double dist = d.length();
                        // printf("dist: %f\n", dist);
                        double fMag = -interactEnergy.computeGrad(dist);
                        // printf("fMag: %f\n", fMag);
                        Vector3 f = fMag/dist*d;
                        // printf("f: (%f, %f, %f)\n", f.x, f.y, f.z);
                        // printf("force[%i] (me) before: (%f, %f, %f), after: (%f, %f, %f)\n",currentAtomInMyCell, force[currentAtomInMyCell].x, force[currentAtomInMyCell].y, force[currentAtomInMyCell].z, force[currentAtomInMyCell].x + f.x, force[currentAtomInMyCell].y + f.y, force[currentAtomInMyCell].z + f.z);
                        force[currentAtomInMyCell] += f;
                        // printf("force[%i] (neigbor) before: (%f, %f, %f), after: (%f, %f, %f)\n",currentAtomInNeighborCell, force[currentAtomInNeighborCell].x, force[currentAtomInNeighborCell].y, force[currentAtomInNeighborCell].z, force[currentAtomInNeighborCell].x - f.x, force[currentAtomInNeighborCell].y - f.y, force[currentAtomInNeighborCell].z - f.z);
                        force[currentAtomInNeighborCell] -= f;


                        // Go to next atom in neighbor cell
                        currentAtomInNeighborCell = cellList[currentAtomInNeighborCell];
                    }
                    // Go to next atom in my cell
                    currentAtomInMyCell = cellList[currentAtomInMyCell];
                }
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
