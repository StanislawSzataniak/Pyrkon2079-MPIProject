#include "main.h"
#include "vector.h"

int rank, size, lamportTimer, wkspNumber, wkspTicketsNumber, ticketsNumber, pyrkonNumber;

int *tickets_agreements_array;
int **workshops_agreements_array;

vector queue;

pthread_mutex_t timerMutex;
pthread_mutex_t packetMut = PTHREAD_MUTEX_INITIALIZER;
sem_t pyrkonStartSem, everyoneGetsTicketsInfoSem, pyrkonTicketSem, workshopTicketSem;
//czy zamiana na pthread_cond_t
pthread_t ticketsThread, communicationThread;

void sendPacket(packet_t *data, int dst, int type) {

    pthread_mutex_lock(&timerMutex);
        data->ts = ++lamportTimer;
    pthread_mutex_unlock(&timerMutex);

    // data->pyrkonNumber = pyrkonNumber;
    packet_t *newP = (packet_t *)malloc(sizeof(packet_t));
    memcpy(newP, data, sizeof(packet_t));
    queueEl_t *queueEl = (queueEl_t *)malloc(sizeof(queueEl_t));
    queueEl->dst = dst;
    queueEl->type = type;
    queueEl->newP = newP;
    VECTOR_ADD(queue, queueEl);

}

void delayFunc(void *ptr) {
    while (!end) {
        int percent = (rand()%5 + 1);
        if (!rank) percent = 0;
        struct timespec t = { 0, percent*25000000 };
        struct timespec rem = { 1, 0 };
        if (rank)
	    nanosleep(&t,&rem);
        pthread_mutex_lock( &packetMut );
        queueEl_t *queueEl = vector_get(&queue, 0);
        VECTOR_DELETE(queue, 0);
        pthread_mutex_unlock( &packetMut );
        if (!end && queueEl) {
            MPI_Send(queueEl->newP, 1, MPI_PACKET_T, queueEl->dst, queueEl->type, MPI_COMM_WORLD);
            free(queueEl->newP);
            free(queueEl);
        }
    }
    
}

void initialize(int *argc, char ***argv) {
    // initialize MPI
    int provided;
    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    // check_thread_support(provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // initialize MPI struct
    const int nitems = FIELDNO;                    
    int blocklengths[FIELDNO] = {1, 1, 1, 1, 1, 1, 1, 1};          
    MPI_Aint offsets[FIELDNO];
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, ticketsNumber);
    offsets[2] = offsetof(packet_t, pyrkonNumber);
    offsets[3] = offsetof(packet_t, wkspshopsNumber);
    offsets[4] = offsetof(packet_t, wkspTicketsNumber);
    offsets[5] = offsetof(packet_t, wkspNumber);
    offsets[6] = offsetof(packet_t, dst);
    offsets[7] = offsetof(packet_t, src);                        
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};     /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PACKET_T);
    MPI_Type_commit(&MPI_PACKET_T);

    // initialize stuff
    lamportTimer = 0;
    pyrkonNumber = 0; 

    // initialize semaphores
    sem_init(&pyrkonStartSem, 0, 0);
    sem_init(&everyoneGetsTicketsInfoSem, 0, 0);
    sem_init(&pyrkonTicketSem, 0, 0);
    sem_init(&workshopTicketSem, 0, 0);
    vector_init(&queue);

    //init handlers
    
    printf("COMM THREAD\n");
    // init communication thread
    pthread_create(&communicationThread, NULL, comFunc, 0);
}

void finalize(void){
    pthread_mutex_destroy(&timerMutex);
    pthread_join(communicationThread, NULL);
    // pthread_join(ticketsThread, NULL);
    sem_destroy(&pyrkonStartSem);
    sem_destroy(&pyrkonTicketSem);
    sem_destroy(&everyoneGetsTicketsInfoSem);
    sem_destroy(&workshopTicketSem);
    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}
