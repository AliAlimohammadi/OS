#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>

#define NUMBER_OF_SEMAPHORES 2
#define PROJECT_ID 12345   // Arbitrary project identifier
#define SHARED_MEMORY_SEGMENT_SIZE 8192 + 1   // 8 kilobytes + 1 byte

int main(int argc, char *argv[])
{
    // Handling command-line arguments:
    if (argc < 2)
    {
        printf("No text message was given.\n");
        exit(1);
    }

    // "sembuf" is implemented in <sem.h>
    struct sembuf operations[2];
    void *shared_memory_address;
    int semaphore_id, shared_memory_id, rc;
    key_t semaphore_key, shared_memory_key;

    // Generating IPC key for semaphore set and shared memory segment:
    semaphore_key = ftok("/dev/null", PROJECT_ID);
    if (semaphore_key == (key_t) -1)
    {
        printf("Error acquiring semaphore; Method ftok() for semaphore failed.\n");
        return -1;
    }
    shared_memory_key = ftok("/dev/null", PROJECT_ID);
    if (shared_memory_key == (key_t) -1)
    {
        printf("Error acquiring shared_memory; Method ftok() for shared memory failed.\n");
        return -1;
    }

    // Acquiring corresponding semaphore ID for the key:
    semaphore_id = semget(semaphore_key, NUMBER_OF_SEMAPHORES, 0666);
    if (semaphore_id == -1)
    {
        printf("Error acquiring semaphore ID; Method semget() failed.\n");
        return -1;
    }

    // Acquiring corresponding shared memory ID for the key:
    shared_memory_id = shmget(shared_memory_key, SHARED_MEMORY_SEGMENT_SIZE, 0666);
    if (shared_memory_id == -1)
    {
        printf("Error acquiring shared memory ID; Method shmget() failed.\n");
        return -1;
    }

    // Attaching shared memory segment to client process:
    shared_memory_address = shmat(shared_memory_id, NULL, 0);
    if (shared_memory_address == NULL)
    {
        printf("Error attaching shared memory segment to client process; Method shmat() failed.\n");
        return -1;
    }

    // Measuring the time taken by a cycle of exchanging messages:
    clock_t t = clock();

    operations[0].sem_num = 0;   // Operate on first semaphore
    operations[0].sem_op  = 0;   // Wait for the value to become zero
    operations[0].sem_flg = 0;   // Allow waiting

    operations[1].sem_num = 0;   // Operate on second semaphore
    operations[1].sem_op  = 1;   // Add one to semaphore value
    operations[1].sem_flg = 0;   // Allow waiting

    rc = semop(semaphore_id, operations, 2);
    if (rc == -1)
    {
        printf("Error operating semaphores; Method semop() failed.\n");
        return -1;
    }

    // Writing information into the shared memory
    strcpy((char *) shared_memory_address, argv[1]);

    // Releasing shared memory segment by decrementing first semaphore and
    // incrementing second semaphore to show that the client is finished with it:
    operations[0].sem_num =  0;   // Operate on first semaphore
    operations[0].sem_op  = -1;   // Subtract one from semaphore value
    operations[0].sem_flg =  0;   // Allow waiting

    operations[1].sem_num =  1;   // Operate on second semaphore
    operations[1].sem_op  =  1;   // Add one to semaphore value
    operations[1].sem_flg =  0;   // Allow waiting

    rc = semop(semaphore_id, operations, 2);
    if (rc == -1)
    {
        printf("Error operating semaphores; Method semop() failed.\n");
        return -1;
    }

    // Detaching shared memory segment from current process:
    rc = shmdt(shared_memory_address);
    if (rc == -1)
    {
        printf("Error detaching shared memory; Method shmdt() failed.\n");
        return -1;
    }

    t = clock() - t;
    double elapsed_time = ((double) t * 1000) / CLOCKS_PER_SEC;   // Time spent in milliseconds

    printf("A cycle of exchanging messages took %f milliseconds to execute.\n", elapsed_time);

    return 0;
}
