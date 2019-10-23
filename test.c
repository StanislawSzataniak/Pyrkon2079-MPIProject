#include <stdio.h>
#include <stdlib.h>

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
    packet_t packet;
    packet.ts = 2;
    
    queueEl_t *queueEl = (queueEl_t *)malloc(sizeof(queueEl_t));
    queueEl->newP = &packet;
    queueEl->dst = 10;

    VECTOR_INIT(v);

    VECTOR_ADD(v, queueEl);
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
    VECTOR_ADD(v, "World");

    for (i = 0; i < VECTOR_TOTAL(v); i++)
        printf("%s ", VECTOR_GET(v, char*, i));
    printf("\n");
    VECTOR_FREE(v);
}
