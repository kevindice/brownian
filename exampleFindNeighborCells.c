// This file describes the logic of determining neigbor cells
// Not yet correct: Need to check each cell, doesn't seem to be working for edges


// how the cell index is computed
// thisCellIndex = ((pos[i].x + (sizeX/2))/numCellsX) * numCellsY * numCellsZ + ((pos[i].y + (sizeY/2))/numCellsY) * numCellsZ + ((pos[i].z + (sizeZ/2))/numCellsZ);


int scalarIndexCompose(int x, int y, int z, int dx, int dy, int dz, int numCellsX, int numCellsY, int numCellsZ) {
    return ((dx + x) mod numCellsX) + ((dy + y) mod numCellsY) * numCellsX + ((dz + z) mod numCellsZ) * numCellsX * numCellsY;
}

int[] scalarIndexDecompose(int numCellsX, int numCellsY, int numCellsZ, int thisCellIndex) {
    int xyz[3], x, y, z;

    x = thisCellIndex mod numCellsX;
    y = (((thisCellIndex - x)) / numCellsX) mod numCellsY;
    z = (thisCellIndex - y * numCellsX - x) / (numCellsX * numCellsY);

    xyz[0] = x;
    xyz[1] = y;
    xyz[3] = z;
    return xyz;
}


int[] findNeighborCells(int thisCellIndex, int numCellsX, int numCellsY, int numCellsZ) {
    int neighborIndexes[27];

    int neighborIndexesIndex = 0;

    int xyz[3];

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
