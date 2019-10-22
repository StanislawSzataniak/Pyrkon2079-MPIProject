#include "main.h"



ticket_t pyrkonTicket;
ticket_t workshops[3];

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

void becomeHost();

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
    int i;
    request_t *req;
    for (i = 0; i < VECTOR_TOTAL(hosts); i++) {
        //printf("i: %d\n", i);
        req = vector_get(&hosts, i);
        int ts = req->ts;
        int src = req->src;
        //printf("%d %d\n", src, rank);
        if (src < hostRequest.src) {
            host = false;
        }
    }
    if (host) isHost = true;
    if (!host) isHost = false;
}

void finishHandler(packet_t *pakiet)
{
    printf("%s", "Otrzymałem FINISH\n" );
    end = TRUE; 
}

void *comFunc(void *ptr) {
    MPI_Status status;
    packet_t packet;;
    int flaga=0;

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



/*

void pyrkonTicketRequestHandler(packet_t *packet)
{
    int senderId = packet->src;
    if (pyrkonTicket.want)
    {
        if (!pyrkonTicket.has && (pyrkonTicket.requestTS > packet->ts || (pyrkonTicket.requestTS == packet->ts && rank > senderId))) {
            sendPacket(packet, packet -> dst, PYRKON_TICKET_ACK);
	}
    }
    else {
        sendPacket(packet, packet -> dst, PYRKON_TICKET_ACK);
    }
}

void pyrkonResponseHandler(packet_t *packet) {
	pyrkonTicket.want = true;
	pyrkonTicket.confs++;
	if(pyrkonTicket.confs == size ) {
		sendPacket(packet, packet -> dst, PYRKON_ENTER);
	}
}

void freePyrkonTicket(packet_t *packet) {
	pyrkonTicket.confs = 0;
	sendPacket(packet, packet -> dst, FINISH);
}

void workshopRequestHandler(packet_t *packet)
{
    int senderId = packet->src;
    if (workshops[packet -> wkspNumber].want)
    {
        if (!workshops[packet -> wkspNumber].has && (workshops[packet -> wkspNumber].requestTS > packet->ts || (workshops[packet -> wkspNumber].requestTS == packet->ts && rank > senderId))) {
            sendPacket(packet, packet -> dst, PYRKON_TICKET_ACK);
	}
    }
    else {
        sendPacket(packet, packet -> dst, PYRKON_TICKET_ACK);
    }
}

void workshopResponseHandler(packet_t* packet) {
	workshops[packet -> wkspNumber].confs++;
	if(workshops[packet -> wkspNumber].confs == size ) {
		sendPacket(packet, packet -> dst, WORKSHOP_TCIKET_ACK);
	}	
}

void freeWorkshop(packet_t *packet) {
	workshops[packet -> wkspNumber].want = false;
	workshops[packet -> wkspNumber].has = false;
	workshops[packet -> wkspNumber].confs = 0;
	sendPacket(packet, packet -> dst, WORKSHOP_END );
}
*/

int main(int argc, char **argv) {
    initialize(&argc, &argv);
    /*packet_t packet;
    packet.ts = 1;
    packet.pyrkonNumber = 0;
    while (!end) {
        //printf("%s %d %s", "SendPacket", rank, "\n");
        sendToEveryoneBut(&packet, WANT_TO_BE_HOST, rank);
    }*/
    becomeHost();
    request_t *request0 = vector_get(&hosts, 0);
    request_t *request1 = vector_get(&hosts, 1);
    printf("%d %d %d\n", VECTOR_TOTAL(hosts), request0->src, request1->src);
    finalize();
    return 0;
}

void endPyrkonHandler(packet_t *packet){
    printf("%s", "Otrzymałem FINISH\n" );
    end = TRUE; 
}
void wantToBeHostHandler(packet_t *packet){
    request_t hRequest;
    hRequest.ts = packet->ts;
    hRequest.src = packet->src;
    vector_add(&hosts, &hRequest);
    //printf("%d %d %d\n", hRequest.ts, hRequest.src, rank);
    
    if (VECTOR_TOTAL(hosts) == size-1) {
        canIBeHost();
        if (isHost) {
            printf("%s%d\n", "I am the host", rank);
        } else {
            printf("%s%d\n", "I am not the host", rank);
        }
        //VECTOR_FREE(hosts);
        sem_post(&pyrkonHostSem);
    }
}
void startPyrkonHandler(packet_t *packet) {
    
}
void pyrkonNumberIncremented(packet_t *packet) {
    
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
    for (dst = 0; dst < size; dst++) {
        if (dst != sender) {
            sendPacket(packet, dst, message);
        }
    }
}

void becomeHost() {
   packet_t packet;
   packet.src = rank;
   packet.pyrkonNumber = pyrkonNumber;
   sendToEveryoneBut(&packet, WANT_TO_BE_HOST, rank);
   sem_wait(&pyrkonHostSem);
}
