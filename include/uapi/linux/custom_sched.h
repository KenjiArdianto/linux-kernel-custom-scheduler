#ifndef _UAPI_LINUX_FCFS_SCHED_H
#define _UAPI_LINUX_FCFS_SCHED_H

#include <linux/types.h>

#define MAX_PROCESSES 128

struct custom_sched_process {
    __s32 pid;
    __s32 arrival_time;
    __s32 burst_time;
};


struct custom_sched_result {
    double avg_response_time;
    double avg_waiting_time;
    double avg_turnaround_time;
};

#endif
