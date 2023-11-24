
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#include "pgm.h"
#include "wfc.h"
#include "tile.h"

#include "cJSON.h"

#define MAX_SIZE_CELL 5
#define QTD_RESULT 1
#define REPEAT 1

int get_height_world_by_rank(){
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_process = MAX_SIZE_CELL / size;
    int remaining_rows = MAX_SIZE_CELL % size;

    if(rank == size-1){
        rows_per_process += remaining_rows;
    }

    return rows_per_process;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank);

    double start_time, end_time;

    int increment = (MAX_SIZE_CELL)/QTD_RESULT;
    Tileset t = open_tileset("tileset/maze 32.txt");

    cJSON *rootArray = cJSON_CreateArray();
    for(int i = MAX_SIZE_CELL; i > 0; i-= increment)
    {
        cJSON *elementObject = cJSON_CreateObject();
        cJSON *resultArray = cJSON_CreateArray();

        cJSON_AddNumberToObject(elementObject, "cell_height", i);
        cJSON_AddNumberToObject(elementObject, "cell_width", i);

        cJSON *resultItem = cJSON_CreateObject();
        cJSON *timeArray = cJSON_CreateArray();
        cJSON_AddNumberToObject(resultItem, "processos", size);


        for(int k = 0; k < REPEAT; k++)
        {
            int height = get_height_world_by_rank();
            World w = new_world(height, i, t->qtd);
            //------------------------------------------------
            start_time = MPI_Wtime();
            waveFuctionCollapse(t, w);
            end_time = MPI_Wtime();
            //------------------------------------------------
            Pgm p = convertWfc(w, t);
            char *name = malloc(sizeof(char) * 100);
            sprintf(name, "result/imagem/wfc(C-%dx%d)(%d)(R-%d).pgm", i, i, rank, k); //Imagens Separadas
            //sprintf(name, "result/imagem/wfc(C-%dx%d)(R-%d).pgm", i, i, k);// Uma Imagem
            pgm_files(name, p);
            free(name);
            free_pgm(p);

            double total_time = end_time - start_time;
            double global_start_time, global_end_time;
            MPI_Allreduce(&start_time, &global_start_time, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
            MPI_Allreduce(&end_time, &global_end_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
            printf("Cell(%dx%d)\ntentativa:%d\ntempo:%lf\n\n",w->width,w->height,k, total_time);
            total_time = global_end_time - global_start_time;
            free_world(w);

            cJSON *time = cJSON_CreateNumber(total_time);
            cJSON_AddItemToArray(timeArray, time);
        }

        cJSON_AddItemToObject(resultItem, "time", timeArray);
        cJSON_AddItemToArray(resultArray, resultItem);

        cJSON_AddItemToObject(elementObject, "result", resultArray);
        cJSON_AddItemToArray(rootArray, elementObject);
    }
    free_tileset(t);

    /*
    if(rank == 0){
        char *json_string = cJSON_Print(rootArray);
        char *name = malloc(sizeof(char) * 100);
        sprintf(name, "result/dados(processos %d).json", size);
        FILE *arquivo = fopen(name, "w");
        if (arquivo != NULL) {
            fprintf(arquivo, json_string);
            fclose(arquivo);
        }
        free(name);
        free(json_string);
    }
    */
    cJSON_Delete(rootArray);

    //system("python graficos.py");
    MPI_Finalize();
    return 0;
}

