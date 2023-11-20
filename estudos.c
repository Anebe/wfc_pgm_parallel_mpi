#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
void findLowestEntropy(Cell **cell, int h_cell, int w_cell, int qtd_tileset, int *lowXResult, int *lowYResult)
{
    int lowEntropy = qtd_tileset;
    int lowX = 0;
    int lowY = 0;
    int const height = h_cell;
    int const width = w_cell;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if(cell[y][x].collapsedValue == -1 &&
            cell[y][x].totalEntropy < lowEntropy &&
            cell[y][x].totalEntropy != 0){
                lowEntropy = cell[y][x].totalEntropy;
                lowX = x;
                lowY = y;
            }
        }
    }
}
*/
void a()
{
    MPI_Init(NULL, NULL);
    double start_time = MPI_Wtime();
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int total_elements = 40;
    int elements_per_process = total_elements / size;

    // Cria uma matriz no processo 0
    int *global_array = NULL;
    if (rank == 0) {
        global_array = (int *)malloc(sizeof(int) * total_elements);
        for (int i = 0; i < total_elements; i++) {
            global_array[i] = i;
        }
    }

    // Cria um buffer local para armazenar a parte distribuída
    int *local_array = (int *)malloc(sizeof(int) * elements_per_process);

    // Distribui a matriz entre os processos
    MPI_Scatter(global_array, elements_per_process, MPI_INT,
                local_array, elements_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    // Cada processo imprime a parte distribuída
    printf("Processo %d: ", rank);
    for (int i = 0; i < elements_per_process; i++) {
        printf("%d ", local_array[i]);
    }
    printf("\n");

    // Cada processo pode realizar cálculos independentes em sua parte da matriz

    // Coleta os resultados de volta para o processo 0
    MPI_Gather(local_array, elements_per_process, MPI_INT,
               global_array, elements_per_process, MPI_INT, 0, MPI_COMM_WORLD);

    // O processo 0 imprime a matriz global
    if (rank == 0) {
        printf("Matriz global após a coleta: ");
        for (int i = 0; i < total_elements; i++) {
            printf("%d ", global_array[i]);
        }
        printf("\n");
    }

    // Libera a memória alocada
    free(global_array);
    free(local_array);

    printf("tempo mpi %lf", MPI_Wtime() - start_time);
    MPI_Finalize();
}
int main(int argc, char** argv) {
    a();
}