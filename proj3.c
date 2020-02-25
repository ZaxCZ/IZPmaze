/*
**   Autor:                 Petr Valenta (xvalen20)
**   Nazev:                 proj3.c
**   Popis:		            3. projekt IZP na FIT VUT (pruchod bludistem)
**   Vytvoreno:             22. 11. 2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



const char *HELPMSG =
    "///////////////////////////////////////////////////////////////////////////////\n"
    "Program Pruchod bludistem.\n"
    "Autor: Petr Valenta\n"
    "\n"
    "--help       Vypise tuto napovedu\n"
    "\n"
    "--test soubor.txt  Zkontroluje, zda dany soubor obsahuje radnou definici \n"
    "                   mapy bludiste. Odpovida => Valid / Neodpovida => Invalid\n"
    "\n"
    "--rpath R C bludiste.txt Hleda pruchod bludistem na vstupu na radku R a\n"
    "                         sloupci C, za pomoci pravidla prave ruky.\n"
    "\n"
    "--lpath R C bludiste.txt Hleda pruchod bludistem na vstupu na radku R a\n"
    "                         sloupci C, za pomoci pravidla leve ruky.\n"
    "\n"
    "--shortest R C bludiste.txt  Hleda nejkratsi cestu z bludiste.\n";


typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

typedef struct {
  int rows;
  int cols;
  int mode;
} Args;

/*shortest*/

#define STACK_MAX 1000



typedef struct item Titem;

struct item
{
    int distance;  /*vzdalenost bunky od pocatecni bunky*/
    char value;
    int r;
    int c;
    Titem * levy;  /*ukazatel na bundku, ktera navazuje po prejiti leve hranice, pokud je hranice pevna -> NULL */
    Titem * pravy;  /* pokud ukazuje ven z bludiste, a je pruchozi, nastavi se na END */
    Titem * vertikalni;
};

typedef struct {
    Titem * prvni; /* ukazatel na startovni policko*/
} Ttree;

struct Stack {
    Titem* data[STACK_MAX];
    int size;
};
typedef struct Stack Stack;


void Stack_Init(Stack *S)
{
    S->size = 0;
}


void Stack_Push(Stack *S, Titem* d)
{
    if (S->size < STACK_MAX)
        S->data[S->size++] = d;
    else
        fprintf(stderr, "Error: stack full\n");
}

Titem * Stack_Pop(Stack *S)
{
    if (S->size == 0)
        return NULL;
    else
    {
        S->size--;
        return S->data[S->size];
    }
}


#define NSTACK_MAX  300
/*** stack 2**///


struct NStack {
    int data[NSTACK_MAX];
    int size;
};
typedef struct NStack NStack;


void NStack_Init(NStack *S)
{
    S->size = 0;
}


void NStack_Push(NStack *S, int sour)
{
    if (S->size < NSTACK_MAX)
        S->data[S->size++] = sour;
    else
        fprintf(stderr, "Error: stack full\n");
}

int NStack_Pop(NStack *S)
{
    if (S->size == 0)
        return -1;
    else
    {
        S->size--;
        return S->data[S->size];
    }
}

/**** stack 2 konec ***/



/*konec shortest*/

enum errors {EOK, EIN, EARG, EHLP, EFILE, EML, EUN};
enum borders {left = 11, right, top, bot};   /*posunute kvuli prekryvu s errors*/
enum vysledek {valid, invalid};
enum modes {lefth, righth, shortestm, testm};

void mapInit(Map * map);
int mapLoad(Map * map, char * fileName);
void mapClear(Map * map);
void mapAppend(Map * map, char a, int r, int c);    /// prida bunku na konec *cells

int isnumber(char * buffer);  /* overi, zda je string validni cislo */
int tonumber(char * buffer);  /* konvertuje string na double */
int myisdigit(int a);         /* vraci 1 kdyz se jedna o cislici, jinak 0 */
bool isborder(Map *map, int r, int c, int border); /* vrati true kdyz tam je hranice, jinak false */
int start_border(Map *map, int r, int c, int leftright); /* vrati, ktera hranice se ma nasledovat  */
int test(Map *map);
int rpath(Map * map, int r, int c);
int lpath(Map * map, int r, int c);
int shortest(Map * map, int r, int c);
int getargs(int argc, char ** argv, Args * args);
void printErr(int e);
int start (char ** argv, Args * args);
void printHlp(void);
char getcell(Map * map, int r, int c);
void freemap(Map * map);
void uvolni(Titem ** array, Map * map);

/*shortest fce*/
void treeInit(Ttree * tree);
Titem * itemNew(int dist, int r, int c);

/* + NEJAKA FCE NA FREE*/

/*konec shortest*/



int main(int argc, char ** argv)
{
    Args args;
    int returnValue = EUN;

    returnValue = getargs(argc, argv, &args);
    if (returnValue != EOK)
    {
        printErr(returnValue);
        return returnValue;
    }

    returnValue = start(argv, &args);
    if (returnValue != EOK)
    {
        printErr(returnValue);
        return returnValue;
    }

    return returnValue;
}



/*shortest STUFF*/

char getcell(Map * map, int r, int c)
{
    return map->cells[map->cols*(r - 1) + c - 1];
}



int shortest(Map * map, int r, int c)
{
    Stack stack;
    Stack_Init(&stack);

    int col = c;
    int row = r;
    int dist = 0;
    int sdist = 80000;
    int i;
    int j;


    /* print bludiste*/
     /*


    for (i = 1; i <= map->rows; i++)
    {
        for (j = 1; j <= map->cols; j++)
        {
            printf("%c ", map->cells[map->cols*(i - 1) + j - 1]);
        }
         printf("\n");
    }
     */
    /*****************/


    Titem * vychod = NULL;

    Titem ** array = malloc((map->cols+2)*((map->rows+2)*sizeof(Titem)));
    for (i = 0; i <= map->rows + 1; i++)
    {
        for (j = 0; j <= map->cols + 1; j++)
        {
            array[i * (map->cols + 1) + j] = NULL;
        }
    }


    Ttree tree;
    tree.prvni = itemNew(dist, r, c);
    Titem * currItem = tree.prvni;

    array[r * (map->cols + 1) + c] = currItem;

    Stack_Push(&stack, currItem);

    while ((currItem = Stack_Pop(&stack)) != NULL)
    {
        dist = currItem->distance + 1;

        col = currItem->c;
        row = currItem->r;

        if (isborder(map, row, col, left))
        {
            currItem->levy = NULL;
        }
        else
        {
            if (col - 1 == 0)
            {
                currItem->levy = NULL;
                if (dist < sdist)
                {
                    sdist = dist;
                    vychod = currItem;
                }
            }
            else
            {
                if (currItem->levy == NULL)
                {
                    if (array[row * (map->cols + 1) + col - 1] != NULL)
                    {
                        currItem->levy = array[row * (map->cols + 1) + col -1];
                        if (currItem->levy->distance > dist)
                            Stack_Push(&stack, currItem->levy);
                    }
                    else
                    {
                        currItem->levy = itemNew(dist, row, col - 1);
                        Stack_Push(&stack, currItem->levy);
                    }
                }
                else
                {
                    if (currItem->levy->distance > dist)
                    {
                        currItem->levy->distance = dist;
                        Stack_Push(&stack, currItem->levy);
                    }
                }
            }
        }

        if (isborder(map, row, col, right))
        {
            currItem->pravy = NULL;
        }
        else
        {
            if (col == map->cols)
            {
                currItem->pravy = NULL;
                if (dist < sdist)
                {
                    sdist = dist;
                    vychod = currItem;
                }
            }
            else
                if (currItem->pravy == NULL)
                {
                    if (array[row * (map->cols + 1) + col + 1] != NULL)
                    {
                        currItem->pravy = array[row * (map->cols + 1) + col + 1];
                        if (currItem->pravy->distance > dist)
                            Stack_Push(&stack, currItem->pravy);
                    }
                    else
                    {
                        currItem->pravy = itemNew(dist, row, col + 1);
                        Stack_Push(&stack, currItem->pravy);
                    }
                }
                else
                {
                    if (currItem->pravy->distance > dist)
                    {
                        currItem->pravy->distance = dist;
                        Stack_Push(&stack, currItem->pravy);
                    }
                }
        }

        if (isborder(map, row, col, top))
        {
            currItem->vertikalni = NULL;
        }
        else
        {
            if(row % 2 == 0)
            {
                if(col % 2 == 0)  /*sudy radek sudy sloupec*/
                {
                    if (row - 1 == 0)
                    {
                        currItem->vertikalni = NULL;
                        if (dist < sdist)
                        {
                            sdist = dist;
                            vychod = currItem;
                        }
                    }
                    else
                    {
                        if (currItem->vertikalni == NULL)
                        {
                            if (array[(row - 1) * (map->cols + 1) + col] != NULL)
                            {
                                currItem->vertikalni = array[(row - 1) * (map->cols + 1) + col];
                                if (currItem->vertikalni->distance > dist)
                                    Stack_Push(&stack, currItem->vertikalni);
                            }
                            else
                            {
                                currItem->vertikalni = itemNew(dist, row - 1, col);
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                        else
                        {
                            if (currItem->vertikalni->distance > dist)
                            {
                                currItem->vertikalni->distance = dist;
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                    }
                }
                else
                {
                    if (row  == map->rows)
                    {
                        currItem->vertikalni = NULL;
                        if (dist < sdist)
                        {
                            sdist = dist;
                            vychod = currItem;
                        }
                    }
                    else
                    {
                        if (currItem->vertikalni == NULL)
                        {
                            if (array[(row + 1) * (map->cols + 1) + col] != NULL)
                            {
                                currItem->vertikalni = array[(row + 1) * (map->cols + 1) + col];
                                if (currItem->vertikalni->distance > dist)
                                    Stack_Push(&stack, currItem->vertikalni);
                            }
                            else
                            {
                                currItem->vertikalni = itemNew(dist, row + 1, col);
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                        else
                        {
                            if (currItem->vertikalni->distance > dist)
                            {
                                currItem->vertikalni->distance = dist;
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                    }
                }
            }
            else
            {
                if(col % 2 == 0)
                {
                    if (row == map->rows)
                    {
                        currItem->vertikalni = NULL;
                        if (dist < sdist)
                        {
                            sdist = dist;
                            vychod = currItem;
                        }
                    }
                    else
                    {
                        if (currItem->vertikalni == NULL)
                        {
                            if (array[(row + 1) * (map->cols + 1) + col] != NULL)
                            {
                                currItem->vertikalni = array[(row + 1) * (map->cols + 1) + col];
                                if (currItem->vertikalni->distance > dist)
                                    Stack_Push(&stack, currItem->vertikalni);
                            }
                            else
                            {
                                currItem->vertikalni = itemNew(dist, row + 1, col);
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                        else
                        {
                            if (currItem->vertikalni->distance > dist)
                            {
                                currItem->vertikalni->distance = dist;
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                    }
                }
                else
                {
                    if (row - 1 == 0)
                    {
                        currItem->vertikalni = NULL;
                        if (dist < sdist)
                        {
                            sdist = dist;
                            vychod = currItem;
                        }
                    }
                    else
                    {
                        if (currItem->vertikalni == NULL)
                        {
                            if (array[(row - 1) * (map->cols + 1) + col] != NULL)
                            {
                                currItem->vertikalni = array[(row - 1) * (map->cols + 1) + col];
                                if (currItem->vertikalni->distance > dist)
                                    Stack_Push(&stack, currItem->vertikalni);
                            }
                            else
                            {
                                currItem->vertikalni = itemNew(dist, row - 1, col);
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                        else
                        {
                            if (currItem->vertikalni->distance > dist)
                            {
                                currItem->vertikalni->distance = dist;
                                Stack_Push(&stack, currItem->vertikalni);
                            }
                        }
                    }
                }
            }
        }


        array[row * (map->cols + 1) + col] = currItem;

    }



    if (vychod != NULL)
    {
        Titem * next = vychod;
        NStack * nstack = malloc(sizeof(NStack));
        if (nstack == NULL)
            return EML;
        NStack_Init(nstack);
        int change = 0;


        NStack_Push(nstack, next->c);
        NStack_Push(nstack, next->r);


        while (next->distance > 0)
        {
            if (next->vertikalni != NULL)
            {
                if (next->vertikalni->distance == next->distance - 1)
                {
                    next = next->vertikalni;
                    change = 1;
                }
            }
            if (!change)
            {
                if (next->levy != NULL)
                {
                    if (next->levy->distance == next->distance - 1)
                    {
                        next = next->levy;
                        change = 1;
                    }
                }
            }
            if (!change)
            {
                if (next->pravy != NULL)
                {
                    if (next->pravy->distance == next->distance - 1)
                    {
                        next = next->pravy;
                        change = 1;
                    }
                }
            }

            NStack_Push(nstack, next->c);
            NStack_Push(nstack, next->r);
            change = 0;
        }

        while (1)
        {
            if((row = NStack_Pop(nstack)) == -1)
                break;
            if((col = NStack_Pop(nstack)) == -1)
                break;
            printf("%d,%d\n",row, col);
        }
        free(nstack);
        uvolni(array, map);
        return EOK;
    }
    else
    {
        printf("Z daneho mista neni mozne bludiste opustit\n");
        uvolni(array, map);
        return EOK;
    }

    uvolni(array, map);

}

void uvolni(Titem ** array, Map * map)
{
    int i;
    int j;
    for (i = 0; i <= map->rows + 1; i++)
    {
        for (j = 0; j <= map->cols + 1; j++)
        {
            if (array[i * (map->cols + 1) + j] != NULL)
            {
                free(array[i * (map->cols + 1) + j]);
            }
        }
    }
    free(array);
    freemap(map);
}

void freemap(Map * map)
{
    free(map->cells);
}

Titem * itemNew(int dist, int r, int c)
{
    Titem * item = malloc(sizeof(Titem));
    item->c = c;
    item->r = r;
    item->distance = dist;
    item->levy = NULL;
    item->pravy = NULL;
    item->vertikalni = NULL;

    return item;
}


/*SHORTEST STUFF END*/





int start (char ** argv, Args * args)
{
    Map map;
    mapInit(&map);
    int ret;

    switch (args->mode)
    {
    case righth:
        ret = mapLoad(&map, argv[4]);
        if (ret != EOK)
        {
            free(map.cells);
            return ret;
        }
        if (test(&map) == valid)
        {
            ret = rpath(&map, args->rows, args->cols);
            free(map.cells);
        }
        else
        {
            free(map.cells);
            return EIN;
        }
        break;

    case lefth:
        ret = mapLoad(&map, argv[4]);
        if (ret != EOK)
        {
            free(map.cells);
            return ret;
        }
        if (test(&map) == valid)
        {
            ret = lpath(&map, args->rows, args->cols);
            free(map.cells);
        }
        else
        {
            free(map.cells);
            return EIN;
        }
        break;

    case testm:
        ret = mapLoad(&map, argv[2]);
        if (ret != EOK)
        {
            free(map.cells);
            printf("Invalid\n");
            return EOK;
        }
        ret = test(&map);
        if (ret == valid)
            printf("Valid\n");
        else
            printf("Invalid\n");
        free(map.cells);
        ret = EOK;
        break;

    case shortestm:
        ret = mapLoad(&map, argv[4]);
        if (ret != EOK)
        {
            free(map.cells);
            return ret;
        }
        if (test(&map) == valid)
        {
            ret = shortest(&map, args->rows, args->cols);
            ret = EOK;
        }
        else
        {
            free(map.cells);
            return EIN;
        }
        break;

    default:
        return EUN;
        break;
    }
    return ret;
}

void printHlp(void) /* tiskne napovedu */
{
    printf("%s\n", HELPMSG);
}

void printErr(int e)  /* tiskne chybove hlasky na chybovy vystup stderr */
{
    char* err = "unknown";

    switch (e)
    {
    case EARG:
        err = "argument";
        break;
    case EIN:
        err = "input";
        break;
    case EUN:
        err = "unknown";
        break;
    case EFILE:
        err = "file";
        break;
    case EML:
        err = "malloc";
        break;;
    case EHLP:
        printHlp();
        return;
        break;
    default:
        break;
    }

    fprintf(stderr,"ERROR: %s (errcode: %d)  // try: --help\n", err, e);
}

int getargs(int argc, char ** argv, Args * args)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            printHlp();
            return EHLP;
        }
        else if (strcmp(argv[1], "--test") == 0)
        {
            if (argc == 3)
            {
                args->mode = testm;
                return EOK;
            }
            else
                return EARG;
        }
        else if (strcmp(argv[1], "--rpath") == 0)
        {
            if (argc == 5)
            {
                if (isnumber(argv[2]))
                    args->rows = tonumber(argv[2]);
                else
                    return EARG;
                if (isnumber(argv[3]))
                    args->cols = tonumber(argv[3]);
                else
                    return EARG;

                args->mode = righth;
                return EOK;
            }
            else
                return EARG;
        }
        else if (strcmp(argv[1], "--lpath") == 0)
        {
            if (argc == 5)
            {
                if (isnumber(argv[2]))
                    args->rows = tonumber(argv[2]);
                else
                    return EARG;

                if (isnumber(argv[3]))
                    args->cols = tonumber(argv[3]);
                else
                    return EARG;

                args->mode = lefth;
                return EOK;
            }
            else
                return EARG;
        }
        else if (strcmp(argv[1], "--shortest") == 0)
        {
            if (argc == 5)
            {
                if (isnumber(argv[2]))
                    args->rows = tonumber(argv[2]);
                else
                    return EARG;
                if (isnumber(argv[3]))
                    args->cols = tonumber(argv[3]);
                else
                    return EARG;

                args->mode = shortestm;
                return EOK;
            }
            else
                return EARG;
        }
    }
    else
        return EARG;
    return EUN;
}

int rpath(Map * map, int r, int c)
{
    int R = r;
    int C = c;
    /*validity test*/
    int border = start_border(map, r, c, right);
    if (border == EIN)
        return EIN;

    int change = 0;

    printf("%d,%d\n",R, C);
    while (1)
    {

        switch (border)
        {
        case bot:
            if (isborder(map, R, C, bot))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /*sudy sude*/
                    }
                    else
                        border = right;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = right; /* lichy sude*/
                    }
                    else
                        return EIN; /*lichy liche*/
                }
            }

            else
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /*sudy sude*/
                    }
                    else
                        border = left;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = left; /* lichy sude*/
                    }
                    else
                        return EIN; /*lichy liche*/
                }

                R++;
                change = 1;
            }
            break;

        case top:
            if (isborder(map, R, C, top))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = left; /*sudy sude*/
                    }
                    else
                        return EIN;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /* lichy sude*/
                    }
                    else
                        border = left; /*lichy liche*/
                }
            }

            else
            {

                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = right; /*sudy sude*/
                    }
                    else
                        return EIN;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /* lichy sude*/
                    }
                    else
                        border = right; /*lichy liche*/
                }

                R--;
                change = 1;
            }
            break;

        case left:
            if (isborder(map, R, C, left))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = right; /*sudy sude*/
                    }
                    else
                        border = bot;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = bot; /* lichy sude*/
                    }
                    else
                        border = right; /*lichy liche*/
                }
            }

            else
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = left; /*sudy sude*/
                    }
                    else
                        border = top;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = top; /* lichy sude*/
                    }
                    else
                        border = left; /*lichy liche*/
                }

                C--;
                change = 1;
            }
            break;

        case right:
            if (isborder(map, R, C, right))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = top; /*sudy sude*/
                    }
                    else
                        border = left;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = left; /* lichy sude*/
                    }
                    else
                        border = top; /*lichy liche*/
                }
            }

            else  /* neni tam stena*/
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = bot; /*sudy sude*/
                    }
                    else
                        border = right;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = right; /* lichy sude*/
                    }
                    else
                        border = bot; /*lichy liche*/
                }

                C++;
                change = 1;
            }
            break;

        default:
            break;
        }
        if (R == map-> rows + 1  || R == 0 || C == map->cols +1 || C == 0)
        {
            return EOK;
        }
        if (change)
        {
            printf("%d,%d\n",R, C);
            change = 0;
        }
    }
    return EUN;
}

int lpath(Map * map, int r, int c)
{
    int R = r;
    int C = c;
    /*validity test*/
    int border = start_border(map, r, c, left);
    if (border == EIN)
    {
        return EIN;
    }


    int change = 0;

    printf("%d,%d\n",R, C);
    while (1)
    {
        switch (border)
        {
        case bot:
            if (isborder(map, R, C, bot))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /*sudy sude*/
                    }
                    else
                        border = left;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = left; /* lichy sude*/
                    }
                    else
                        return EIN; /*lichy liche*/
                }
            }

            else
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /*sudy sude*/
                    }
                    else
                        border = right;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = right; /* lichy sude*/
                    }
                    else
                        return EIN; /*lichy liche*/
                }

                R++;
                change = 1;
            }
            break;

        case top:
            if (isborder(map, R, C, top))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = right; /*sudy sude*/
                    }
                    else
                    {
                        return EIN;   /*sudy liche*/
                    }
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /* lichy sude*/
                    }
                    else
                        border = right; /*lichy liche*/
                }
            }

            else
            {

                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = left; /*sudy sude*/
                    }
                    else
                        return EIN;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        return EIN; /* lichy sude*/
                    }
                    else
                        border = left; /*lichy liche*/
                }

                R--;
                change = 1;
            }
            break;

        case left:
            if (isborder(map, R, C, left))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = top; /*sudy sude*/
                    }
                    else
                        border = right;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = right; /* lichy sude*/
                    }
                    else
                        border = top; /*lichy liche*/
                }
            }

            else
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = bot; /*sudy sude*/
                    }
                    else
                        border = left;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = left; /* lichy sude*/
                    }
                    else
                        border = bot; /*lichy liche*/
                }

                C--;
                change = 1;
            }
            break;

        case right:
            if (isborder(map, R, C, right))
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = left; /*sudy sude*/
                    }
                    else
                        border = bot;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = bot; /* lichy sude*/
                    }
                    else
                        border = left; /*lichy liche*/
                }
            }

            else  /* neni tam stena*/
            {
                if (R % 2 == 0)
                {
                    if (C % 2 ==0)
                    {
                        border = right; /*sudy sude*/
                    }
                    else
                        border = top;   /*sudy liche*/
                }
                else
                {
                    if (C % 2 ==0)
                    {
                        border = top; /* lichy sude*/
                    }
                    else
                        border = right; /*lichy liche*/
                }

                C++;
                change = 1;
            }
            break;

        default:
            break;
        }
        if (R == map-> rows + 1  || R == 0 || C == map->cols +1 || C == 0)
        {
            return EOK;
        }
        if (change)
        {
            printf("%d,%d\n",R, C);
            change = 0;
        }
    }
    return EUN;
}

int test(Map* map)
{
    int c;
    int r;

    /*test radek po radku - prave / leve hranice*/
    for (r = 1; r < map->rows +1; r++)
    {
        for (c = 1; c < map->cols; c++)
        {
            if (isborder(map, r, c, right))
            {
                if (!isborder(map, r, c + 1, left))
                    return invalid;
            }
        }
    }

    /* projizdeni sudych sloupcu*/
    for (r = 1; r < map->rows; r+=2)
    {
        for (c = 2; c < map->cols+1; c+=2) /* sloupce 1,1     1,3   1,5      */
        {
            if (isborder(map, r, c, bot))
            {
                if (!isborder(map, r+1, c, top))
                    return invalid;
            }
            else
            {
                if (isborder(map, r+1, c, top))
                {
                    return invalid;
                }
            }
        }
    }

    /*projizdeni lichych sloupcu*/
    for (r = 2; r < map->rows; r+=2)
    {
        for (c = 1; c < map->cols+1; c+=2) /* sloupce 1,1     1,3   1,5      */
        {
            if (isborder(map, r, c, bot))
            {
                if (!isborder(map, r+1, c, top))
                    return invalid;
            }
            else
            {
                if (isborder(map, r+1, c, top))
                {
                    return invalid;
                }
            }
        }
    }

    return valid;

}

int start_border(Map *map, int r, int c, int leftright) /* vrati, ktera hranice se ma nasledovat  */
{
    char v = map->cells[map->cols*(r - 1) + c - 1]; /* nacte se hodnota vstupni bunky*/
    if (leftright == right) /*pravidlo prave ruky*/
    {
        if (r == 1) /*shora */
        {
            if(c == 1) /*levy horni roh*/
            {
                if (v & 1) /*leva hranice*/
                {
                    if (v & 4)
                        return EIN;
                    else
                        return left;  /*vstup shora*/
                }
                else
                {
                    if (v & 4)
                        return right;/*vstup zleva*/
                    else
                        return EIN;
                }


            }
            else if (c == map->cols)/*pravy horni roh*/
            {
                if (c % 2 == 0)  /* sudy */
                {
                    if (v & 2)
                        return EIN;
                    else
                        return left;    /*vstup zprava*/
                }
                else        /* lichy  */
                {
                    if (v & 4)
                    {
                        if (v & 2)
                            return EIN;
                        else
                            return top; /*vstup zprava*/
                    }
                    else
                    {
                        if (v & 2)
                            return left;  /* vstup shora*/
                        else
                            return EIN;
                    }
                }
            }
            else   /* normalni vstup shora */
            {
                if (c % 2 == 0)
                    return EIN;
                else if (v & 4)
                    return EIN;
                else
                    return left; /* vstup shora*/
            }
        }
        else if (r == map->rows) /* zdola */
        {
            if(c == 1) /*levy dolni roh*/
            {
                if (r % 2 == 0)   /*sudy */
                {
                    if (v & 4)
                    {
                        if (v & 1)
                            return EIN;
                        else
                            return bot; /*vstup zleva*/
                    }
                    else
                        return right; /* vstup zdola*/
                }
                else         /*lichy */
                {
                    if (v & 1)
                        return EIN;
                    else
                        return right;
                }
            }
            else if (c == map->cols)/*pravy dolni roh*/
            {
                if (r % 2 == 0)  /*sudy radek*/
                {
                    if (c % 2 == 0)  /*sudy radek sudy sloupec*/
                    {
                        if (v & 2)
                            return EIN;
                        else
                            return top; /*vstup zprava*/
                    }
                    else             /*sudy radek lichy sloupec*/
                    {
                        if (v & 2)
                        {
                            if (v & 4)
                                return EIN;
                            else
                                return right; /* vstup zdola*/
                        }
                        else
                        {
                            if  (v & 4)   /*vstup zprava*/
                                return left;
                            else
                                return EIN;
                        }
                    }
                }
                else             /*lichy radek*/
                {
                    if (c % 2 == 0)  /*lichy radek sudy sloupec*/
                    {
                        if (v & 2)
                        {
                            if (v & 4)
                                return EIN;
                            else
                                return right; /* vstup zdola*/
                        }
                        else
                        {
                            if  (v & 4)   /*vstup zprava*/
                                return left;
                            else
                                return EIN;
                        }
                    }
                    else             /*lichy radek lichy sloupec*/
                    {
                        if (v & 2)
                            return EIN;
                        else
                            return top; /*vstup zprava*/
                    }
                }
            }
            else   /* normalni vstup zdola */
            {
               if (r % 2 == 0)    /* sudy radek*/
               {
                    if (c % 2 == 0)  /* sudy radek sudy sloupec*/
                    {
                        return EIN;
                    }
                    else            /* sudy radek lichy sloupec*/
                    {
                        if (v & 4)
                            return EIN;
                        else
                            return right;
                    }
               }
               else
               {
                    if (c % 2 == 0)  /* lichy radek sudy sloupec*/
                    {
                        if (v & 4)
                            return EIN;
                        else
                            return right;
                    }
                    else            /* lichy radek lichy sloupec*/
                    {
                        return EIN;

                    }
                }
            }
        }
        else if (c == 1)   /*vstup zleva, rohy jsou pokryte*/
        {
            if (v & 1)
            {
                return EIN;
            }
            else
            {
                if (r % 2 == 0)
                    return bot;         /*sudy radek vstup zleva*/
                else
                    return right;    /*lichy radek zleva*/
            }
        }

        else if (c == map->cols) /*vstup zprava, rohy jsou pokryte*/
        {
            if (r % 2 == 0)  /*zprava sudy radek*/
            {
                if (v & 2)
                {
                    return EIN;
                }
                else
                {
                    if (c % 2 == 0)
                        return top; /* sudy sloupec */
                    else
                        return left;  /* lichy sloupec */
                }
            }
            else            /*zprava lichy radek*/
            {
                if (v & 2)
                {
                    return EIN;
                }
                else
                {
                    if (c % 2 == 0)
                        return left; /* sudy sloupec */
                    else
                        return top;  /* lichy sloupec */
                }
            }
        }
    }
    else if (leftright == left)
    {
        if (r == 1) /*shora */
        {
            if(c == 1) /*levy horni roh*/
            {
                if (v & 1) /*vstup shora*/
                {
                    if (v & 4)
                        return EIN;
                    else
                        return right;  /*vstup shora*/
                }
                else
                {
                    if (v & 4)
                        return top;/*vstup zleva*/
                    else
                        return EIN;
                }
            }
            else if (c == map->cols)/*pravy horni roh*/
            {
                if (c % 2 == 0)  /* sudy */
                {
                    if (v & 2)
                        return EIN;
                    else
                        return bot;    /*vstup zprava*/
                }
                else        /* lichy  */
                {
                    if (v & 4)
                    {
                        if (v & 2)
                            return EIN;
                        else
                            return left; /*vstup zprava*/
                    }
                    else
                    {
                        if (v & 2)
                            return right;  /* vstup shora*/
                        else
                            return EIN;
                    }
                }
            }
            else   /* normalni vstup shora */
            {
                if (c % 2 == 0)
                    return EIN;
                else if (v & 4)
                    return EIN;
                else
                    return right; /* vstup shora*/
            }
        }
        else if (r == map->rows) /* zdola */
        {
            if(c == 1) /*levy dolni roh*/
            {
                if (r % 2 == 0)   /*sudy */
                {
                    if (v & 4)
                    {
                        if (v & 1)
                            return EIN;
                        else
                            return right; /*vstup zleva*/
                    }
                    else
                        return left; /* vstup zdola*/
                }
                else         /*lichy */
                {
                    if (v & 1)
                        return EIN;
                    else
                        return top;
                }
            }
            else if (c == map->cols)/*pravy dolni roh*/
            {
                if (r % 2 == 0)  /*sudy radek*/
                {
                    if (c % 2 == 0)  /*sudy radek sudy sloupec*/
                    {
                        if (v & 2)
                            return EIN;
                        else
                            return left; /*vstup zprava*/
                    }
                    else             /*sudy radek lichy sloupec*/
                    {
                        if (v & 2)
                        {
                            if (v & 4)
                                return EIN;
                            else
                                return left; /* vstup zdola*/
                        }
                        else
                        {
                            if  (v & 4)   /*vstup zprava*/
                                return bot;
                            else
                                return EIN;
                        }
                    }
                }
                else             /*lichy radek*/
                {
                    if (c % 2 == 0)  /*lichy radek sudy sloupec*/
                    {
                        if (v & 2)
                        {
                            if (v & 4)
                                return EIN;
                            else
                                return left; /* vstup zdola*/
                        }
                        else
                        {
                            if  (v & 4)   /*vstup zprava*/
                                return bot;
                            else
                                return EIN;
                        }
                    }
                    else             /*lichy radek lichy sloupec*/
                    {
                        if (v & 2)
                            return EIN;
                        else
                            return left; /*vstup zprava*/
                    }
                }
            }
            else   /* normalni vstup zdola */
            {
               if (r % 2 == 0)    /* sudy radek*/
               {
                    if (c % 2 == 0)  /* sudy radek sudy sloupec*/
                    {
                        return EIN;
                    }
                    else            /* sudy radek lichy sloupec*/
                    {
                        if (v & 4)
                            return EIN;
                        else
                            return left;
                    }
               }
               else
               {
                    if (c % 2 == 0)  /* lichy radek sudy sloupec*/
                    {
                        if (v & 4)
                            return EIN;
                        else
                            return left;
                    }
                    else            /* lichy radek lichy sloupec*/
                    {
                        return EIN;

                    }
                }
            }
        }
        else if (c == 1)   /*vstup zleva, rohy jsou pokryte*/
        {
            if (v & 1)
            {
                return EIN;
            }
            else
            {
                if (r % 2 == 0)
                    return right;         /*sudy radek vstup zleva*/
                else
                    return top;    /*lichy radek zleva*/
            }
        }

        else if (c == map->cols) /*vstup zprava, rohy jsou pokryte*/
        {
            if (r % 2 == 0)  /*zprava sudy radek*/
            {
                if (v & 2)
                {
                    return EIN;
                }
                else
                {
                    if (c % 2 == 0)
                        return left; /* sudy sloupec */
                    else
                        return bot;  /* lichy sloupec */
                }
            }
            else            /*zprava lichy radek*/
            {
                if (v & 2)
                {
                    return EIN;
                }
                else
                {
                    if (c % 2 == 0)
                        return bot; /* sudy sloupec */
                    else
                        return left;  /* lichy sloupec */
                }
            }
        }
    }

    return EUN; /* pokud neni ani left ani right */

}

bool isborder(Map *map, int r, int c, int border)
{
    if(border == left)
    {
        if(map->cells[map->cols*(r - 1) + c - 1] & 1)  /* 001 leva hranice*/
            return true;
        else
            return false;
    }
    else if(border == right)
    {
        if(map->cells[map->cols*(r - 1) + c - 1] & 2) /* 010 prava hranice*/
            return true;
        else
            return false;
    }
    else if(border == top || border == bot)
    {
        if(map->cells[map->cols*(r - 1) + c - 1] & 4) /* 100 horizontalni hranice*/
            return true;
        else
            return false;
    }
    return false;
}


void mapInit(Map * map)
{
    map->rows = 0;
    map->cols = 0;
    map->cells = NULL;
}

void mapAppend(Map * map, char a, int r, int c)
{
    map->cells[r*(map->cols)+c] = a;
}

int mapLoad(Map * map, char * fileName)
{
    FILE* file = fopen(fileName, "r");
    if(file == NULL)
    {
        return EFILE;
    }

    int r = 0;   /*  aktualni row  */
    int c = 0;   /*  aktualni col  */
    int z = 0;   /*  znak   */
    char buffer[10];
    int i = 0;

    while (1)
    {
        z = fgetc(file);
        if (z == '\n')
        {
            fclose(file);
            return EIN;
        }
        else if (z == ' ')
        {
            break;
        }
        buffer[i] = z;
        i++;
    }
    buffer[i] = '\0';
    if (isnumber(buffer))
    {
        map->rows = tonumber(buffer);
    }
    else
    {
        fclose(file);
        return EIN;
    }

    i = 0;
    while (1)
    {
        z = fgetc(file);
        if (z == '\n' || z == ' ')
        {
            break;
        }
        buffer[i] = z;
        i++;
    }
    buffer[i] = '\0';
    if (isnumber(buffer))
    {
        map->cols = tonumber(buffer);
    }
    else
    {
        fclose(file);
        return EIN;
    }


    map->cells = malloc(map->cols * map->rows * sizeof(char)); /* alokace pole pro bunky */
    if (map->cells == NULL)
    {
        fclose(file);
        return EML;  /*chyba alokace*/
    }
    if (z == ' ')
    {
        while ((z = fgetc(file)) == ' ')
        {
         /*donacita prvni radek*/
        }
    }
    for (r = 0; r < map->rows; r++)
    {
        for(; c < map->cols + 1;)
        {
            z = fgetc(file);
            if (z == ' ')
            {

            }
            else if (z == '\n')
            {
                break;
            }
            else if (z == EOF)
            {
                break;
            }
            else if (myisdigit(z) < 8 && myisdigit(z) >= 0)
            {
                mapAppend(map, z, r, c);
                c++;
            }
            else
            {
                fclose(file);
                return EIN;
            }
        }
        if (c != (map->cols))
        {
            fclose(file);
            return EIN;
        }
        c = 0;
    }
    fclose(file);
    return EOK;
}

void mapClear(Map * map)
{
    free(map->cells);
    map->cells = NULL;
    map->rows = 0;
    map->cols = 0;
}

int isnumber(char * buffer)  /* overi, zda je string validni cislo */
{
    while (*buffer != '\0')
    {
        if (! myisdigit(*buffer))
        {
            return 0;
        }
        buffer++;
    }
    return 1;
}

int myisdigit(int a)   /* vraci 1 kdyz se jedna o cislici, jinak 0 */
{
    if (a >= '0' && a <= '9')
        return 1;
    return 0;
}

int tonumber (char * buffer)  /* konvertuje string na double */
{
    double number = 0;
    int i = 0;

    while (buffer[i] != '\0')
    {
        number = number * 10 + (buffer[i] - '0');
        i++;
    }

    return number;
}
