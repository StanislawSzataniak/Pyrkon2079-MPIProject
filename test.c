#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

typedef struct {
    int ts;                 /* zegar lamporta */
    int ticketsNumber;      /* liczba biletów na obecny Pyrkon */
    int pyrkonNumber;       /* numer obecnego Pyrkonu */
    int wkspshopsNumber;    /* liczba warsztatów na obecnym Pyrkonie */
    int wkspTicketsNumber;  /* liczba miejsc na warsztatach */
    int wkspNumber;         /* opcjonalny numer warsztatu na który uczestnik chce zdobyć bilet */
    int dst;                /* pole ustawiane w sendPacket */
    int src;                /* pole ustawiane w wątku komunikacyjnym na processId nadawcy */
} packet_t;

typedef struct {
    packet_t *newP;
    int type;
    int dst;
} queueEl_t;

typedef struct {
    int ts;
    int src;
} request_t;

int main(void)
{
    int i;

    VECTOR_INIT(v);
    
    VECTOR_ADD(v, 1);
    VECTOR_ADD(v, 2);

<<<<<<< HEAD
    for (i = 0; i < VECTOR_TOTAL(v); i++) {
        printf("%ls ", VECTOR_GET(v, int*, i));
    }
=======
   /*VECTOR_ADD(v, queueEl);
    queueEl_t *xd = (queueEl_t *)malloc(sizeof(queueEl_t));
    xd = vector_get(&v, 0);
    VECTOR_DELETE(v, 0);
    printf("%d\n", xd->newP->ts);

    for (i = 0; i < VECTOR_TOTAL(v); i++)
        printf("%d ", (int)VECTOR_TOTAL(v));
    printf("\n");

    // VECTOR_DELETE(v, 3);
    // VECTOR_DELETE(v, 2);
    VECTOR_DELETE(v, 1);

    VECTOR_SET(v, 0, "Hello");
    VECTOR_ADD(v, "World");*/

	VECTOR_ADD(v, 10);

    for (i = 0; i < VECTOR_TOTAL(v); i++)
        printf("%d ", VECTOR_GET(v, int*, i));
>>>>>>> c60f3d0cb438edfd068a91f39edc0bfdf9b8d964
    printf("\n");
    VECTOR_FREE(v);
}
