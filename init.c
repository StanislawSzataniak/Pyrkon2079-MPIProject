#include "main.h"

int rank, size, lamportTimer, wkspNumber, wkspTicketsNumber, ticketsNumber, pyrkonNumber, incrementedAck, gotTicketInfoAck, hostAck, pyrkonExit;
bool isHost;

int *tickets_agreements_array;
int **workshops_agreements_array;

vector queue;
vector hosts;
vector pTicketQueue;
vector wTicketQueue;
request_t hostRequest;
request_t pTicketRequest;
request_t wTicketRequest[WORKSHOP_NUMBER];
pthread_t threadDelay;

pthread_mutex_t timerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ticketMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t packetMut = PTHREAD_MUTEX_INITIALIZER;
sem_t pyrkonHostSem, pyrkonStartSem, pyrkonIncrementedSem, everyoneGotTicketInfoSem, pyrkonTicketSem, workshopTicketSem, endPyrkonSem;
//czy zamiana na pthread_cond_t
pthread_t ticketsThread, communicationThread;

void updateRequests(packet_t *data, int type) {
    if (type == WANT_TO_BE_HOST) {
        hostRequest.ts = data->ts;
        hostRequest.src = data->src;
    } else if (type == WANT_PYRKON_TICKET) {
        if (pTicketRequest.ts == INT_MAX) {
            pTicketRequest.ts = data->ts;
            pTicketRequest.src = data->src;
        }
    } else if (type == WANT_WORKSHOP_TICKET) {
        if (wTicketRequest[data->wkspNumber].ts == INT_MAX) {
            wTicketRequest[data->wkspNumber].ts = data->ts;
            wTicketRequest[data->wkspNumber].src = data->src;
        }
    }
}

void sendPacket(packet_t *data, int dst, int type) {

    
    
    //updateRequests(data, type);


    // data->pyrkonNumber = pyrkonNumber;
    packet_t *newP = (packet_t *)malloc(sizeof(packet_t));
    memcpy(newP, data, sizeof(packet_t));
    queueEl_t *queueEl = (queueEl_t *)malloc(sizeof(queueEl_t));
    queueEl->dst = dst;
    queueEl->type = type;
    queueEl->newP = newP;
    VECTOR_ADD(queue, queueEl);

}

void *delayFunc(void *ptr) {
    while (!end) {
        int percent = (rand()%500 + 200);
        //struct timespec t = { 0, percent*25000000 };
        //struct timespec rem = { 1, 0 };
	    //nanosleep(&t,&rem);
        usleep(percent);
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
    return 0;
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
    incrementedAck = 0;
    gotTicketInfoAck = 0;
    hostAck = 0;
    pyrkonExit = 0;
    isHost = false;
    hostRequest.ts = INT_MAX;
    pTicketRequest.ts = INT_MAX;
    int i;
    for (int i = 0; i < WORKSHOP_NUMBER; i++) {
        wTicketRequest[i].ts = INT_MAX;
    }
    
    srand(rank+time(NULL));

    // initialize semaphores
    sem_init(&pyrkonHostSem, 0, 0);
    sem_init(&pyrkonStartSem, 0, 0);
    sem_init(&pyrkonIncrementedSem, 0, 0);
    sem_init(&everyoneGotTicketInfoSem, 0, 0);
    sem_init(&pyrkonTicketSem, 0, 0);
    sem_init(&workshopTicketSem, 0, 0);
    sem_init(&endPyrkonSem, 0, 0);
    vector_init(&queue);
    vector_init(&hosts);
    vector_init(&pTicketQueue);
    vector_init(&wTicketQueue);

    //init handlers
    
    //printf("COMM THREAD %d\n", rank);
    // init communication thread
    pthread_create(&communicationThread, NULL, comFunc, 0);
    pthread_create(&threadDelay, NULL, delayFunc, 0);
}

void finalize(void){
    pthread_mutex_destroy(&timerMutex);
    pthread_mutex_destroy(&packetMut);
    pthread_mutex_destroy(&ticketMutex);
    pthread_join(communicationThread, NULL);
    pthread_join(threadDelay, NULL);
    // pthread_join(ticketsThread, NULL);
    sem_destroy(&pyrkonHostSem);
    sem_destroy(&pyrkonStartSem); 
    sem_destroy(&pyrkonIncrementedSem);
    sem_destroy(&pyrkonTicketSem);
    sem_destroy(&everyoneGotTicketInfoSem);
    sem_destroy(&workshopTicketSem);
    sem_destroy(&endPyrkonSem);
    VECTOR_FREE(queue);
    VECTOR_FREE(hosts);
    VECTOR_FREE(pTicketQueue);
    VECTOR_FREE(wTicketQueue);
    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}
