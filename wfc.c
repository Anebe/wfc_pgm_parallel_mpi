#include "wfc.h"

World new_world(int height, int width, int entropy)
{
    World world = (World) malloc(sizeof(struct World));

    world->height = height;
    world->width = width;

    world->map = (Cell **) malloc(sizeof(Cell *) * world->height);
    for (int y = 0; y < world->height; y++)
    {
        world->map[y] = (Cell *) malloc(sizeof(Cell) * world->width);

        for (int x = 0; x < world->width; x++)
        {
            world->map[y][x].options = (int *)  malloc(sizeof(int) * entropy);

            for (int i = 0; i < entropy; i++)
            {
                world->map[y][x].options[i] = 1;
            }
            world->map[y][x].totalEntropy = entropy;
            world->map[y][x].collapsedValue = -1;
        }
    }
    return world;
}

void free_world(World world)
{
    for (int i = 0; i < world->height; i++)
    {
        for (int j = 0; j < world->width; j++)
        {
            free(world->map[i][j].options);
        }
        free(world->map[i]);
    }
    free(world->map);
    free(world);
}

void findLowestEntropy(World world, Tileset tileset, int *lowXResult, int *lowYResult)
{
    int height = world->height;
    int width = world->width;
    int lowEntropy = tileset->qtd;
    int lowX = 0;
    int lowY = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Cell cell = world->map[y][x];
            if( cell.collapsedValue == -1 &&
                cell.totalEntropy < lowEntropy &&
                cell.totalEntropy != 0){
                lowEntropy = cell.totalEntropy;
                lowX = x;
                lowY = y;
            }
        }
    }
    *lowXResult = lowX;
    *lowYResult = lowY;
}

void propagateCollapse(const int collapseTarget, const int y, const int x, World world, const Tileset tileset)
{
    const int size = tileset->size;

    if(x >= 0 && x < world->width){
        if (y-1 >= 0)
        {
            if(world->map[y-1][x].collapsedValue == -1){
                Cell *top = &world->map[y-1][x];

                for (int i = 0; i < tileset->qtd; i++)
                {
                    for (int j = 0; j < tileset->size; j++)
                    {
                        int isNotCombine = tileset->tile[i][size-1][j] != tileset->tile[collapseTarget][0][j];
                        top->totalEntropy -= isNotCombine && top->options[i];
                        top->options[i] = top->options[i] ? !(isNotCombine) : 0;
                    }
                }
            }

        }

        if (y+1 < world->height)
        {
            if(world->map[y+1][x].collapsedValue == -1){

            
                Cell *down =  &world->map[y+1][x];


                for (int i = 0; i < tileset->qtd; i++)
                {
                    for (int j = 0; j < tileset->size; j++)
                    {
                        int isNotCombine = tileset->tile[i][0][j] != tileset->tile[collapseTarget][size-1][j];
                        down->totalEntropy -= isNotCombine && down->options[i];
                        down->options[i] = down->options[i]? !(isNotCombine) : 0;
                    }
                }
            }
        }

    }

    if(y >= 0 && y < world->height){
        if (x-1 >= 0 && world->map[y][x-1].collapsedValue == -1)
        {
            Cell *left = &world->map[y][x-1];


            for (int i = 0; i < tileset->qtd; i++)
            {
                for (int j = 0; j < tileset->size; j++)
                {
                    int isNotCombine = tileset->tile[i][j][size-1] != tileset->tile[collapseTarget][j][0];
                    left->totalEntropy -= isNotCombine && left->options[i];
                    left->options[i] = left->options[i] ? !(isNotCombine) : 0;
                }
            }

        }


        if (x+1 < world->width && world->map[y][x+1].collapsedValue == -1)
        {
            Cell *right = &world->map[y][x+1];


            for (int i = 0; i < tileset->qtd; i++)
            {
                for (int j = 0; j < tileset->size; j++)
                {
                    int isNotCombine = tileset->tile[i][j][0] != tileset->tile[collapseTarget][j][size-1];
                    right->totalEntropy -= isNotCombine && right->options[i];
                    right->options[i] = right->options[i] ? !(isNotCombine) : 0;
                }
            }

        }

    }

}

void collapse(World world, Tileset tileset)
{
    int lowestY = 0;
    int lowestX = 0;

    findLowestEntropy(world, tileset, &lowestX, &lowestY);

    int lowestTotalEntropy = world->map[lowestY][lowestX].totalEntropy;

    if(lowestTotalEntropy == 0)
        return;

    int *entropyPossibility = (int *) malloc(sizeof(int) * lowestTotalEntropy);
    int index = 0;

    for (int j = 0; j < tileset->qtd; j++)
    {
        if(world->map[lowestY][lowestX].options[j] == 1){
            entropyPossibility[index++] = j;
        }
    }

    int collapseValue = entropyPossibility[rand() % lowestTotalEntropy];
    free(entropyPossibility);


    for (int i = 0; i < tileset->qtd; i++)
    {
        world->map[lowestY][lowestX].options[i] = 0;
    }


    world->map[lowestY][lowestX].options[collapseValue] = 1;
    world->map[lowestY][lowestX].collapsedValue = collapseValue;
    world->map[lowestY][lowestX].totalEntropy = 1;

    propagateCollapse(collapseValue, lowestY, lowestX, world, tileset);
}

void waveFuctionCollapse(Tileset tileset, World world)
{
    make_border(world, tileset);
    //all_to_all(world, tileset);
    for (int i = 0; i < world->height * world->width; i++)
    {
        collapse(world, tileset);
    }
}

void make_border(World world, Tileset tileset){
    
    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int y = 0;
    for (int x = 0; x < world->width; x++)
    {
        int lowestTotalEntropy = world->map[y][x].totalEntropy;
        int *entropyPossibility = (int *) malloc(sizeof(int) * lowestTotalEntropy);
        int index = 0;

        for (int j = 0; j < tileset->qtd; j++)
        {
            if(world->map[y][x].options[j] == 1){
                entropyPossibility[index++] = j;
            }
        }

        int collapseValue = entropyPossibility[rand() % lowestTotalEntropy];
        free(entropyPossibility);

        Cell *map = &world->map[y][x];
        int priv_qtd = tileset->qtd;

        for (int i = 0; i < priv_qtd; i++)
        {
            map->options[i] = 0;
        }

        map->options[collapseValue] = 1;
        map->collapsedValue = collapseValue;
        map->totalEntropy = 1;

        propagateCollapse(collapseValue, y, x, world, tileset);
        int other_collapseValue = -1;

        if(rank > 0)
            MPI_Send(&collapseValue, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
        if(rank < size - 1)
            MPI_Recv(&other_collapseValue, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(other_collapseValue != -1 && rank < size - 1){
            propagateCollapse(other_collapseValue, world->height, x, world, tileset);
        }
        
        
    }
}

void all_to_all(World w, Tileset tileset){
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

    int finals_rows[size], counter_row = 0;
    MPI_Allgather(&final_row, 1, MPI_INT, finals_rows, 1, MPI_INT, MPI_COMM_WORLD);
    for (int y = 0; y < w->height; y++)
    {
        if(y == finals_rows[counter_row]){
            counter_row++;
        }

        
        if(y + 1== finals_rows[counter_row] && counter_row < size -1){

            for (int x = 0; x < w->width; x++)
            {
                MPI_Bcast(&w->map[y+1][x].collapsedValue, 1, MPI_INT, counter_row + 1, MPI_COMM_WORLD);
                propagateCollapse(w->map[y+1][x].collapsedValue, y+1, x, w, tileset);

            }
        }
        
        
        for (int x = 0; x < w->width; x++)
        {
            MPI_Bcast(&w->map[y][x].collapsedValue, 1, MPI_SHORT, counter_row, MPI_COMM_WORLD);
            MPI_Bcast(&w->map[y][x].totalEntropy, 1, MPI_INT, counter_row, MPI_COMM_WORLD);
            for (int i = 0; i < tileset->qtd; i++)
            {
                MPI_Bcast(&w->map[y][x].options[i], 1, MPI_INT, counter_row, MPI_COMM_WORLD);
            }
            
        }
        
        
    }
}

void print_world(World world)
{
    printf("height: %d width: %d\n", world->height, world->width);
    for (int i = 0; i < world->height; i++)
    {
        for (int j = 0; j < world->width; j++)
        {
            printf("%d ", world->map[i][j].collapsedValue);
        }
        printf("\n");
    }
    printf("\n");
}

Pgm convertWfc(World world, Tileset tileset)
{
    int linha = world->height * tileset->size;
    int coluna = world->width * tileset->size;

    Pgm result = new_pgm(linha, coluna);

    int a = 0, b = 0;
    for (int y = 0; y < world->height; y++)
    {
        for (int i = 0; i < tileset->size; i++)
        {
            for (int x = 0; x < world->width; x++)
            {
                for (int j = 0; j < tileset->size; j++)
                {
                    int index = world->map[y][x].collapsedValue;
                    result->data[a][b++] = tileset->tile[(index != -1) ? index : 0][i][j];
                }
            }
            b = 0;
            a++;
        }
    }
    return result;
}