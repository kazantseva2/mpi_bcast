#include <cstdio>
#include <mpi.h>
#include <string.h>

#define MESSAGE "I get the message."
#define N strlen(MESSAGE) + 1
#define K 4


void sendUp(int my_coords[2], const char* message, MPI_Comm comm) {
    if (my_coords[1] > K - 2) {
        return;
    }

    int up_coords[2] = { my_coords[0], my_coords[1] + 1};
    MPI_Request request;
    int rank;
    MPI_Cart_rank(comm, up_coords, &rank);
    MPI_Isend(message, strlen(message) + 1, MPI_CHAR, rank, 0, comm, &request);

    printf("(%d, %d) sent message to (%d, %d)\n", my_coords[0], my_coords[1], up_coords[0], up_coords[1]);
    fflush(stdout);
}

void sendRight(int my_coords[2], const char* message, MPI_Comm comm) {
    if (my_coords[0] > K - 2) {
        return;
    }

    int right_coords[2] = { my_coords[0] + 1, my_coords[1]};
    MPI_Request request;
    int rank;
    MPI_Cart_rank(comm, right_coords, &rank);
    MPI_Isend(message, strlen(message) + 1, MPI_CHAR, rank, 0, comm, &request);

    printf("(%d, %d) sent message to (%d, %d)\n", my_coords[0], my_coords[1], right_coords[0], right_coords[1]);
    fflush(stdout);
}

void recev(int my_coords[2], MPI_Comm comm, char* message) {
    int prev_coords[2] = { my_coords[0], my_coords[1] };

    if (my_coords[0] == 0) {
        prev_coords[1] -= 1;
    }
    else {
        prev_coords[0] -= 1;
    }

    int rank;
    MPI_Cart_rank(comm, prev_coords, &rank);
    MPI_Recv(message, N, MPI_CHAR, rank, 0, comm, MPI_STATUS_IGNORE);

    if (strcmp(message, MESSAGE)) {
        printf(" ERROR \n");
        printf(" Provided : %s\n", message);
        printf(" Expected : %s\n", MESSAGE);
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    printf("(%d, %d) received message from (%d, %d)\n", my_coords[0], my_coords[1], prev_coords[0], prev_coords[1]);
    fflush(stdout);
}

int main(int* argc, char** argv) {
    MPI_Init(argc, &argv);

    int comm_size;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    if (comm_size != K*K) {
        printf(" ERROR : Expected %d processes , but %d provided \n", K*K, comm_size);
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int ndims = 2;
    int dims[2] = { K, K };
    int periods[2] = { 0, 0 };
    int reorder = 0;
    MPI_Comm new_comm;
    MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, reorder, &new_comm);

    int coords[2];
    MPI_Cart_coords(new_comm, rank, ndims, coords);

    printf("My rank is %d (%d, %d), %d processes in work\n", rank, coords[0], coords[1], comm_size);

    if (coords[0] == 0 && coords[1] == 0) {
        sendUp(coords, MESSAGE, new_comm);
        sendRight(coords, MESSAGE, new_comm);
    }
    else {
        char* message = new char[N];
        recev(coords, new_comm, message);

        if (coords[0] == 0) {
            sendUp(coords, message, new_comm);
        }
        sendRight(coords, message, new_comm);

        delete[] message;
    }

    MPI_Barrier(new_comm);

    printf("Finish: my rank is %d (%d, %d)\n", rank, coords[0], coords[1]);
    fflush(stdout);

    MPI_Finalize();

    return 0;
}
