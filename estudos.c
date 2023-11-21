#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "wfc.h"

MPI_Datatype createCellType(int length_totalEntropy) {

    MPI_Datatype cellType;
    int blocklengths[3] = {length_totalEntropy, 1, 1};
    MPI_Aint offsets[3];
    MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_SHORT, MPI_INT};

    offsets[0] = offsetof(Cell, options);
    offsets[1] = offsetof(Cell, collapsedValue);
    offsets[2] = offsetof(Cell, totalEntropy);

    MPI_Type_create_struct(3, blocklengths, offsets, types, &cellType);
    MPI_Type_commit(&cellType);

    return cellType;
}

void findLowestEntropy(World world, Tileset tileset, int *lowXResult, int *lowYResult)
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
    *lowXResult = lowX;
    *lowYResult = lowY;
}

typedef struct {
    int value;
    int location;
} MaxLoc;


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Número de elementos por processo
    const int elements_per_process = 5;

    // Array local em cada processo
    int local_data[elements_per_process];
    for (int i = 0; i < elements_per_process; ++i) {
        local_data[i] = (rank + 1) * 10 - i; // Dados de exemplo
        
        //printf("%d ", local_data[i]);
    }
    printf("\n");
    // Encontrar o menor valor local e seu índice
    int min_value = INT_MAX;
    int min_index = -1;

    for (int i = 0; i < elements_per_process; ++i) {
        if (local_data[i] < min_value) {
            min_value = local_data[i];
            min_index = i + rank * elements_per_process; // Ajusta o índice para levar em conta o deslocamento
        }
    }
    int a[] = {min_index, min_value};
    int global_max[2];
    MPI_Reduce(a, global_max, 1, MPI_2INT, MPI_MINLOC, 0, MPI_COMM_WORLD);
    // Somente o processo raiz imprime o resultado
    if (rank == 0) {
        printf("min global: %d na localização: %d\n", global_max[1], global_max[0]);
    }
    /*
    // Variáveis para armazenar o menor valor global e seu índice
    int global_min_value;
    int global_min_index;

    // Reduce para encontrar o menor valor global
    MPI_Reduce(&min_value, &global_min_value, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&min_index, &global_min_index, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    // Somente o processo raiz imprime o resultado
    if (rank == 0) {
        printf("Menor valor global: %d no índice: %d\n", global_min_value, global_min_index);
    }
    */

    MPI_Finalize();

    return 0;
}
