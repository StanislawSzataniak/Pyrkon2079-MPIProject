#include "main.h"

int PID, size, lamportTimer, jestPyrkon, workshopsNumber;


pthread_mutex_t timerMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, pyrkonEndSem, workshopTicketSem;
pthread_t communicationThread, ticketsThread;

extern void * comFunc(void *);

MPI_Datatype MPI_PACKET_T;

void initializeMPI(int argc, char *argv[]) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);
}

void createMPIDataTypes() {
    //MPI_PACKET_T
    const int nitems = FIELDNO;                    
    int blocklengths[FIELDNO] = {1, 1, 1, 1, 1};          
    MPI_Aint offsets[FIELDNO];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, workshopNumber);
    offsets[2] = offsetof(packet_t, ticketsNumber);
    offsets[3] = offsetof(packet_t, dst);
    offsets[4] = offsetof(packet_t, src);                        
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PACKET_T);
    MPI_Type_commit(&MPI_PACKET_T);
}

void initializeLamportTimer() {
    lamportTimer = 0; 
}

void runThreads() {
    pthread_create(&communicationThread, NULL, comFunc, 0);
}

void initializeSemaphores() {
    sem_init(&pyrkonStartSem, 0, 0);
    sem_init(&everyoneGetsTicketsInfoSem, 0, 0);
    sem_init(&pyrkonTicketSem, 0, 0);
    sem_init(&pyrkonNumberIncrementedSem, 0, 0);
    sem_init(&workshopTicketSem, 0, 0);
}

void initialize(int argc, char *argv[]) {
    
    initializeMPI(argc, argv);
    createMPIDataTypes();
    initializeLamportTimer();
    srand(processId);
    initializeSemaphores();
    println("Process initialized");
    runThreads();
}

void finalize(void) {
    pthread_mutex_destroy(&timerMutex);
    pthread_join(communicationThread, NULL);
    pthread_join(ticketsThread, NULL);
    sem_destroy(&pyrkonStartSem);
    sem_destroy(&pyrkonTicketSem);

    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}