#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    int ts;                 /* zegar lamporta */
    int ticketsNumber;      /* liczba biletów na obecny Pyrkon */
    int wkspshopsNumber;    /* liczba warsztatów na obecnym Pyrkonie */
    int wkspTicketsNumber;  /* liczba miejsc na warsztatach */
    int wkspNumber;         /* opcjonalny numer warsztatu na który uczestnik chce zdobyć bilet */
    int dst;                /* pole ustawiane w sendPacket */
    int src;                /* pole ustawiane w wątku komunikacyjnym na processId nadawcy */
} packet_t;
#define FIELDNO 7  //liczba pól w strukturze packet_t

#define PYRKON_START 1
#define PYRKON_TICKET 2
#define WORKSHOP_TICKET 3
#define PYRKON_TICKET_ACK 4
#define WORKSHOP_TCIKET_ACK 5
#define PYRKON_ENTER 6
#define WORKSHOP_ENTER 7

extern MPI_Datatype MPI_PACKET_T;

extern int processId, size, ticketsNumber, wkspNumber, wkspTicketsNumber, lamportTimer;
#define MIN_WORKSHOPS 3
#define MAX_WORKSHOPS 8

extern pthread_mutex_t timerMutex;
extern sem_t pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, workshopTicketSem;
extern pthread_t ticketsThread;

extern void * prepareAndSendTicketsDetails(void *);
extern void sendPacket(packet_t *, int, int);

#endif
