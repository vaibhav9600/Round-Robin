#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include <errno.h>

#include "struct_utils.h"

#define MAX_THREAD 16
#define THREAD_COUNT 4
#define checkError(x)                           \
    if (x == -1)                                \
    {                                           \
        printf("Error found in Shared Memory"); \
        exit(1);                                \
    }
#define checkError2(x, y)                             \
    if (x == (y)-1)                                   \
    {                                                 \
        perror("Error attaching to shared memory\n"); \
        exit(1);                                      \
    }

long long elementCount = 0;
int count = 0;

pthread_mutex_t lock;

int I, J, K;

int *s1, *s2, *shm_flags;

int **mul;

int cond_check_1(int i, int x)
{
    return (i < x && i < I);
}
void func_helper_col_multiply(int row_term, int start_col, int end_col)
{
    int i;
    for (i = 0; cond_check_1(i, row_term); i++)
    {
        for (int j = start_col; j < end_col; j++)
        {

            mul[i][j] = 0;
            int k = 0;
            while (k < J)
            {
                int row_cursor = i * J + k;
                int col_cursor = k * K + j;
                mul[i][j] = mul[i][j] + s1[row_cursor] * s2[col_cursor];
                k++;
            }
        }
    }
}
void func_helper_row_multiply(int start_row, int end_row, int end_col)
{
    int i = 0;
    for (i = start_row; cond_check_1(i, end_row); i++)
    {
        for (int j = 0; j < end_col; j++)
        {
            mul[i][j] = 0;
            int k = 0;
            while (k < J)
            {
                int row_cursor = i * J + k;
                int col_cursor = k * K + j;
                mul[i][j] = mul[i][j] + s1[row_cursor] * s2[col_cursor];
                k++;
            }
        }
    }
}
int thingsToRead(int x)
{
    if (!(x % THREAD_COUNT))
    {
        return x / THREAD_COUNT;
    }
    return x / THREAD_COUNT + 1;
}

void *thread_multiply(void *args)
{
    thread_args *t_info = args;

    int initial_row = t_info->start_row;
    int final_row = t_info->end_row;
    int initial_col = t_info->start_col;
    int final_col = t_info->end_col;
    int row_cursor = 0;
    int col_cursor = 0;
    int i = 0;
    int j = initial_col;
    int k = 0;
    func_helper_col_multiply(final_row, initial_col, final_col);
    func_helper_row_multiply(initial_row, final_row, final_col);
    pthread_exit(NULL);
}

void initializeMatrix()
{

    mul = (int **)malloc(I * sizeof(long long int *));
    int i = 0;
    while (i < I)
    {
        mul[i] = (int *)malloc(K * sizeof(long long int));
        i++;
    }
}

void PrintToFile(const char *args)
{
    FILE *out;
    out = fopen(args, "w+");

    for (int i = 0; i < I; i++)
    {
        for (int j = 0; j < K; j++)
        {
            elementCount++;
            fprintf(out, "%d ", mul[i][j]);
        }
        fprintf(out, "\n");
    }
    if (I * K == elementCount)
    {
        fclose(out);
    }
    else
    {
        perror("Error in printing to file\n");
        fclose(out);
    }
}

int main(int argc, char const *argv[])
{
    if (!(argc == 7))
    {
        printf("i j k in1.txt in2.txt out.txt\n");
    }

    printf("P2 has been started\n");

    I = atoi(argv[1]);
    J = atoi(argv[2]);
    K = atoi(argv[3]);

    initializeMatrix();

    key_t shm_time_logging = ftok("group56_assignment2.c", 54);
    int shm_id_time_logging = shmget(shm_time_logging, sizeof(time_logging), 0666);
    checkError(shm_id_time_logging)
    time_logging *time_log = shmat(shm_id_time_logging, NULL, 0);

    key_t shm_proc_data = ftok("group56_assignment2.c", 50);
    int shm_id_proc_data = shmget(shm_proc_data, sizeof(proc_data), 0666);
    checkError(shm_id_proc_data)

    proc_data *shared_proc_data = (proc_data *)shmat(shm_id_proc_data, NULL, 0);
    checkError2(shared_proc_data, proc_data *)

    key_t shm_matrix1;
    shm_matrix1 = ftok("group56_assignment2.c", 51);
    key_t shm_matrix2;
    shm_matrix2 = ftok("group56_assignment2.c", 52);
    key_t shm_threadFlags;
    shm_threadFlags = ftok("group56_assignment2.c", 53);

    int shm_id_matrix1 = shmget(shm_matrix1, (I * J) * sizeof(int), 0666);
    checkError(shm_id_matrix1) int shm_id_matrix2 = shmget(shm_matrix2, (J * K) * sizeof(int), 0666);
    checkError(shm_id_matrix2) int shmid_id_flags;
    shmid_id_flags = shmget(shm_threadFlags, (THREAD_COUNT) * sizeof(int), 0666 | IPC_CREAT);
    checkError(shmid_id_flags)

    s1 = shmat(shm_id_matrix1, NULL, 0);
    s2 = shmat(shm_id_matrix2, NULL, 0);
    shm_flags = (int *)shmat(shmid_id_flags, NULL, 0);
    checkError2(s1, int *)
        checkError2(s2, int *)
            checkError2(shm_flags, int *)

                int rowsToRead = thingsToRead(I);

    int colsToRead = thingsToRead(K);

    pthread_t threads[THREAD_COUNT];

    clock_t start_time = clock();
    int sp_row = 0, sp_col = 0;
    thread_args *t[THREAD_COUNT];
    for (int j = 0; j < THREAD_COUNT; j++)
    {
        t[j] = malloc(sizeof(thread_args));

        t[j]->thread_id = j;

        t[j]->start_row = sp_row;
        t[j]->end_row = sp_row + rowsToRead;
        sp_row += rowsToRead;

        t[j]->start_col = sp_col;
        t[j]->end_col = sp_row + colsToRead;
        sp_col += colsToRead;

        while (shm_flags[t[j]->thread_id] != 1)
        {
            ;
        }

        pthread_create(&threads[j], NULL, thread_multiply, t[j]);
    }

    for (int j = 0; j < THREAD_COUNT; j++)
    {
        pthread_join(threads[j], NULL);
    }

    clock_t end_time = clock();

    double turnAround_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    time_log->ta_time[1] = turnAround_time;

    PrintToFile(argv[6]);

    shared_proc_data->finished[1] = 1;
    return 0;
}