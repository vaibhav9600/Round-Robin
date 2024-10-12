#ifndef struct_utils
#define struct_utils

#include <time.h>

typedef struct
{
    int flag1;
    int flag2;
    int finished[2];

} proc_data;

typedef struct
{
    double ta_time[2];
    struct timespec switch_time[2];
    double wt_time[2];
    double op_count;
    double op_time[2];
    double run_time[2];

} time_logging;

typedef struct thread_args
{
    int thread_id;

    int start_row;
    int end_row;

    int start_col;
    int end_col;

} thread_args;

#endif