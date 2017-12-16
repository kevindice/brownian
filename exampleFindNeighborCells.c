// This file describes the logic of determining neigbor cells
// Not yet correct: Need to check each cell, doesn't seem to be working for edges


// how the cell index is computed
// thisCellIndex = ((pos[i].x + (sizeX/2))/numCellsX) * numCellsY * numCellsZ + ((pos[i].y + (sizeY/2))/numCellsY) * numCellsZ + ((pos[i].z + (sizeZ/2))/numCellsZ);

int[] findNeighborCells(int thisCellIndex, int numCellsX, int numCellsY, int numCellsZ) {
    int neighborIndexes[27];
    int numCells = numCellsX * numCellsY * numCellsZ;

    // This cell
    neighborIndexes[0] = thisCellIndex;

    // X-direction
    neighborIndexes[1] = (thisCellIndex + numCells - 1) % numCells; // cell to left -- need to check
    neighborIndexes[2] = (thisCellIndex + numCells + 1) % numCells; // cell to right -- not right (should it be numCellsX in the middle?)

    // Y-direction
    neighborIndexes[3] = (thisCellIndex + numCells + (-1 * numCellsZ)) % numCells; // cell above -- need to check
    neighborIndexes[4] = (thisCellIndex + numCells + (1 * numCellsZ)) % numCells; // cell below -- need to check

    // Z-direction
    neighborIndexes[5] = (thisCellIndex + numCells + (-1 * numCellsY * numCellsZ)) % numCells; // cell behind -- need to check
    neighborIndexes[6] = (thisCellIndex + numCells + (1 * numCellsY * numCellsZ)) % numCells; // cell in front -- need to check


// NOT YET IMPLEMENTED/CORRECT:
    // X-Y
    neighborIndexes[7] = (thisCellIndex + numCells - 1 + (-1 * numCellsZ)) % numCells; // cell above and left
    neighborIndexes[8] = (thisCellIndex + numCells + 1 + (-1 * numCellsZ)) % numCells; // cell above and right
    neighborIndexes[9] = (thisCellIndex + numCells - 1 + (1 * numCellsZ)) % numCells; // cell below and left
    neighborIndexes[10] = (thisCellIndex + numCells + 1 + (1 * numCellsZ)) % numCells; // cell below and right

    // X-Z
    neighborIndexes[11] = ; // cell behind and left
    neighborIndexes[12] = ; // cell behind and right
    neighborIndexes[13] = ; // cell in front and left
    neighborIndexes[14] = ; // cell in front and right

    // Y-Z
    neighborIndexes[15] = ; // cell above and behind
    neighborIndexes[16] = ; // cell above and in front
    neighborIndexes[17] = ; // cell below and behind
    neighborIndexes[18] = ; // cell below and in front

    // X-Y-Z
    neighborIndexes[19] = ; // cell above, left, behind
    neighborIndexes[20] = ; // cell above, right, behind
    neighborIndexes[21] = ; // cell above, left, in front
    neighborIndexes[22] = ; // cell above, right, in front

    neighborIndexes[23] = ; // cell below, left, behind
    neighborIndexes[24] = ; // cell below, right, behind
    neighborIndexes[25] = ; // cell below, left, in front
    neighborIndexes[26] = ; // cell below, right, in front

    return neighborIndexes;
}
