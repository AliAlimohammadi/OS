#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define NUMBER_OF_SEMAPHORES 2
#define PROJECT_ID 12345   // Arbitrary project identifier
#define SHARED_MEMORY_SEGMENT_SIZE 8192 + 1   // 8 kilobytes + 1 byte
#define NUMBER_OF_MESSAGES 10   // Server only doing ten "receives" on shared memory segment

int main()
{
    // "sembuf" is implemented in <sem.h>
    struct sembuf operations[2];
    // "shmid_ds" is implemented in <struct_shmid_ds.h> (sys/shm.h)
    struct shmid_ds shmid_struct;
    void *shared_memory_address;
    int semaphore_id, shared_memory_id, rc;
    key_t semaphore_key, shared_memory_key;
    short semaphore_array[NUMBER_OF_SEMAPHORES];

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
    semaphore_id = semget(semaphore_key, NUMBER_OF_SEMAPHORES, 0666 | IPC_CREAT | IPC_EXCL);
    if (semaphore_id == -1)
    {
        printf("Error acquiring semaphore ID; Method semget() failed.\n");
        return -1;
    }

    // IMPORTANT NOTE:
    // Initializing the first and the second semaphores in the set to 0.
    // The first semaphore in the semaphore set means:
    //        '1' --  The shared memory segment is being used.
    //        '0' --  The shared memory segment is freed.
    // The second semaphore in the semaphore set means:
    //        '1' --  The shared memory segment has been changed by the client.
    //        '0' --  The shared memory segment has not been changed by the client.

    for (int i = 0; i < NUMBER_OF_SEMAPHORES; i++)
        semaphore_array[i] = 0;

    rc = semctl(semaphore_id, 1, SETALL, semaphore_array);
    if (rc == -1)
    {
        printf("Error initializing semaphores; Method semctl() initialization failed.\n");
        return -1;
    }

    // Acquiring corresponding shared memory ID for the key:
    shared_memory_id = shmget(shared_memory_key, SHARED_MEMORY_SEGMENT_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
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

    printf("Ready for client jobs...\n");

    // Looping only for a specified number of times:
    for (int i = 0; i < NUMBER_OF_MESSAGES; i++)
    {
        operations[0].sem_num = 1;   // Operate on second semaphore
        operations[0].sem_op = -1;   // Subtract one from semaphore value
        operations[0].sem_flg = 0;   // Allow waiting

        operations[1].sem_num = 0;   // Operate on first semaphore
        operations[1].sem_op =  1;   // Add one to semaphore value
        operations[1].sem_flg = IPC_NOWAIT;   // Disallow waiting

        rc = semop(semaphore_id, operations, 2);
        if (rc == -1)
        {
            printf("Error operating semaphores; Method semop() failed.\n");
            return -1;
        }

        // Printing the shared memory contents:
        printf("Server Received: \"%s\"\n", (char *) shared_memory_address);

        // Signaling the first semaphore to free the shared memory:
        operations[0].sem_num = 0;
        operations[0].sem_op  = -1;
        operations[0].sem_flg = IPC_NOWAIT;

        rc = semop(semaphore_id, operations, 1);
        if (rc == -1)
        {
            printf("Error operating semaphores; Method semop() failed.\n");
            return -1;
        }

    }

    // Cleaning up the environment:
    rc = semctl(semaphore_id, 1, IPC_RMID);
    if (rc == -1)
    {
        printf("Error removing semaphore ID; Method semctl() remove ID failed.\n");
        return -1;
    }
    // Detaching shared memory segment from current process:
    rc = shmdt(shared_memory_address);
    if (rc == -1)
    {
        printf("Error detaching shared memory; Method shmdt() failed.\n");
        return -1;
    }
    rc = shmctl(shared_memory_id, IPC_RMID, &shmid_struct);
    if (rc == -1)
    {
        printf("Error deleting shared memory segment ID; Method shmctl() failed.\n");
        return -1;
    }

    return 0;
}
