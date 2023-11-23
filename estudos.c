#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "pgm.h"
#include "wfc.h"
#include "tile.h"

#include "cJSON.h"



void findLowestEntropy_MPI(World world, Tileset tileset, int *lowXResult, int *lowYResult)
{
    int lowEntropy = tileset->qtd;
    int lowX = 0;
    int lowY = 0;
    int const height = world->height;
    int const width = world->width;


    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Calcula quantas linhas cada processo receberá
    int rows_per_process = height / size;
    int remaining_rows = height % size;

    // Distribui as linhas da matriz entre os processos
    int local_rows = (rank < remaining_rows) ? (rows_per_process + 1) : rows_per_process;

    int initial_row = local_rows * rank;
    int final_row = local_rows * (rank+1);
    for (int y = initial_row; y < final_row; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if(world->map[y][x].collapsedValue == -1 &&
            world->map[y][x].totalEntropy < lowEntropy &&
            world->map[y][x].totalEntropy != 0){
                lowEntropy = world->map[y][x].totalEntropy;
                lowX = x;
                lowY = y;
            }
        }
    }
    printf("%d %d %d\n", lowEntropy, lowX, lowY);

    int global_lowEntropy;
    MPI_Reduce(&lowEntropy, &global_lowEntropy, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&lowX, &lowXResult, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&lowY, &lowYResult, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

}

/*
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    Tileset t = open_tileset("tileset/short line.txt");
    World w = new_world(15, 15, t->qtd);
    int a,b;
    findLowestEntropy_MPI(w, t, &a, &b);
    MPI_Finalize();
    return 0;
}
*/
