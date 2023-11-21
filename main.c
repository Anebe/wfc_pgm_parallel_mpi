#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#include "pgm.h"
#include "wfc.h"
#include "tile.h"

#include "cJSON.h"

#define MAX_THREADS 8
#define MIN_THREADS 1
#define MAX_SIZE_CELL 10
#define QTD_RESULT 1
#define REPEAT 1


int main()
{
    srand((unsigned int)time(NULL));
    double start_time, end_time;
    cJSON *rootArray = cJSON_CreateArray();

    int increment = (MAX_SIZE_CELL)/QTD_RESULT;

    Tileset t = open_tileset("tileset/short line.txt");
    for(int i = MAX_SIZE_CELL; i > 0; i-= increment)
    {
        cJSON *elementObject = cJSON_CreateObject();
        cJSON *resultArray = cJSON_CreateArray();

        cJSON_AddNumberToObject(elementObject, "cell_height", i);
        cJSON_AddNumberToObject(elementObject, "cell_width", i);

        //for (int j = MIN_THREADS; j <= MAX_THREADS; j+=j)
        //{
            cJSON *resultItem = cJSON_CreateObject();
            cJSON *timeArray = cJSON_CreateArray();
            cJSON_AddNumberToObject(resultItem, "threads", 4);

            
            for(int k = 0; k < REPEAT; k++)
            {
                World w = new_world(i, i, t->qtd);
                //------------------------------------------------
                start_time = MPI_Wtime();
                waveFuctionCollapse(t, w);
                end_time = MPI_Wtime();
                //------------------------------------------------
                Pgm p = convertWfc(w, t);
                char *name = malloc(sizeof(char) * 100);
                sprintf(name, "result/imagem/wfc(C-%dx%d)(R-%d).pgm", i, i, k);
                //pgm_file(name, p);

                free(name);
                free_world(w);
                free_pgm(p);
                
                double total_time = end_time - start_time;
                printf("Cell(%dx%d)\ntentativa:%d\ntempo:%lf\n\n",i,i,k, total_time);
                cJSON *time = cJSON_CreateNumber(total_time);
                cJSON_AddItemToArray(timeArray, time);
            }
            
            cJSON_AddItemToObject(resultItem, "time", timeArray);
            cJSON_AddItemToArray(resultArray, resultItem);
        //}

        cJSON_AddItemToObject(elementObject, "result", resultArray);
        cJSON_AddItemToArray(rootArray, elementObject);

    }
    free_tileset(t);

    char *json_string = cJSON_Print(rootArray);
    FILE *arquivo = fopen("result/dados.json", "w");

    if (arquivo != NULL) {
        fprintf(arquivo, json_string);
        fclose(arquivo);
    }
    
    cJSON_Delete(rootArray);
    free(json_string);

    system("python graficos.py");

    return 0;
}


