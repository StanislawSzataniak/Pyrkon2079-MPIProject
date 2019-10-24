#include "main.h"



ticket_t pyrkonTicket;
ticket_t workshopTickets[WORKSHOP_NUMBER];
int idx;
int currentWorkshop;

void endPyrkonHandler(packet_t *packet);
void wantToBeHostHandler(packet_t *pakiet);
void wantToBeHostHandlerAck(packet_t *pakiet);
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
void getWorkshopTicket();
void freeWorkshopTicket(int wsNumber);
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
            [WANT_TO_BE_HOST_ACK] = wantToBeHostHandlerAck,
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
        //printf("%d %d %d %d\n", req->ts, hostRequest.ts, req->src, rank);
        if (req->ts < hostRequest.ts || (req->ts == hostRequest.ts && req->src < hostRequest.src)) {
            host = false;
            break;
        }
    }
    if (host) isHost = true;
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
    int k;
    for (k = 0; k < 2; k++) {
        becomeHost();
        startPyrkon();
        getTicketsInfo();
        getPyrkonTicket();
        getWorkshopTicket();
        freePyrkonTicket();
        println("KONIEC\n");
    }
    //end = TRUE;
    finalize();
    return 0;
}

void endPyrkonHandler(packet_t *packet){
    pyrkonExit++;
    println("PYRKON EXIT %d %d\n", pyrkonExit, pyrkonTicket.visited)
    if (pyrkonExit >= size && pyrkonTicket.visited) {
        println("OTRZYMAŁEM FINISH\n" );
        int i;
        pyrkonTicket.visited = false;
        for (i = 0; i < wkspNumber; i++) {
            workshopTickets[i].visited = false;
        }
        lamportTimer = 0;
        incrementedAck = 0;
        gotTicketInfoAck = 0;
        hostAck = 0;
        pyrkonExit = 0;
        for (i = 0; i < VECTOR_TOTAL(hosts); i++)
            vector_delete(&hosts, i);
        //end = TRUE;
        sem_post(&endPyrkonSem);
    }
    
    
}
void wantToBeHostHandler(packet_t *packet){
    request_t *req = (request_t *)malloc(sizeof(request_t));
    req->ts = packet->ts;
    req->src = packet->src;
    vector_add(&hosts, req);
    if (VECTOR_TOTAL(hosts) == size-1) {
        canIBeHost();
        if (isHost) {
            println("JESTEM HOSTEM\n");
        } else {
            println("NIE JESTEM HOSTEM\n");
        }
        sendToEveryoneBut(packet, WANT_TO_BE_HOST_ACK, rank);
        sem_post(&pyrkonHostSem);
    }
}

void wantToBeHostHandlerAck(packet_t *pakiet){
    hostAck++;
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
        println("DOSTAŁEM INFORMACJĘ O BILETACH\n");
        sem_post(&everyoneGotTicketInfoSem);
    }
}
void wantPyrkonTicketHandler(packet_t* packet) {
    if (!pyrkonTicket.want) {
        pthread_mutex_lock(&timerMutex);
        packet->ts = ++lamportTimer;
        updateRequests(packet, WANT_PYRKON_TICKET_ACK);
        pthread_mutex_unlock(&timerMutex);
        sendPacket(packet, packet->src, WANT_PYRKON_TICKET_ACK);
    } else {
        if (!pyrkonTicket.has && (packet->ts < pTicketRequest.ts || (packet->ts == pTicketRequest.ts && packet->src < pTicketRequest.src))) {
            pthread_mutex_lock(&timerMutex);
            packet->ts = ++lamportTimer;
            updateRequests(packet, WANT_PYRKON_TICKET_ACK);
            pthread_mutex_unlock(&timerMutex);
            sendPacket(packet, packet->src, WANT_PYRKON_TICKET_ACK);
        } else {
            request_t *req = (request_t *)malloc(sizeof(request_t));
            req->ts = packet->ts;
            req->src = packet->src;
            //printf("WARTOŚĆ A: %d\n", req->src);
            vector_add(&pTicketQueue, req);
        }
    }
}

void wantPyrkonTicketAckHandler(packet_t *packet) {
    if (pyrkonTicket.want && !pyrkonTicket.has) {
        pyrkonTicket.confs++;
        pthread_mutex_lock(&ticketMutex);
        if (pyrkonTicket.confs == size - pyrkonTicket.number) {
            pyrkonTicket.has = true;
            pyrkonTicket.confs = 0;
            //pyrkonTicket.number--;
            //sendToEveryoneBut(packet, PYRKON_ENTER, rank);
            println("WSZEDŁEM NA PYRKON NUMER %d, WYSŁAŁEM ŻĄDANIE %d\n", pyrkonNumber, pTicketRequest.ts);
            pyrkonTicket.requestTS = INT_MAX;
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
    if (!pyrkonTicket.has) {
        pthread_mutex_lock(&timerMutex);
        packet->ts = ++lamportTimer;
        updateRequests(packet, WANT_WORKSHOP_TICKET_ACK);
        pthread_mutex_unlock(&timerMutex);
        sendPacket(packet, packet->src, WANT_WORKSHOP_TICKET_ACK);
    } else {
        if (!workshopTickets[packet->wkspNumber].want) {
            pthread_mutex_lock(&timerMutex);
            packet->ts = ++lamportTimer;
            updateRequests(packet, WANT_WORKSHOP_TICKET_ACK);
            pthread_mutex_unlock(&timerMutex);
            sendPacket(packet, packet->src, WANT_WORKSHOP_TICKET_ACK);
        } else {
            if (!workshopTickets[packet->wkspNumber].has && (packet->ts < wTicketRequest[packet->wkspNumber].ts || (packet->ts == wTicketRequest[packet->wkspNumber].ts && packet->src < wTicketRequest[packet->wkspNumber].src))) {
                pthread_mutex_lock(&timerMutex);
                packet->ts = ++lamportTimer;
                updateRequests(packet, WANT_WORKSHOP_TICKET_ACK);
                pthread_mutex_unlock(&timerMutex);
                sendPacket(packet, packet->src, WANT_WORKSHOP_TICKET_ACK);
            } else {
                request_t *req = (request_t *)malloc(sizeof(request_t));
                req->ts = packet->ts;
                req->src = packet->src;
                req->wkspNumber = packet->wkspNumber;
                //printf("WARTOŚĆ A: %d\n", req->src);
                vector_add(&wTicketQueue, req);
        }
        }
    }
}
void wantWorkshopTicketAckHandler(packet_t *packet){
    if (workshopTickets[packet->wkspNumber].want && !workshopTickets[packet->wkspNumber].has) {
        workshopTickets[packet->wkspNumber].confs++;
        //println("Zgody na warsztat nr %d, %d\n", workshopTickets[packet->wkspNumber].workshopNumber, workshopTickets[packet->wkspNumber].confs);
        if (workshopTickets[packet->wkspNumber].confs == size - wkspTicketsNumber) {
            workshopTickets[packet->wkspNumber].has = true;
            workshopTickets[packet->wkspNumber].confs = 0;
            //pyrkonTicket.number--;
            //sendToEveryoneBut(packet, PYRKON_ENTER, rank);
            println("WSZEDŁEM NA WARSZTAT %d\n", packet->wkspNumber);
            workshopTickets[packet->wkspNumber].requestTS = INT_MAX;
            sem_post(&workshopTicketSem);
        }
    }
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
        while (hostAck < size - 1);
		sendToEveryoneBut(&packet, PYRKON_START, rank);
        pyrkonNumber++;
        sem_wait(&pyrkonIncrementedSem);
	}
	else {
		sem_wait(&pyrkonStartSem);
	}
	println("ROZPOCZYNA SIĘ PYRKON NUMER %d\n", pyrkonNumber);
}

void getTicketsInfo() {
    if (isHost) {
        int pTickets = (rand()%size)/2 + 1;
        int wNumber = WORKSHOP_NUMBER;
        int wTickets = NUMBER_OF_WORKSHOP_TICKETS;
        
        println("LICZBA BILETÓW NA PYRKON: %d, LICZBA WARSZTATÓW: %d, LICZBA BILETÓW NA WARSZTATY: %d\n", pTickets, wNumber, wTickets);
        
        packet_t packet;
        packet.src = rank;
        packet.pyrkonNumber = pyrkonNumber;
        packet.ticketsNumber = pTickets;
        packet.wkspshopsNumber = wNumber;
        packet.wkspTicketsNumber = wTickets;
        isHost = false;
        
        sendToEveryone(&packet, PYRKON_TICKETS);
    }
    sem_wait(&everyoneGotTicketInfoSem);
}

void getPyrkonTicket() {
    println("UBIEGAM SIĘ O BILET NA PYRKON");
    int delay = (rand()%1000 + 1);
    usleep(delay);
    packet_t packet;
    packet.src = rank;
    packet.pyrkonNumber = pyrkonNumber;
    pyrkonTicket.want = true;
    pyrkonTicket.has = false;
    pyrkonTicket.visited = false;
    sendToEveryoneBut(&packet, WANT_PYRKON_TICKET, rank);
    sem_wait(&pyrkonTicketSem);
}

void getWorkshopTicket() {
    int wantToAttendNumber = rand()%(wkspNumber - 1) + 1;
    println("CHCĘ IŚĆ NA %d WARSZTATÓW\n", wantToAttendNumber);
    int i;
    for (i = 0; i < wantToAttendNumber; i++) {
        int wsNumber = rand()%wkspNumber;
        println("WYLOSOWAŁEM %d\n", wsNumber);
        while (workshopTickets[wsNumber].visited)
            wsNumber = rand()%wkspNumber;
        println("IDĘ TERAZ NA WARSZTAT %d\n", wsNumber);
        workshopTickets[wsNumber].want = true;
        workshopTickets[wsNumber].has = false;
        workshopTickets[wsNumber].workshopNumber = wsNumber;
        packet_t packet;
        packet.src = rank;
        packet.pyrkonNumber = pyrkonNumber;
        packet.wkspNumber = wsNumber;
        sendToEveryoneBut(&packet, WANT_WORKSHOP_TICKET, rank);
        sem_wait(&workshopTicketSem);
        usleep(500);
        freeWorkshopTicket(wsNumber);
        
    }
}

void freeWorkshopTicket(int wsNumber) {
    pthread_mutex_lock(&ticketMutex);
    workshopTickets[wsNumber].has = false;
    workshopTickets[wsNumber].want = false;
    workshopTickets[wsNumber].visited = true;
    pthread_mutex_unlock(&ticketMutex);
    while(vector_get(&wTicketQueue, 0)) {
        packet_t pakiet;
        pakiet.src = rank;
        pakiet.pyrkonNumber = pyrkonNumber;
        pakiet.wkspNumber = wsNumber;
        pthread_mutex_lock(&timerMutex);
        pakiet.ts = ++lamportTimer;
        pthread_mutex_unlock(&timerMutex);
        request_t *req = (request_t *)malloc(sizeof(request_t));
        memcpy(req, vector_get(&pTicketQueue, 0), sizeof(request_t));
        //printf("WORKSHOP SENDER ID %d, MOJE ID: %d\n", req->src, rank);
		sendPacket(&pakiet, req->src, WANT_WORKSHOP_TICKET_ACK);
		vector_delete(&wTicketQueue, 0);
    }
    println("ZWALNIAM WARSZTAT %d\n", wsNumber);
}

void freePyrkonTicket() {
    pthread_mutex_lock(&ticketMutex);
	pyrkonTicket.has = false;
    pyrkonTicket.want = false;
    pyrkonTicket.visited = true;
    pthread_mutex_unlock(&ticketMutex);
	while(vector_get(&pTicketQueue, 0)) {
		packet_t pakiet;
        pakiet.src = rank;
        pakiet.pyrkonNumber = pyrkonNumber;
        pthread_mutex_lock(&timerMutex);
        pakiet.ts = ++lamportTimer;
        pthread_mutex_unlock(&timerMutex);
        request_t *req = (request_t *)malloc(sizeof(request_t));
        memcpy(req, vector_get(&pTicketQueue, 0), sizeof(request_t));
        //printf("SENDER ID %d, MOJE ID: %d\n", req->src, rank);
		sendPacket(&pakiet, req->src, WANT_PYRKON_TICKET_ACK);
		vector_delete(&pTicketQueue, 0);
	}
	println("ZWALNIAM BILET\n");
    packet_t packet;
    packet.src = rank;
    packet.pyrkonNumber = pyrkonNumber;
    sendToEveryone(&packet, PYRKON_END);
    sem_wait(&endPyrkonSem);
}
