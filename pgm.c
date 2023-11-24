
#include "pgm.h"

Pgm new_pgm(int height, int width)
{
    Pgm pgm = (Pgm) malloc(sizeof(struct Pgm));

    pgm->height = height;
    pgm->width = width;

    pgm->data = (int **) malloc(sizeof(int *) * height);
    for (int i = 0; i < height; i++)
    {
        pgm->data[i] = (int *) malloc(sizeof(int) * width);
    }
    return pgm;
}

int getGrayColor(int val)
{
    if(val > 0)
    {
        if(val < MAX_GRAY)
        {
            return MAX_GRAY/val;
        }
        return MAX_GRAY;
    }
    return 0;
}

void pgm_file(char *path, Pgm pgm)
{
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    FILE *arq;

    if(rank == 0)
        arq = fopen(path, "w");
    else
        arq = fopen(path, "a");
    
    int max_gray = higher_number(pgm);

    if(arq != NULL)
    {
        int sum_height;
        MPI_Allreduce(&pgm->height, &sum_height, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
        if(rank == 0)
            fprintf(arq, "P2\n%d %d\n%d\n", sum_height, pgm->width, max_gray);

        for (int k = 0; k < size; k++)
        {
            if(rank == k)
            {
                for (int i = 0; i < pgm->height; i++)
                {
                    for (int j = 0; j < pgm->width; j++)
                    {
                        fprintf(arq, "%d", pgm->data[i][j]);
                        if(j != pgm->width-1)
                            fprintf(arq, " ");

                    }
                    if(i != pgm->height-1)
                    {
                        fprintf(arq, "\n");
                    }
                }
                fprintf(arq, "\n");
                fclose(arq);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
}

void pgm_files(char *path, Pgm pgm)
{
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    FILE *arq;

    arq = fopen(path, "w");
    int max_gray = higher_number(pgm);
    if(arq != NULL)
    {
        fprintf(arq, "P2\n%d %d\n%d\n", pgm->width, pgm->height, max_gray);

        for (int k = 0; k < size; k++)
        {
            if(rank == k)
            {
                for (int i = 0; i < pgm->height; i++)
                {
                    for (int j = 0; j < pgm->width; j++)
                    {
                        fprintf(arq, "%d", pgm->data[i][j]);
                        if(j != pgm->width-1)
                            fprintf(arq, " ");

                    }
                    if(i != pgm->height-1)
                    {
                        fprintf(arq, "\n");
                    }
                }
                fprintf(arq, "\n");
                fclose(arq);
            }
        }
    }
}

void printPgm(Pgm pgm)
{
    for (int i = 0; i < pgm->height; i++)
    {
        for (int j = 0; j < pgm->width; j++)
        {
            printf("%d ", pgm->data[i][j]);
        }
        printf("\n");

    }

}

void free_pgm(Pgm pgm)
{
    for (int i = 0; i < pgm->height; i++)
    {
        free(pgm->data[i]);
    }
    free(pgm->data);
    free(pgm);
}

int higher_number(Pgm p)
{
    int result = p->data[0][0];
    for (int i = 0; i < p->height; i++)
    {
        for (int j = 0; j < p->width; j++)
        {
            if(p->data[i][j] > result)
                result = p->data[i][j];
        }
    }
    return result;
}
