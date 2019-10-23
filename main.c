#include "main.h"



ticket_t pyrkonTicket;
int idx;
int currentWorkshop;

void endPyrkonHandler(packet_t *packet);
void wantToBeHostHandler(packet_t *pakiet);
void startPyrkonHandler(packet_t *packet);
void pyrkonNumberIncremented(packet_t *packet);
void workshopsTicketsHandler(packet_t *packet);
void pyrkonTicketsHandler(packet_t *packet);
void gotTicketInfoHandler(packet_t *packet);
void wantPyrkonTicketHandler(packet_t* packet);
void wantPyrkonTicketAckHandler(packet_t *packet);
void wantWorkshopTicketHandler(packet_t *packet);
void wantWorkshopTicketAckHandler(packet_t *packet);
void pyrkonEnterHandler(packet_t *packet);
void freePyrkonTicketHandler(packet_t *packet);

void sendToEveryoneBut(packet_t *packet, int message, int sender);
void sendToEveryone(packet_t *packet, int message);

void becomeHost();
void startPyrkon();
void getTicketsInfo();
void getPyrkonTicket();
<<<<<<< HEAD
void getWorkshopTicket();
=======
>>>>>>> c60f3d0cb438edfd068a91f39edc0bfdf9b8d964
void freePyrkonTicket();

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
            [GOT_TICKETS_INFO]  = gotTicketInfoHandler,
            [WANT_PYRKON_TICKET] = wantPyrkonTicketHandler,
            [WANT_PYRKON_TICKET_ACK] = wantPyrkonTicketAckHandler,
            [WANT_WORKSHOP_TICKET] = wantWorkshopTicketHandler,
            [WANT_WORKSHOP_TICKET_ACK] = wantWorkshopTicketAckHandler,
            [PYRKON_ENTER] = pyrkonEnterHandler,
            [FREE_TICKET] = freePyrkonTicketHandler};

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
        printf("%d %d %d %d\n", req->ts, hostRequest.ts, req->src, rank);
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
    getTicketsInfo();
    getPyrkonTicket();
<<<<<<< HEAD
    getWorkshopTicket();
    //freePyrkonTicket();
=======
    freePyrkonTicket();
>>>>>>> c60f3d0cb438edfd068a91f39edc0bfdf9b8d964
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
    ticketsNumber = packet->ticketsNumber;
    wkspNumber = packet->wkspshopsNumber;
    wkspTicketsNumber = packet->wkspTicketsNumber;
    pyrkonTicket.number = ticketsNumber;
    sendToEveryoneBut(packet, GOT_TICKETS_INFO, rank);
    
}
void gotTicketInfoHandler(packet_t *packet) {
    gotTicketInfoAck++;
    if (gotTicketInfoAck == size - 1) {
        printf("%s %d\n", "MAM INFO", rank);
        sem_post(&everyoneGotTicketInfoSem);
    }
}
void wantPyrkonTicketHandler(packet_t* packet) {
    printf("%s %d Moje ID: %d Liczba biletów: %d Czas jego: %d Czas mój: %d\n", "Dostałem żądanie od", packet->src, rank, pyrkonTicket.number, packet->ts, pTicketRequest.ts);
    if (pyrkonTicket.want) {
        if (!pyrkonTicket.has && (packet->ts < pTicketRequest.ts || (packet->ts == pTicketRequest.ts && packet->src < pTicketRequest.src))) {
            pthread_mutex_lock(&timerMutex);
            packet->ts = ++lamportTimer;
            updateRequests(packet, WANT_PYRKON_TICKET_ACK);
            pthread_mutex_unlock(&timerMutex);
            sendPacket(packet, packet->src, WANT_PYRKON_TICKET_ACK);
        } /*else {
            request_t *req = (request_t *)malloc(sizeof(request_t));
            req->ts = packet->ts;
            req->src = packet->src;
            //printf("WARTOŚĆ A: %d\n", req->src);
            vector_add(&pTicketQueue, req);
        }*/
    } else {
        pthread_mutex_lock(&timerMutex);
        packet->ts = ++lamportTimer;
        updateRequests(packet, WANT_PYRKON_TICKET_ACK);
        pthread_mutex_unlock(&timerMutex);
        sendPacket(packet, packet->src, WANT_PYRKON_TICKET_ACK);
    }
}
void wantPyrkonTicketAckHandler(packet_t *packet) {
    if (pyrkonTicket.want && !pyrkonTicket.has) {
        pyrkonTicket.confs++;
        printf("LICZBA BILETOW %d, RANK: %d, ACK: %d\n", pyrkonTicket.number, rank, pyrkonTicket.confs);
        if (pyrkonTicket.number == 0)
            printf("Czekam na bilety, %d\n", rank);
        pthread_mutex_lock(&ticketMutex);
        if (pyrkonTicket.confs == size - pyrkonTicket.number && pyrkonTicket.number>0) {
            pyrkonTicket.has = true;
            pyrkonTicket.confs = 0;
            pyrkonTicket.requestTS = INT_MAX;
            pyrkonTicket.number--;
            sendToEveryoneBut(packet, PYRKON_ENTER, rank);
            printf("%s %d\n", "WSZEDŁEM", rank);
            sem_post(&pyrkonTicketSem);
        }
        pthread_mutex_unlock(&ticketMutex);
   }
}

void pyrkonEnterHandler(packet_t *packet) {
    pthread_mutex_lock(&ticketMutex);
    pyrkonTicket.number--;
    pthread_mutex_unlock(&ticketMutex);
}

void wantWorkshopTicketHandler(packet_t *packet) {
    
}
void wantWorkshopTicketAckHandler(packet_t *packet){
    
}

void freePyrkonTicketHandler(packet_t *packet) {
    printf("CZY ABY NA PEWNO %d, ZRODLO %d, MOJ %d\n", pyrkonTicket.number, packet->src, rank);
    pthread_mutex_lock(&ticketMutex);
    pyrkonTicket.number++;
    pyrkonTicket.confs = 0;
    pthread_mutex_unlock(&ticketMutex);
    printf("PO WYJSCIU %d ZOSTAŁO %d BILETOW, ID: %d\n", packet->src, pyrkonTicket.number, rank);
    packet_t newPacket;
    newPacket.src = rank;
    newPacket.pyrkonNumber = pyrkonNumber;
    //printf("%s %d %d\n", "Chcę bilet", lamportTimer, rank);
    sendToEveryoneBut(&newPacket, WANT_PYRKON_TICKET, rank);
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

void getTicketsInfo() {
    if (isHost) {
        int pTickets = (rand()%size)/2 + 1;
        int wNumber = WORKSHOP_NUMBER;
        int wTickets = NUMBER_OF_WORKSHOP_TICKETS;
        
        packet_t packet;
        packet.src = rank;
        packet.pyrkonNumber = pyrkonNumber;
        packet.ticketsNumber = pTickets;
        packet.wkspshopsNumber = wNumber;
        packet.wkspTicketsNumber = wTickets;
        
        sendToEveryone(&packet, PYRKON_TICKETS);
    }
    sem_wait(&everyoneGotTicketInfoSem);
}

void getPyrkonTicket() {
    int delay = (rand()%1000 + 1);
    usleep(delay);
    packet_t packet;
    packet.src = rank;
    packet.pyrkonNumber = pyrkonNumber;
    pyrkonTicket.want = true;
    pyrkonTicket.has = false;
    pyrkonTicket.visited = false;
    //printf("%s %d %d\n", "Chcę bilet", lamportTimer, rank);
    sendToEveryoneBut(&packet, WANT_PYRKON_TICKET, rank);
    sem_wait(&pyrkonTicketSem);
}

<<<<<<< HEAD
void getWorkshopTicket() {
    int wantToAttendNumber = rand()%(wkspNumber - 1) + 1;
    int i;
    for (i = 0; i < wkspNumber; i++) {
        ticket_t *wTicket = (ticket_t *)malloc(sizeof(ticket_t));
        wTicket->number = wkspTicketsNumber;
        wTicket->workshopNumber = i;
        wTicket->confs = 0;
        vector_add(&wTicketQueue, wTicket);
    }
    while (wantToAttendNumber > 0) {
        int j = rand()%VECTOR_TOTAL(wTicketQueue);
        ticket_t *ticket = (ticket_t *)malloc(sizeof(ticket_t));
        ticket = vector_get(&wTicketQueue, j);
        currentWorkshop = ticket->workshopNumber;
        printf("Current workshop: %d\n", currentWorkshop);
        packet_t packet;
        packet.packet.src = rank;
        packet.pyrkonNumber = pyrkonNumber;
        wantToAttendNumber--;
    }
}

void freePyrkonTicket() {
    pthread_mutex_lock(&ticketMutex);
	pyrkonTicket.has = false;
    pyrkonTicket.want = false;
    pyrkonTicket.visited = true;
    pyrkonTicket.number++;
    pthread_mutex_unlock(&ticketMutex);
    packet_t packet;
    packet.src = rank;
    packet.pyrkonNumber = pyrkonNumber;
    sendToEveryoneBut(&packet, FREE_TICKET, rank);
	/*while(vector_get(&pTicketQueue, 0)) {
		packet_t pakiet;
        pakiet.src = rank;
        pakiet.pyrkonNumber = pyrkonNumber;
        request_t *req = (request_t *)malloc(sizeof(request_t));
        memcpy(req, vector_get(&pTicketQueue, 0), sizeof(request_t));
        //printf("SENDER ID %d, MOJE ID: %d\n", req->src, rank);
		sendPacket(&pakiet, req->src, FREE_TICKET);
		vector_delete(&pTicketQueue, 0);
	}*/
	printf("%d zwalnia bilet\n", rank);
    sem_wait(&endPyrkonSem);
=======
void freePyrkonTicket() {
	pyrkonTicket.want = false;
	pyrkonTicket.has = false;
	while(!&pTicketQueue.total > 0) {
		packet_t pakiet;
    		pakiet.src = rank;
    		pakiet.pyrkonNumber = pyrkonNumber;
		sendPacket(&pakiet, vector_get(&pTicketQueue, 0), WANT_PYRKON_TICKET_ACK);
		vector_delete(&pTicketQueue, 0);
		printf("%d zwalnia bilet\n", rank);
	}
>>>>>>> c60f3d0cb438edfd068a91f39edc0bfdf9b8d964
}
