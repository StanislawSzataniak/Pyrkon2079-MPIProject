#ifndef MAINH
#define MAINH

#define TRUE 1
#define FALSE 0

#define FIELDNO 8

#define WANT_TO_BE_HOST 1
#define PYRKON_START 2
#define PYRKON_NUMBER_INCREMENTED 3
#define PYRKON_TICKETS 4
#define WORKSHOPS_TICKETS 5
#define GOT_TICKETS_INFO 6
#define WANT_PYRKON_TICKET 7
#define WANT_PYRKON_TICKET_ACK 8
#define WANT_WORKSHOP_TICKET 9
#define WANT_WORKSHOP_TICKET_ACK 10
#define PYRKON_END 11
#define PYRKON_ENTER 12
#define FREE_TICKET 13

#define MAX_HANDLERS 14

#define VECTOR_INITIAL_CAPACITY 16
#define PYRKON_TICKET_NUMBER 10

#define NUMBER_OF_WORKSHOP_TICKETS 2
#define WORKSHOP_NUMBER 3
#define MIN_WORKSHOPS 3
#define MAX_WORKSHOPS 8

#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <limits.h>
#include "vector.h"

extern MPI_Datatype MPI_PACKET_T;

typedef struct {
    bool want;              /* czy proces ubiega się o bilet */
    bool has;               /* czy proces ma już bilet */
    bool visited;
    int confs;              /* liczba zgód na zajęcie biletu */
    int number;             /* liczba dostępnych biletów na pyrkon */
    int requestTS;
    int workshopNumber;
} ticket_t;

typedef struct {
    int ts;
    int src;
} request_t;

typedef struct {
    int ts;                 /* zegar lamporta */
    int requestTS;          /* zegar lamporta przy pierwszej wiadomości */
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


extern int rank; //rank
extern int size;
extern int ticketsNumber;
extern int wkspNumber;
extern int wkspTicketsNumber;
extern int lamportTimer;
extern int pyrkonNumber;
extern int incrementedAck;
extern int gotTicketInfoAck;
extern bool isHost;

extern volatile char end;
extern vector hosts;
extern vector pTicketQueue;
extern vector wTicketQueue;
extern request_t hostRequest;
extern request_t pTicketRequest;
extern request_t wTicketRequest;


extern pthread_mutex_t timerMutex, ticketMutex;
extern sem_t pyrkonHostSem, pyrkonStartSem, pyrkonIncrementedSem, everyoneGotTicketInfoSem, pyrkonTicketSem, workshopTicketSem, endPyrkonSem;
//czy zamiana na pthread_cond_t
extern pthread_t ticketsThread;

extern void *comFunc(void *);
extern void *prepareAndSendTicketsDetails(void *);
extern void updateRequests(packet_t *data, int type);
extern void sendPacket(packet_t *, int, int);

typedef void (*f_w)(packet_t*);


#endif
