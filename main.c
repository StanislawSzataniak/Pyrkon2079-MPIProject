#include "main.h"

MPI_Datatype MPI_PAKIET_T;
pthread_mutex_t timerMutex;

void sendPacket(packet_t *data, int dst, int type) {
//
//    //pthread_mutex_lock(&timerMutex);
//    //     data->ts = ++lamportTimer;
//    //     setrequestTS(data, lamportTimer, type);                     /* jeżeli konieczne, ustaw requestTimer na aktualny lamportTimer - Ricart Agrawala Aglorithm */
//    //     // println("%s -> %d", getMessageCode(type).c_str(), dst);     /* printowanie w mutexie, żeby zapewnić idealną informację o punktach w czasie */
//    //pthread_mutex_unlock(&timerMutex);
//
    MPI_Send(data, 1, MPI_PACKET_T, dst, type, MPI_COMM_WORLD);
}




int main(int argc, char *argv[]) {
    //initialize(argc, argv);
    //mainLoop();
    //finalize();
    int rank, size;
    MPI_Init (&argc, &argv);      /* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);        /* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);        /* get number of processes */
    printf( "Hello world from process %d of %d\n", rank, size );
    MPI_Finalize();
    return 0;
}