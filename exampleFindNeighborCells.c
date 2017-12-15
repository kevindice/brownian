// This file describes the logic of determining neigbor cells

int surrounding[27]; // Array to store indexes of neighbor cells

//   |            In Front            |  |            This Layer          |  |             Behind             |
//   00  01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26

bool edgeX = false;
bool edgeY = false;
bool edgeZ = false;
if ((thisCellIndex - 1) % numCellsX) edgeX = true; // Check if cell is an edge in X dimension
if ((thisCellIndex - 1) % numCellsY) edgeY = true; // Check if cell is an edge in Y dimension
if ((thisCellIndex - 1) % numCellsZ) edgeZ = true; // Check if cell is an edge in Z dimension

surrounding[13] = thisCellIndex;

if !(edgeX) {
    surrounding[12] = thisCellIndex - 1;
    surrounding[14] = thisCellIndex + 1;
}
if !(edgeY) {
    surrounding[10] = thisCellIndex - numCellsX;
    surrounding[16] = thisCellIndex + numCellsX;
}
if !(edgeZ) {
    surrounding[4] = thisCellIndex - (numCellsX * numCellsY);
    surrounding[22] = thisCellIndex + (numCellsX * numCellsY);
}
if !(edgeX) && !(edgeY) {
    surrounding[9] = thisCellIndex - (numCellsX - 1);
    surrounding[11] = thisCellIndex + (numCellsX - 1);
    surrounding[15] = thisCellIndex - (numCellsX + 1);
    surrounding[17] = thisCellIndex + (numCellsX + 1);
}
if !(edgeX) && !(edgeZ) {
    surrounding[3] = thisCellIndex - (numCellsX * numCellsY) - 1;
    surrounding[5] = thisCellIndex - (numCellsX * numCellsY) + 1;
    surrounding[21] = thisCellIndex + (numCellsX * numCellsY) - 1;
    surrounding[23] = thisCellIndex + (numCellsX * numCellsY) + 1;
}
if !(edgeY) && !(edgeZ) {
    surrounding[1] = thisCellIndex - numCellsX - (numCellsX * numCellsY);
    surrounding[7] = thisCellIndex + numCellsX - (numCellsX * numCellsY);
    surrounding[19] = thisCellIndex - numCellsX + (numCellsX * numCellsY);
    surrounding[25] = thisCellIndex + numCellsX + (numCellsX * numCellsY);
}
if !(edgeX) && !(edgeY) && !(edgeZ) {
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
