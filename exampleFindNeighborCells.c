// This file describes the logic of determining neigbor cells

int surrounding[27]; // Array to store indexes of neighbor cells

//   |            In Front            |  |            This Layer          |  |             Behind             |
//   00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26

// X
// (thisCellIndex + numCells + (d)) % numCells

// Y
// (thisCellIndex + numCells + (d * numCellsZ)) % numCells

// Z
// (thisCellIndex + numCells + (d * numCellsY * numCellsZ)) % numCells

// General
// (thisCellIndex + numCells + (dz * numCellsY * numCellsZ) + (dy * numCellsZ) + (dx)) % numCells


(thisCellIndex + numCells + dx + (dy * numCellsZ) + (dz * numCellsY * numCellsZ)) % numCells


// Compute the scalar cell index
thisCellIndex = ((pos[i].x + (sizeX/2))/numCellsX) * numCellsY * numCellsZ + ((pos[i].y + (sizeY/2))/numCellsY) * numCellsZ + ((pos[i].z + (sizeZ/2))/numCellsZ);

// Works with edges
// X-direction
surrounding[13] = thisCellIndex; // this cell
surrounding[12] = (thisCellIndex + numCells + (-1 * numCellsY * numCellsZ)) % numCells; // cell to left
surrounding[14] = (thisCellIndex + numCells + (1 * numCellsY * numCellsZ)) % numCells; // cell to right

//
surrounding[10] = (thisCellIndex + numCells + (-1 * numCellsZ)) % numCells; // cell above
surrounding[16] = (thisCellIndex + numCells + (1 * numCellsZ)) % numCells; // cell below

urrounding[4] = thisCellIndex - (numCellsX * numCellsY); // cell in front of
surrounding[22] = thisCellIndex + (numCellsX * numCellsY); // cell behind






if (posZ == 0) {

} else if (posZ == (numCellsZ - 1)) { // If the cell is on an edge in the Z dimension

} else { // If the cell is not on an edge in the Z dimension
    s
}

if (edgeX == 0) && (edgeY == 0) {
    surrounding[9] = thisCellIndex - (numCellsX - 1);
    surrounding[11] = thisCellIndex + (numCellsX - 1);
    surrounding[15] = thisCellIndex - (numCellsX + 1);
    surrounding[17] = thisCellIndex + (numCellsX + 1);
}
if (edgeX == 0) && (edgeZ == 0) {
    surrounding[3] = thisCellIndex - (numCellsX * numCellsY) - 1;
    surrounding[5] = thisCellIndex - (numCellsX * numCellsY) + 1;
    surrounding[21] = thisCellIndex + (numCellsX * numCellsY) - 1;
    surrounding[23] = thisCellIndex + (numCellsX * numCellsY) + 1;
}
if (edgeY == 0) && (edgeZ == 0) {
    surrounding[1] = thisCellIndex - numCellsX - (numCellsX * numCellsY);
    surrounding[7] = thisCellIndex + numCellsX - (numCellsX * numCellsY);
    surrounding[19] = thisCellIndex - numCellsX + (numCellsX * numCellsY);
    surrounding[25] = thisCellIndex + numCellsX + (numCellsX * numCellsY);
}
if (edgeX == 0) && (edgeY == 0) && (edgeZ == 0) {
    surrounding[0] = thisCellIndex - numCellsX - (numCellsX * numCellsY) - 1;
    surrounding[2] = thisCellIndex - numCellsX - (numCellsX * numCellsY) + 1;
    surrounding[6] = thisCellIndex + numCellsX - (numCellsX * numCellsY) - 1;
    surrounding[8] = thisCellIndex + numCellsX - (numCellsX * numCellsY) + 1;

    surrounding[18] = thisCellIndex - numCellsX + (numCellsX * numCellsY) - 1;
    surrounding[20] = thisCellIndex - numCellsX + (numCellsX * numCellsY) + 1;
    surrounding[24] = thisCellIndex + numCellsX + (numCellsX * numCellsY) - 1;
    surrounding[26] = thisCellIndex + numCellsX + (numCellsX * numCellsY) + 1;
}

// We now have an array of indexes for the neigboring cells, assuming none are edges.
