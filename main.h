#ifndef MAINH
#define MAINH

#define FIELDNO 7
#define PYRKON_START 1
#define PYRKON_TICKET 2
#define WORKSHOP_TICKET 3
#define PYRKON_TICKET_ACK 4
#define WORKSHOP_TCIKET_ACK 5
#define PYRKON_ENTER 6
#define WORKSHOP_ENTER 7
#define MIN_WORKSHOPS 3
#define MAX_WORKSHOPS 8
#define MAX_HANDLERS 10
#define FINISH 11
#define VECTOR_INITIAL_CAPACITY 16

#include<mpi.h>
#include<pthread.h>
#include<stdio.h>
#include <stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<semaphore.h>
#include"vector.h"

extern MPI_Datatype MPI_PACKET_T;

typedef struct {
    bool want;              /* czy proces ubiega się o bilet */
    bool has;               /* czy proces ma już bilet */
    int confs;              /* liczba zgód na zajęcie biletu */
    int number;             /* liczba dostępnych biletów na pyrkon */
    int requestTS;
} ticket;

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
    int size;
    int capacity;
    packet_t *data;
} Vector;

void init(Vector *vector);
void free_memory(Vector *vector);
void add_sort(Vector* vector, packet_t new_item);
int size(Vector *vector);
int capacity(Vector *vector);
int my_latest_position_in_queue(Vector *vector, int my_rank);

extern int rank; //rank
extern int size;
extern int ticketsNumber;
extern int wkspNumber;
extern int wkspTicketsNumber;
extern int lamportTimer;
extern int pyrkonNumber;


extern int* tickets_agreements_array;
extern int* workshops_agreements_array;

extern pthread_mutex_t timerMutex;
extern sem_t pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, workshopTicketSem;
//czy zamiana na pthread_cond_t
extern pthread_t ticketsThread;

extern void *comFunc(void *);
extern void *prepareAndSendTicketsDetails(void *);
extern void sendPacket(packet_t *, int, int);

typedef void (*f_w)(packet_t*);
// TODO
// f_w handlers[MAX_HANDLERS] = {

// }

#endif