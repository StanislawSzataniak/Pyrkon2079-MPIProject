#include "main.h"



ticket_t pyrkonTicket;
ticket_t workshops[3];
int idx;

void endPyrkonHandler(packet_t *packet);
void wantToBeHostHandler(packet_t *pakiet);
void startPyrkonHandler(packet_t *packet);
void pyrkonNumberIncremented(packet_t *packet);
void workshopsTicketsHandler(packet_t *packet);
void pyrkonTicketsHandler(packet_t *packet);
void gotTicketsInfoHandler(packet_t *packet);
void wantPyrkonTicketHandler(packet_t* packet);
void wantPyrkonTicketAckHandler(packet_t *packet);
void wantWorkshopTicketHandler(packet_t *packet);
void wantWorkshopTicketAckHandler(packet_t *packet);

void sendToEveryoneBut(packet_t *packet, int message, int sender);
void sendToEveryone(packet_t *packet, int message);

void becomeHost();
void startPyrkon();

typedef void (*f_w)(packet_t *);
/* Lista handlerów dla otrzymanych pakietów
   Nowe typy wiadomości dodaj w main.h, a potem tutaj dodaj wskaźnik do 
     handlera.
   Funkcje handleróœ są na końcu pliku. Nie zapomnij dodać
     deklaracji zapowiadającej funkcji!
*/
f_w handlers[MAX_HANDLERS] = {
            [PYRKON_END] = endPyrkonHandler,
            [WANT_TO_BE_HOST] = wantToBeHostHandler,
            [PYRKON_START] = startPyrkonHandler,
            [PYRKON_NUMBER_INCREMENTED] = pyrkonNumberIncremented,
            [PYRKON_TICKETS] = pyrkonTicketsHandler,
            [WORKSHOPS_TICKETS] = workshopsTicketsHandler,
            [GOT_TICKETS_INFO]  = gotTicketsInfoHandler,
            [WANT_PYRKON_TICKET] = wantPyrkonTicketHandler,
            [WANT_PYRKON_TICKET_ACK] = wantPyrkonTicketAckHandler,
            [WANT_WORKSHOP_TICKET] = wantWorkshopTicketHandler,
            [WANT_WORKSHOP_TICKET_ACK] = wantWorkshopTicketAckHandler };

MPI_Datatype MPI_PACKET_T;

volatile char end = FALSE;

extern void initialize(int *argc, char ***argv);
extern void finalize(void);

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

void canIBeHost() {
    bool host = true;
    for (idx = 0; idx < VECTOR_TOTAL(hosts); idx++) {
        request_t *req = (request_t *)malloc(sizeof(request_t));
        memcpy(req, vector_get(&hosts, idx), sizeof(request_t));
        //printf("%d %d %d %d\n", req->ts, hostRequest.ts, req->src, rank);
        if (req->ts < hostRequest.ts || (req->ts == hostRequest.ts && req->src < hostRequest.src)) {
            host = false;
            break;
        }
    }
    if (host) isHost = true;
}

void finishHandler(packet_t *pakiet)
{
    printf("%s", "Otrzymałem FINISH\n" );
    end = TRUE; 
}

void *comFunc(void *ptr) {
    MPI_Status status;
    packet_t packet;

    while (!end) {
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        
        pthread_mutex_lock(&timerMutex);
            lamportTimer = max(lamportTimer, packet.ts) + 1;
            // println("%d -> %s", status.MPI_SOURCE, getMessageCode(status.MPI_TAG).c_str());
        pthread_mutex_unlock(&timerMutex);

        packet.src = status.MPI_SOURCE;
        
        if (packet.pyrkonNumber == pyrkonNumber) {        //protection from receiving messages from previous Pyrkon
            handlers[(int)status.MPI_TAG](&packet);
        }
    }
    return 0;
}


int main(int argc, char **argv) {
    initialize(&argc, &argv);
    becomeHost();
    startPyrkon();
    finalize();
    return 0;
}

void endPyrkonHandler(packet_t *packet){
    printf("%s", "Otrzymałem FINISH\n" );
    end = TRUE; 
}
void wantToBeHostHandler(packet_t *packet){
    request_t *req = (request_t *)malloc(sizeof(request_t));
    req->ts = packet->ts;
    req->src = packet->src;
    vector_add(&hosts, req);
    
    if (VECTOR_TOTAL(hosts) == size-1) {
        canIBeHost();
        if (isHost) {
            printf("%s %d\n", "I am the host", rank);
        } else {
            printf("%s %d\n", "I am not the host", rank);
        }
        sem_post(&pyrkonHostSem);
    }
}
void startPyrkonHandler(packet_t *packet) {
    packet->pyrkonNumber = ++pyrkonNumber;
    pthread_mutex_lock(&timerMutex);
    packet->ts = ++lamportTimer;
    updateRequests(packet, PYRKON_NUMBER_INCREMENTED);
    pthread_mutex_unlock(&timerMutex);
    sendPacket(packet, packet->src, PYRKON_NUMBER_INCREMENTED);
	sem_post(&pyrkonStartSem);
}
void pyrkonNumberIncremented(packet_t *packet) {
    incrementedAck++;
    if (incrementedAck == size - 1) {
        sem_post(&pyrkonIncrementedSem);
        printf("%s\n", "Incremented");
    }
}
void workshopsTicketsHandler(packet_t *packet) {
    
}

void pyrkonTicketsHandler(packet_t *packet){ 
    
}
void gotTicketsInfoHandler(packet_t *packet) {
    
}
void wantPyrkonTicketHandler(packet_t* packet) {
    
}
void wantPyrkonTicketAckHandler(packet_t *packet) {
    
}
void wantWorkshopTicketHandler(packet_t *packet) {
    
}
void wantWorkshopTicketAckHandler(packet_t *packet){
    
}

void sendToEveryoneBut(packet_t *packet, int message, int sender) {
    int dst;
    pthread_mutex_lock(&timerMutex);
    packet->ts = ++lamportTimer;
    updateRequests(packet, message);
    pthread_mutex_unlock(&timerMutex);
    for (dst = 0; dst < size; dst++) {
        if (dst != sender) {
            sendPacket(packet, dst, message);
        }
    }
}

void sendToEveryone(packet_t *packet, int message) {
    int dst;
    pthread_mutex_lock(&timerMutex);
    packet->ts = ++lamportTimer;
    updateRequests(packet, message);
    pthread_mutex_unlock(&timerMutex);
    for (dst = 0; dst < size; dst++) {
        sendPacket(packet, dst, message);
    }
}

void becomeHost() {
   packet_t packet;
   packet.src = rank;
   packet.pyrkonNumber = pyrkonNumber;
   sendToEveryoneBut(&packet, WANT_TO_BE_HOST, rank);
   sem_wait(&pyrkonHostSem);
}

void startPyrkon() {
	packet_t packet;
   	packet.src = rank;
   	packet.pyrkonNumber = pyrkonNumber;
	if(isHost) {
		sendToEveryoneBut(&packet, PYRKON_START, rank);
        pyrkonNumber++;
        sem_wait(&pyrkonIncrementedSem);
	}
	else {
		sem_wait(&pyrkonStartSem);
	}
	printf("New Pyrkon - %d\n", pyrkonNumber);
}
