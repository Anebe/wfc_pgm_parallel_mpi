
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#include "pgm.h"
#include "wfc.h"
#include "tile.h"

//#include "cJSON.h"

#define MAX_THREADS 8
#define MIN_THREADS 1
#define MAX_SIZE_CELL 10
#define QTD_RESULT 1
#define REPEAT 1


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


void all_to_zero(World w, int qtd_tileset){
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    int rows_per_process = w->height / size;
    int remaining_rows = w->height % size;

    int initial_row = rows_per_process * rank;
    int final_row = rows_per_process * (rank+1);
    if(rank == size-1){
        initial_row = (rows_per_process) * rank;
        final_row = w->height;
    }


    int initials_rows[size];
    MPI_Gather(&initial_row, 1, MPI_INT, initials_rows, 1, MPI_INT, 0 , MPI_COMM_WORLD);
    
    for (int y = initial_row; y < final_row; y++)
    {
        for (int x = 0; x < w->width; x++)
        {
            if(rank == 0){
                int valor;
                for (int i = 1; i < size - 1; i++)
                {
                    MPI_Recv(&valor, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    w->map[y + initials_rows[i]][x].collapsedValue = valor;

                    MPI_Recv(&valor, 1, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    w->map[y + initials_rows[i]][x].totalEntropy = valor;

                    for (int i = 0; i < qtd_tileset; i++)
                    {
                        MPI_Recv(&valor, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        w->map[y + initials_rows[i]][x].options[i] = valor;
                    }
                    
                }
            }else{
                int enviar = w->map[y][x].collapsedValue;
                MPI_Send(&enviar, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                enviar = w->map[y][x].totalEntropy;
                MPI_Send(&enviar, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

                for (int i = 0; i < qtd_tileset; i++)
                {
                    enviar = w->map[y][x].options[i];
                    MPI_Send(&enviar, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
                }
            }
        }
    }
    
    MPI_Bcast(&initial_row, 1, MPI_INT, size - 1, MPI_COMM_WORLD);
    if(rank == 0 || rank == size - 1){
        for (int y = initial_row; y < w->height; y++)
        {
            for (int x = 0; x < w->width; x++)
            {
                if(rank == 0){
                    int valor;
                    MPI_Recv(&valor, 1, MPI_INT, size -1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    w->map[y][x].collapsedValue = valor;
                }else{
                    int enviar = w->map[y][x].collapsedValue;
                    MPI_Send(&enviar, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
            }
        }
    }
    
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) + rank);
    //srand((unsigned int)time(NULL));

    double start_time, end_time;
    //cJSON *rootArray = cJSON_CreateArray();

    int increment = (MAX_SIZE_CELL)/QTD_RESULT;

    Tileset t = open_tileset("tileset/maze 32.txt");
    
    for(int i = MAX_SIZE_CELL; i > 0; i-= increment)
    {
        //cJSON *elementObject = cJSON_CreateObject();
        //cJSON *resultArray = cJSON_CreateArray();

        //cJSON_AddNumberToObject(elementObject, "cell_height", i);
        //cJSON_AddNumberToObject(elementObject, "cell_width", i);

        //cJSON *resultItem = cJSON_CreateObject();
        //cJSON *timeArray = cJSON_CreateArray();
        //cJSON_AddNumberToObject(resultItem, "threads", 4);

        
        for(int k = 0; k < REPEAT; k++)
        {
            World w = new_world(i, i, t->qtd);
            //------------------------------------------------
            start_time = MPI_Wtime();
            waveFuctionCollapse(t, w);
            end_time = MPI_Wtime();
            //------------------------------------------------
            all_to_all(w, t->qtd);
            //if(rank == 0){
                Pgm p = convertWfc(w, t);
                char *name = malloc(sizeof(char) * 100);
                sprintf(name, "result/imagem/wfc(C-%dx%d)(Rank-%d)(R-%d).pgm", i, i, rank, k);
                pgm_file(name, p);
          
                free_pgm(p);
                free(name);
                double total_time = end_time - start_time;
                printf("Cell(%dx%d)\ntentativa:%d\ntempo:%lf\n\n",i,i,k, total_time);
            //}
            //free_world(w);
            
            //cJSON *time = cJSON_CreateNumber(total_time);
            //cJSON_AddItemToArray(timeArray, time);
        }
            
        //cJSON_AddItemToObject(resultItem, "time", timeArray);
        //cJSON_AddItemToArray(resultArray, resultItem);

        //cJSON_AddItemToObject(elementObject, "result", resultArray);
        //cJSON_AddItemToArray(rootArray, elementObject);

    }
    free_tileset(t);
    
    //char *json_string = cJSON_Print(rootArray);
    //FILE *arquivo = fopen("result/dados.json", "w");
    //if (arquivo != NULL) {
    //    fprintf(arquivo, json_string);
    //    fclose(arquivo);
    //}
    //cJSON_Delete(rootArray);
    //free(json_string);

    //system("python graficos.py");
    MPI_Finalize();
    return 0;
}

