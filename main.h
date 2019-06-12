#ifndef MAINH
#define MAINH

#include <mpi.h>
#include <cstdlib>
#include <cstdio> 
#include <climits>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctime>
#include <algorithm>
#include <string>

using namespace std;

typedef struct hostRequest{
    int processId;
    int TS;
} hostRequest;

typedef struct {
    int lTimer;              
    int workshopNumber;
    int ticketsNumber;       
    int dst;                
    int src;                
} packet_t;
#define FIELDNO 5

extern bool jestPyrkon;

//Messages structs
extern MPI_Datatype MPI_PACKET_T;

//Messages types
#define PYRKON_START 1
#define PYRKON_END 2
#define PYRKON_TICKETS 3
#define WORKSHOPS_TICKETS 4
#define WANT_PYRKON_TICKET 5
#define CONF_WANT_PYRKON_TICKET 6
#define WANT_WORKSHOP_TICKET 7
#define CONF_WANT_WORKSHOP_TICKET_ACK 8

extern int PID, size, jestPyrkon, workshopsNumber, lamportTimer;
#define MIN_WORKSHOPS 5
#define MAX_WORKSHOPS 12

extern pthread_mutex_t timerMutex;
extern sem_t pyrkonHostSem, pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, pyrkonNumberIncrementedSem, workshopTicketSem;
extern pthread_t ticketsThread;

extern void * prepareAndSendTicketsDetails(void *);
extern void sendPacket(packet_t *, int, int);