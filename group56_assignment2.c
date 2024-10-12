#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NUMBER_OF_THREADS 4
#define sharedMemoryDeallocate(x) shmctl((x), IPC_RMID, NULL);
#define  NANOSECONDS 1000000000

#define TIME_SLICE 2000 
#define checkerror3(x) (x) == NULL ? 1 : 0;
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
#define checkError4(x) if(x==NULL) printf("File not opened \n");

int I;
int J;
int K;
char *in1;
char *in2;
char *out;
int fcounter;


typedef struct
{
    int flag1;
    int flag2;
    int finished[2];
} isProcessFinished;
isProcessFinished *shared_proc_data;


typedef struct
{
    double ta_time[2];
    double wt_time[2];
    double op_count;
    double run_time[2];
    double op_time[2];
    struct timespec switch_time[2];
} time_logging;
time_logging *timings;




int shm_id_proc_data;
int shm_id_matrix1;
int shm_id_matrix2;
int flags_shm;
int shm_id_time_logging;
int shm_id_file1_map;
int shm_id_file2_map;

pid_t process1;
pid_t currentlyScheduledProcess;
pid_t process2;

struct timespec switchingTime_start[2];
struct timespec switchingTime_end[2];
struct timespec smallTime;

clock_t waitingTimeS[2], waitingTimeE[2];





void mapTheFile(char *file, int *mp, int height, int width)
{

    FILE *f = fopen(file, "r");

    checkerror3(f)

        int temp,
        t, i = 0, j = 0;
    int mapping2d[height][width];
    mapping2d[0][0] = 0;
    mp[0] = 0;
    int count_of_mappings=0;
    while (!feof(f))
    {
        t = ftell(f);
        if (j >= width || i >= height)
        {
            if (j >= width)
            {
                j = 0;
                i++;
            }
        }
        count_of_mappings++;
        if (i >= height)
            break;
        mapping2d[i][j] = t;
        j++;
        fscanf(f, "%d", &temp);
    }
    
    for (i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            mp[i * width + j] = mapping2d[i][j];
        }
    }
    fclose(f);
}



void getSwitchingTime(struct timespec a2, struct timespec *smallTime, struct timespec a1)
{
    time_t seconds = a2.tv_sec;
    seconds-=a1.tv_sec;

    time_t nanoSecs=a2.tv_nsec; 
    nanoSecs-=a1.tv_nsec;

    smallTime->tv_sec = seconds;
    smallTime->tv_nsec = nanoSecs;
    
    struct timespec tq;
    int flag_1=(smallTime->tv_sec < 0 && smallTime->tv_nsec > 0);
    int flag_2=(smallTime->tv_sec > 0 && smallTime->tv_nsec < 0);
    if (flag_1)
    {
        
        smallTime->tv_nsec -= NANOSECONDS;
        tq.tv_nsec++;
        tq.tv_sec--;
        smallTime->tv_sec++;
    }
    else if (flag_2)
    {
        
        smallTime->tv_nsec += NANOSECONDS;
        tq.tv_nsec--;
        tq.tv_sec++;
        smallTime->tv_sec--;
    }
}

void createSharedMemory()
{
    int s1, s2; 

    
    key_t shm_finish_data;
    shm_finish_data = ftok("group56_assignment2.c", 50);
    
    shm_id_proc_data = shmget(shm_finish_data, sizeof(isProcessFinished), 0666 | IPC_CREAT);
    checkError(shm_id_proc_data)
        shared_proc_data = (isProcessFinished *)shmat(shm_id_proc_data, NULL, 0);
    
    checkError2(shared_proc_data, isProcessFinished *)
        


        shared_proc_data->finished[0] = 0;
    shared_proc_data->finished[1] = 0;

    
    key_t shm_matrix1;
    key_t shm_matrix2;
    key_t shm_file1_map;
    key_t shm_file2_map;


    shm_matrix1=ftok("group56_assignment2.c", 51);
    shm_matrix2 = ftok("group56_assignment2.c", 52);
    shm_file1_map= ftok("group56_assignment2.c", 58);
    shm_file2_map = ftok("group56_assignment2.c", 59);


    shm_id_matrix1 = shmget(shm_matrix1, (I * J) * sizeof(int), 0666 | IPC_CREAT);
    shm_id_matrix2 = shmget(shm_matrix2, (J * K) * sizeof(int), 0666 | IPC_CREAT);
    shm_id_file1_map = shmget(shm_file1_map, I * J * sizeof(int), 0666 | IPC_CREAT);
    shm_id_file2_map = shmget(shm_file2_map, J * K * sizeof(int), 0666 | IPC_CREAT);


    checkError(shm_id_matrix1)
    checkError(shm_id_matrix2)
      checkError(shm_id_file1_map)

        
    


    int *mp1;
    int *mp2;
    mp1 = shmat(shm_id_file1_map, NULL, 0);
    mapTheFile(in1, mp1, I, J);

    

    checkError(shm_id_file2_map)
    mp2 = shmat(shm_id_file2_map, NULL, 0);
    

    mapTheFile(in2, mp2, J, K);
    flags_shm = shmget(ftok("group56_assignment2.c", 53), (NUMBER_OF_THREADS) * sizeof(int), 0666 | IPC_CREAT);
    
    checkError(flags_shm) 

    int *flags = (int *)shmat(flags_shm, NULL, 0);
    int t = 0;
    int sizeToMemset =  sizeof(flags[0]) * NUMBER_OF_THREADS; 
    memset(flags, 0, sizeToMemset);

    
    key_t shm_time_logging = ftok("group56_assignment2.c", 54);
    shm_id_time_logging = shmget(shm_time_logging, sizeof(time_logging), 0666 | IPC_CREAT);
    checkError(shm_id_time_logging)
        timings = shmat(shm_id_time_logging, NULL, 0);
}
void switchingTimeCalc(int i, int num)
{

    switch (num)
    {
        case 1:


    getSwitchingTime(switchingTime_end[i], &smallTime,switchingTime_start[i]);
    clock_t currentTime = clock();

    long nanoSmallTimeChange = timings->switch_time[i].tv_nsec + smallTime.tv_nsec;
    time_t smallTimeChange = timings->switch_time[i].tv_sec + (int)smallTime.tv_sec;
    

    timings->switch_time[i].tv_sec = smallTimeChange;

    timings->switch_time[i].tv_nsec = nanoSmallTimeChange;

    
    
    waitingTimeE[i] = currentTime;
    
    double diff = (double)waitingTimeE[i] - (double)waitingTimeS[i];
    double waitingTime = (diff) / CLOCKS_PER_SEC;
    timings->wt_time[i] = timings->wt_time[i] + waitingTime;
    break;
    case 0:


     getSwitchingTime(switchingTime_end[i], &smallTime, switchingTime_start[i]);


    time_t timeValue = timings->switch_time[i].tv_sec + (int)smallTime.tv_sec;
    timings->switch_time[i].tv_sec = timeValue;

    timeValue = timings->switch_time[i].tv_nsec + smallTime.tv_nsec; 
    timings->switch_time[i].tv_nsec = timeValue;
    break;
    }

}
void switchProcess(int idx,int num)
{
    switch (num)
    {
        case 0:
        clock_gettime(CLOCK_REALTIME, &switchingTime_start[idx]); 
         kill(currentlyScheduledProcess, SIGCONT);               
         clock_gettime(CLOCK_REALTIME, &switchingTime_end[idx]);
         switchingTimeCalc(idx,1^num);
        break;
        case 1:
        clock_gettime(CLOCK_REALTIME, &switchingTime_start[idx]); 
         kill(currentlyScheduledProcess, SIGSTOP);
         clock_gettime(CLOCK_REALTIME, &switchingTime_end[idx]);
         switchingTimeCalc(idx,1^num);

         break;
         default:
         printf("Invalid type provided\n");
         break;

    }

}

void open_time_data_files(){


    FILE *ta_wl1, *ta_wl2, *st_wl1, *st_wl2, *wt_wl1, *wt_wl2;
    switch(TIME_SLICE){
        case 1000:
        ta_wl1 = fopen("p3_ta_p1_1ms.txt", "a+");
        checkError4(ta_wl1);
        st_wl1 = fopen("p3_st_p1_1ms.txt", "a+");
        checkError4(st_wl1);

        wt_wl1 = fopen("p3_wt_p1_1ms.txt", "a+");
        checkError4(wt_wl1);


        ta_wl2 = fopen("p3_ta_p2_1ms.txt", "a+");
        checkError4(ta_wl2);

        st_wl2 = fopen("p3_st_p2_1ms.txt", "a+");
        checkError4(st_wl2);

        wt_wl2 = fopen("p3_wt_p2_1ms.txt", "a+");
        checkError4(wt_wl2);
        break;
        case 2000:
        ta_wl1 = fopen("p3_ta_p1_2ms.txt", "a+");
        checkError4(ta_wl1);
        st_wl1 = fopen("p3_st_p1_2ms.txt", "a+");
        checkError4(st_wl1);

        wt_wl1 = fopen("p3_wt_p1_2ms.txt", "a+");
        checkError4(wt_wl1);

        ta_wl2 = fopen("p3_ta_p2_2ms.txt", "a+");
        checkError4(ta_wl2);

        st_wl2 = fopen("p3_st_p2_2ms.txt", "a+");
        checkError4(st_wl2);

        wt_wl2 = fopen("p3_wt_p2_2ms.txt", "a+");
        checkError4(wt_wl2);
        break;
        default:
        printf("Enter Valid Time Quantum \n");
    }

    int flag=0;
    flag+=fprintf(ta_wl1, "%lf, ", timings->ta_time[0]);
    if(flag<0)
    flag+=printf("Failed to write in the file");
    flag=0;
    flag+=fprintf(ta_wl2, "%lf, ", timings->ta_time[1]);
    if(flag<0)
    flag+=printf("Failed to write in the file");
    flag=0;
    fprintf(wt_wl1, "%lf, ", timings->wt_time[0]);
    if(flag<0)
    flag+=printf("Failed to write in the file");
    flag=0;
    flag+=fprintf(wt_wl2, "%lf, ", timings->wt_time[1]);
    if(flag<0)
    flag+=printf("Failed to write in the file");
    flag=0;
    flag+=fprintf(st_wl1, "%d.%.9ld, ", (int)timings->switch_time[0].tv_sec, timings->switch_time[0].tv_nsec);
    if(flag<0)
    flag+=printf("Failed to write in the file");
    flag=0;
    flag+=fprintf(st_wl2, "%d.%.9ld, ", (int)timings->switch_time[1].tv_sec, timings->switch_time[1].tv_nsec);
    if(flag<0)
    flag+=printf("Failed to write in the file");
    flag=1;
    flag=fclose(ta_wl1);
    if(flag!=0){
        printf("Fclose failed \n");
    }
    flag=fclose(wt_wl1);
    if(flag!=0){
        printf("Fclose failed \n");
    }
    flag=fclose(st_wl1);
    if(flag!=0){
        printf("Fclose failed \n");
    }
    flag=fclose(ta_wl2);
    if(flag!=0){
        printf("Fclose failed \n");
    }
    flag=fclose(wt_wl2);
    if(flag!=0){
        printf("Fclose failed \n");
    }
    flag=fclose(st_wl2);
    if(flag!=0){
        printf("Fclose failed \n");
    }
}

int main(int argc, char *argv[])
{
    if (!(argc==7)){
        printf("i j k in1.txt in2.txt out.txt\n");
    }


    
    I = atoi(argv[1]); 
    in2 = argv[5];
    K = atoi(argv[3]);
    in1 = argv[4];
    J = atoi(argv[2]);
    out = argv[6];

    createSharedMemory();

    process1 = fork();
    if (process1)
    {
        process2 = fork();
        if (process2)
        {
            process2 ? kill(process2, SIGSTOP) : 0;
            clock_t clockTime = clock();
            waitingTimeS[1] = clockTime;

            process1 ? kill(process1, SIGSTOP) : 0;
            clock_t clockTime1 = clock();
            currentlyScheduledProcess = process1;
            waitingTimeS[0] = clockTime1;

            

            for (int ii = 0; ii > -1;)
            {
                int i = 0;
                int finished1 = shared_proc_data->finished[1];
                int finished2 = shared_proc_data->finished[0];
                if (finished1 && finished2){
                    printf("Done\n");
                    break;
                }

                
                while(i < 2)
                {
                    if (shared_proc_data->finished[i]== 0)
                    {
                        switchProcess(i,0);
                        usleep(TIME_SLICE);
                        switchProcess(i,1);

                        if (process1 != currentlyScheduledProcess)
                        {
                            clock_t time1 = clock();
                            currentlyScheduledProcess = process1;
                            waitingTimeS[1] = time1;
                        }
                        else if ( process2 != currentlyScheduledProcess)
                        {
                            clock_t time2 = clock();
                            currentlyScheduledProcess = process2;
                            waitingTimeS[0] = time2;
                            
                        }
                    }
                    i = i + 1;
                }
            }

            
            open_time_data_files();
            

            
            sharedMemoryDeallocate(shm_id_file1_map)
            sharedMemoryDeallocate(shm_id_file2_map)
            sharedMemoryDeallocate(shm_id_proc_data)
            sharedMemoryDeallocate(shm_id_matrix1)
            sharedMemoryDeallocate(shm_id_matrix2)
            sharedMemoryDeallocate(shm_id_time_logging)
            sharedMemoryDeallocate(flags_shm)

            
        }
        else
        {
            


            execlp("./p1_sched.out", "./p1_sched.out", argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL) == -1 ? printf("Error occured during execution of P1") : printf("P1 working fine");
        }
    }
    else
    {
        
        

        execlp("./p2_sched.out", "./p2_sched.out", argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL) == -1 ? printf("Error occured during execution of P2") : printf("P2 working fine");


    }
}