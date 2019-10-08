#include "main.h"

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

        // if (pakiet.pyrkonNumber == pyrkonNumber)        //protection from receiving messages from previous Pyrkon
        //     handlers[(int)status.MPI_TAG](&pakiet);
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

int main(int argc, char *argv[]) {
    return 0;
}