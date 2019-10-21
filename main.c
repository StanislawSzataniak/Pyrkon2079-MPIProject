#include "main.h"
#include "vector.h"



ticket pyrkonTicket;
ticket workshops[3];

MPI_Datatype MPI_PACKET_T;
int pyrkon_end = 0;

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
    
    
}

void *comFunc(void *ptr) {
    MPI_Status status;
    packet_t pakiet;
    int end = true;

    while (!end) {
        MPI_Recv(&pakiet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        pthread_mutex_lock(&timerMutex);
            lamportTimer = max(lamportTimer, pakiet.ts) + 1;
            // println("%d -> %s", status.MPI_SOURCE, getMessageCode(status.MPI_TAG).c_str());
        pthread_mutex_unlock(&timerMutex);

        pakiet.src = status.MPI_SOURCE;

        if (pakiet.pyrkonNumber == pyrkonNumber) {        //protection from receiving messages from previous Pyrkon
            // handlers[(int)status.MPI_TAG](&pakiet);
	}
    }
    return 0;
}



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

void workshopResponsetHandler(packet_t* packet) {
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


int main(int argc, char **argv) {
    initialize(&argc, &argv);
    VECTOR_INIT(v);
    VECTOR_ADD(v, "a");
    printf("%d \n", VECTOR_TOTAL(v));
    VECTOR_DELETE(v, 1);
    VECTOR_FREE(v);
    finalize();
    return 0;
}
