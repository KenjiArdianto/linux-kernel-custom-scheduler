#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

#include <linux/sched_fcfs.h>

#ifndef __NR_fcfs_schedule
#define __NR_fcfs_schedule 550
#endif

int main(void)
{
    struct fcfs_process procs[] = {
        { .pid = 1, .arrival_time = 0, .burst_time = 5 },
        { .pid = 2, .arrival_time = 2, .burst_time = 3 },
        { .pid = 3, .arrival_time = 4, .burst_time = 1 }
    };

    struct fcfs_result result;

    int n = sizeof(procs) / sizeof(procs[0]);

    long ret = syscall(__NR_fcfs_schedule, procs, n, &result);
    if (ret < 0) {
        perror("sched_fcfs syscall failed");
        return 1;
    }

    printf("FCFS Scheduling Results:\n");
    printf("------------------------\n");
    printf("Average waiting time     : %.2f\n", result.avg_waiting_time);
    printf("Average turnaround time  : %.2f\n", result.avg_turnaround_time);
    printf("Average response time    : %.2f\n", result.avg_response_time);

    return 0;
}
