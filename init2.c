#include "main.h"

int processId, size;

pthread_mutex_t stackMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t packetMut = PTHREAD_MUTEX_INITIALIZER;

typedef struct stack_s {
    stackEl_t *pakiet;
    struct stack_s *prev;
    struct stack_s *next;
} stack_t;

stack_t **stack;

void push_pkt( stackEl_t *pakiet, int n)
{
    stack_t *tmp = malloc(sizeof(stack_t));
    tmp->pakiet = pakiet;

    tmp->next = 0;
    tmp->prev = 0;
    pthread_mutex_lock(&stackMut);
    if (!stack[n]) stack[n] = tmp;
    else {
	stack_t *head = stack[n];
        while (head->next) head = head->next; 
	tmp->prev = head; 
        head->next = tmp;
    }
    pthread_mutex_unlock(&stackMut);

}

stackEl_t *pop_pkt(int n)
{
    pthread_mutex_lock(&stackMut);
    if (stack[n] == NULL ) {
	pthread_mutex_unlock(&stackMut);
        return NULL;
    } else {
        stackEl_t *tmp = stack[n]->pakiet;
        stack_t *next = stack[n]->next;
        if (next)
	    next->prev = 0;
        free(stack[n]);
        stack[n] = next;
	pthread_mutex_unlock(&stackMut);
        return tmp; 
    }
}




void inicjuj(int *argc, char ***argv) {
    // initialize MPI
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // initialize data type
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

    // initialize lamport timer
    lamportTimer = 0;

    // initialize stack
    stack = malloc(sizeof(stack_t*)*size);
    memset(stack, 0, sizeof(stack_t*)*size); 

    // initializeHandlers();
    srand(processId); //for every process set unique rand seed
    // srand(time(NULL));
    // initializeSemaphores();  //musi być przed inicjalizacją wątków
    // runThreads();
}