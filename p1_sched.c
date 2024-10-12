#include <assert.h>
#include <stdio.h>
#define GROUPNAME "group56_assignment2.c"
#include <sys/shm.h>
#include <pthread.h>
#define INT_SIZE sizeof(int)
#include <errno.h>
#include <sys/types.h>
#define MAX_THREAD 16
#include <semaphore.h>
#include <sys/ipc.h>
#define EXIT exit(1)
#include <stdlib.h>
#include "struct_utils.h"
#include <time.h>

#define THREAD_COUNT 4
#define PERMISSION 0666
#define takeInput(y) atoi(argv[y])

#define checkError(x)                     \
    if (x == NULL)                        \
    {                                     \
        printf("Error opening file 1\n"); \
        EXIT;                             \
    }
#define checkError2(y)                                      \
    if (y == -1)                                            \
    {                                                       \
        perror("Error in shared memory time allocation\n"); \
    }

int I, J, K;

const char *file1, *file2;

int *s1, *s2, *shm_flags, *mp1, *mp2;

pthread_mutex_t lock;
FILE *f1, *f2;
void *thread_read(void *args)
{
    thread_args *t_arg = args;

    int i = t_arg->start_row;

    for (; i < t_arg->end_row && i < I;)
    {
        int col = 0;
        for (; col <= J - 1;)
        {

            pthread_mutex_lock(&lock);
            int nxtAddress = col;
            nxtAddress = nxtAddress + J * i;
            fseek(f1, mp1[nxtAddress], SEEK_SET);
            fscanf(f1, "%d", &s1[nxtAddress]);
            pthread_mutex_unlock(&lock);
            ++col;
        }
        i++;
    }

    int col = t_arg->start_col;
    for (; col < t_arg->end_col && col < K;)
    {
        for (i = 0; i < J; ++i)
        {

            pthread_mutex_lock(&lock);
            int nxtAddress = col;
            nxtAddress = nxtAddress + K * i;
            fseek(f2, mp2[nxtAddress], SEEK_SET);
            fscanf(f2, "%d", &s2[nxtAddress]);
            pthread_mutex_unlock(&lock);
        }
        col += 1;
    }

    shm_flags[t_arg->thread_id] = 1;
    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{

    (argc != 7) ? printf("Error in arguments") : 0;

    const char *out = argv[6];

    file2 = argv[5];
    file1 = argv[4];

    printf("Starting P1\n");

    printf("Starting P1\n");
    I = takeInput(1);
    J = takeInput(2);
    K = takeInput(3);

    f1 = fopen(file1, "r");
    checkError(f1)
        f2 = fopen(file2, "r");
    checkError(f2)

    key_t shm_time_logging;
    shm_time_logging = ftok(GROUPNAME, 54);
    int shm_id_time_logging;
    shm_id_time_logging = shmget(shm_time_logging, sizeof(time_logging), PERMISSION);
    checkError2(shm_id_time_logging)

        int time_log_param = shm_id_time_logging;

    time_logging *time_log = shmat(time_log_param, NULL, 0);

    int inputSize = INT_SIZE * (THREAD_COUNT);
    int shmid_id_flags = shmget(ftok(GROUPNAME, 53), inputSize, PERMISSION | IPC_CREAT);

    checkError2(shmid_id_flags)

        shm_flags = (int *)shmat(shmid_id_flags, NULL, 0);

    key_t shm_proc_data = ftok(GROUPNAME, 50);
    int shm_id_proc_data = shmget(shm_proc_data, sizeof(proc_data), PERMISSION);
    checkError2(shmid_id_flags)

        proc_data *shared_proc_data = (proc_data *)shmat(shm_id_proc_data, NULL, 0);
    if (shared_proc_data == (proc_data *)-1)
    {
        perror("Error attaching P1 \n");
        exit(errno);
    }

    key_t shm_matrix1, shm_matrix2;
    shm_matrix2 = ftok(GROUPNAME, 52);
    shm_matrix1 = ftok(GROUPNAME, 51);
    int paramSize = INT_SIZE * (J * I);
    int shm_id_matrix1 = shmget(shm_matrix1, paramSize, PERMISSION);
    checkError2(shm_id_matrix1)
        paramSize = INT_SIZE * (K * J);
    int shm_id_matrix2 = shmget(shm_matrix2, paramSize, PERMISSION);
    checkError2(shm_id_matrix2)

        s2 = shmat(shm_id_matrix2, NULL, 0);

    s1 = shmat(shm_id_matrix1, NULL, 0);

    if ((int *)(-1) == s2 || (int *)(-1) == s1)
    {
        perror("Error attaching to shared memory\n");
        EXIT;
    }

    key_t shm_file1_map = ftok(GROUPNAME, 58);
    int inputSize2 = J * I;
    inputSize2 *= INT_SIZE;
    int shm_id_file1_map = shmget(shm_file1_map, inputSize2, PERMISSION);
    checkError2(shm_id_file1_map);
    mp1 = shmat(shm_id_file1_map, NULL, 0);

    if ((int *)-1 == mp1)
    {
        perror("Error attaching to shared memory\n");
        EXIT;
    }

    key_t shm_file2_map = ftok(GROUPNAME, 59);
    int shm_id_file2_map = shmget(shm_file2_map, J * K * INT_SIZE, PERMISSION);
    checkError2(shm_id_file2_map);

    mp2 = shmat(shm_id_file2_map, NULL, 0);
    if (mp2 == (int *)-1)
    {
        perror("Error attaching to shared memory\n");
        EXIT;
    }
    int rowsToRead, colsToRead;
    if (K % THREAD_COUNT == 0)
        colsToRead = K / THREAD_COUNT;
    else
        colsToRead = K / THREAD_COUNT + 1;
    ;

    if (I % THREAD_COUNT == 0)
        rowsToRead = I / THREAD_COUNT;
    else
        rowsToRead = I / THREAD_COUNT + 1;
    ;

    if (pthread_mutex_init(&lock, NULL))
    {
        printf("\n Failed initialising Mutex\n");
        return 1;
    }

    pthread_t *threads;
    int sizeOfthread = sizeof(pthread_t) * THREAD_COUNT;
    threads = (pthread_t *)malloc(sizeOfthread);
    int sp_row, sp_col;

    sp_row = sp_col = 0;

    clock_t start_time = clock();

    int j = 0;
    for (; j < THREAD_COUNT;)
    {
        thread_args *t;
        t = malloc(sizeof(thread_args));
        t->thread_id = j;

        t->start_row = sp_row;
        int end_row_val = sp_row + rowsToRead;
        t->end_row = end_row_val;
        sp_row = end_row_val;

        t->start_col = sp_col;
        int end_col_val = colsToRead + sp_col;
        t->end_col = end_col_val;
        sp_col = end_col_val;

        pthread_create(&threads[j], NULL, thread_read, t);
        j++;
    }
    j = 0;
    for (; j < THREAD_COUNT;)
    {
        pthread_join(threads[j], NULL);
        j = j + 1;
    }
    clock_t end_time = clock();

    double timeDifference = (double)(end_time);
    timeDifference -= (double)(start_time);

    time_log->ta_time[0] = timeDifference / CLOCKS_PER_SEC;

    shared_proc_data->finished[0] = 1;

    fclose(f1);
    fclose(f2);
    return 0;
}