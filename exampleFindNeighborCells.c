// This file describes the logic of determining neigbor cells

int surrounding[27]; // Array to store indexes of neighbor cells

//   |            In Front            |  |            This Layer          |  |             Behind             |
//   00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26

// edge check may not be accurate
int posX, posY, posZ;
posX = thisCellIndex % numCellsX; // Check if cell is an edge in X dimension (position "0" or "numCellsX - 1" horizontally)

posY = ; // Check if cell is an edge in Y dimension (position "0" or "numCellsY - 1" vertically)
posZ = ; // Check if cell is an edge in Z dimension (position "0" or "numCellsZ - 1" z-direction)

surrounding[13] = thisCellIndex;

if (posX == 0) { // If the cell is on left edge in the X dimension
    surrounding[12] = thisCellIndex + (numCellsX - 1);
    surrounding[14] = thisCellIndex + 1;
} else if (posX == (numCellsX - 1)) { // If the cell is on right edge in the X dimension
    surrounding[12] = thisCellIndex - 1;
    surrounding[14] = thisCellIndex - (numCellsX - 1);
} else { // If the cell is not on an edge in the X dimension
    surrounding[12] = thisCellIndex - 1;
    surrounding[14] = thisCellIndex + 1;
}

if (posY == 0) {
} else if (posY == (numCellsY - 1)) { // If the cell is on an edge in the Y dimension
} else { // If the cell is not on an edge in the Y dimension
    surrounding[10] = thisCellIndex - numCellsY;
    surrounding[16] = thisCellIndex + numCellsY;
}

if (posZ == 0) {

} else if (posZ == (numCellsZ - 1)) { // If the cell is on an edge in the Z dimension

} else { // If the cell is not on an edge in the Z dimension
    surrounding[4] = thisCellIndex - (numCellsX * numCellsY);
    surrounding[22] = thisCellIndex + (numCellsX * numCellsY);
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
