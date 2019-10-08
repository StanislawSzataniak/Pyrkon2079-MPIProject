#include "main.h"

int actual_ts = 0;
ticket pyrkonTicket;
MPI_Datatype MPI_PACKET_T;

extern void initialize(int *argc, char ***argv);
extern void finalize(void);

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

void init(Vector *vector)
{
    vector->size = 0;
    vector->capacity = VECTOR_INITIAL_CAPACITY;
    vector->data = malloc(sizeof(packet_t) * vector->capacity);
}

void add_sort(Vector* vector, packet_t new_item)
{
    if(vector->size + 1 >= vector->capacity)
    {
        vector->capacity *= 2;
        vector->data = realloc(vector->data, sizeof(packet_t) * vector->capacity);
    }

    for(int i=vector->size;i>=0;i--)
    {
        if(vector->data[i-1].ts<new_item.ts||i==0)
        {
            if(i==vector->size)
            {
                vector->data[vector->size++]=new_item;
                return;
            }
            else
            {
                for(int j=vector->size-1;j>=i;j--)
                {
                    vector->data[j+1]=vector->data[j];
                }
                vector->data[i]=new_item;
                vector->size++;
                return;
            }

        }
        else if (vector->data[i-1].ts==new_item.ts)
        {
            if(vector->data[i-1].src<new_item.src)
            {
                if(i==vector->size)
                {
                    vector->data[vector->size++]=new_item;
                    return;
                }
                else
                {
                    for(int j=vector->size-1;j>=i;j--)
                    {
                        vector->data[j+1]=vector->data[j];
                    }
                    vector->data[i]=new_item;
                    vector->size++;
                    return;
                }
                
            }
        }
 
    }
    
}

int my_latest_position_in_queue(Vector *vector, int my_rank)
{
    for(int i=vector->size-1;i>=0;i--)
    {
        if(vector->data[i].src==my_rank)
        {
            return i+1;
        }
    }
    return -1;

}

int vectorSize(Vector *vector)
{
    return vector->size;
}

int capacity(Vector *vector)
{
    return vector->capacity;
}

void free_memory(Vector *vector)
{
    free(vector->data);
}

void *comFunc(void *ptr) {
    MPI_Status status;
    packet_t pakiet;

    while (true) {
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

void sendPacket(packet_t *data, int dst, int type) {

    pthread_mutex_lock(&timerMutex);
        data->ts = ++lamportTimer;
    pthread_mutex_unlock(&timerMutex);

    // data->pyrkonNumber = pyrkonNumber;

    MPI_Send(data, 1, MPI_PACKET_T, dst, type, MPI_COMM_WORLD);
}

void pyrkonTicketRequestHandler(packet_t *packet)
{
    int senderId = packet->src;
    //printf("Ja hunter %d otrazymalem licence request od %ld z ts: %ld, prio: %ld a ja mam ts: %d, prio: %d\n",rank,packet->src,packet->ts,packet->prio,my_request_ts_licence,actual_prio);
    if (pyrkonTicket.want)
    {
        if (!pyrkonTicket.has && (pyrkonTicket.requestTS > packet->ts || (pyrkonTicket.requestTS == packet->ts && rank > senderId))) {
            sendPacket(packet, senderId, PYRKON_TICKET_ACK);
	}
        else {
            // pyrkonTicket.waiting.push_back(senderId);
        }
    }
    else {
        sendPacket(packet, senderId, PYRKON_TICKET_ACK);
    }
}

int main(int argc, char *argv[]) {
    printf("HALKO");
    initialize(&argc, &argv);
    printf("Hello World");
    finalize();
    return 0;
}
