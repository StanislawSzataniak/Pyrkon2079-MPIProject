#include "main.h"

int rank, size, lamportTimer, wkspNumber, wkspTicketsNumber, ticketsNumber;

int tickets_agreements_array;
int workshops_agreements_array;

pthread_mutex_t timerMutex;
sem_t pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, workshopTicketSem;
//czy zamiana na pthread_cond_t
pthread_t ticketsThread, communicationThread;

void initialize(int *argc, char ***argv) {
    // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    // check_thread_support(provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // initialize MPI struct
    const int nitems = FIELDNO;                    
    int blocklengths[FIELDNO] = {1, 1, 1, 1, 1, 1, 1};          
    MPI_Aint offsets[FIELDNO];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, ticketsNumber);
    offsets[2] = offsetof(packet_t, wkspshopsNumber);
    offsets[3] = offsetof(packet_t, wkspTicketsNumber);
    offsets[4] = offsetof(packet_t, wkspNumber);
    offsets[5] = offsetof(packet_t, dst);
    offsets[6] = offsetof(packet_t, src);                        
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};     /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PACKET_T);
    MPI_Type_commit(&MPI_PACKET_T);

    // initialize lamportTimer
    lamportTimer = 0; 

    // initialize semaphores
    sem_init(&pyrkonStartSem, 0, 0);
    sem_init(&everyoneGetsTicketsInfoSem, 0, 0);
    sem_init(&pyrkonTicketSem, 0, 0);
    sem_init(&workshopTicketSem, 0, 0);

    //init handlers
    

    // init communication thread
    pthread_create(&communicationThread, NULL, comFunc, 0);
}

void finalize(void){
    pthread_mutex_destroy(&timerMutex);
    pthread_join(communicationThread, NULL);
    pthread_join(ticketsThread, NULL);
    sem_destroy(&pyrkonStartSem);
    sem_destroy(&pyrkonTicketSem);
    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}