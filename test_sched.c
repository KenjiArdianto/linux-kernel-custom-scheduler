#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

#include <linux/custom_sched.h>

#ifndef __NR_custom_sched
#define __NR_custom_sched 470
#endif

#define SCHED_FCFS 0 
#define SCHED_SJF 1 
#define SCHED_SRT 2 
#define SCHED_RR 3

int main(void)
{
    struct custom_sched_process procs[] = {
        { .pid = 1, .arrival_time = 5, .burst_time = 2 },
        { .pid = 2, .arrival_time = 2, .burst_time = 4 },
        { .pid = 3, .arrival_time = 8, .burst_time = 1 },
        { .pid = 4, .arrival_time = 0, .burst_time = 8 },
        { .pid = 5, .arrival_time = 11, .burst_time = 3 }
    };

    struct custom_sched_result result_fcfs;
    struct custom_sched_result result_sjf;
    struct custom_sched_result result_srt;
    struct custom_sched_result result_rr;

    int n = sizeof(procs) / sizeof(procs[0]);

    long ret_fcfs = syscall(__NR_custom_sched, procs, n, SCHED_FCFS, &result_fcfs);
    if (ret_fcfs < 0) {
        perror("Custom FCFS Scheduler Syscall Failed");
        return 1;
    }
    
    long ret_sjf = syscall(__NR_custom_sched, procs, n, SCHED_SJF, &result_sjf);
    if (ret_sjf < 0) {
        perror("Custom SJF Scheduler Syscall Failed");
        return 1;
    }

    long ret_srt = syscall(__NR_custom_sched, procs, n, SCHED_SRT, &result_srt);
    if (ret_srt < 0) {
        perror("Custom SRT Scheduler Syscall Failed");
        return 1;
    }

    long ret_rr = syscall(__NR_custom_sched, procs, n, SCHED_RR, &result_rr);
    if (ret_rr < 0) {
        perror("Custom RR Scheduler Syscall Failed");
        return 1;
    }

    printf("===================================\n");
    printf("FCFS Scheduling Results:\n");
    printf("Average waiting time     : %.2f\n", result_fcfs.avg_waiting_time);
    printf("Average turnaround time  : %.2f\n", result_fcfs.avg_turnaround_time);
    printf("Average response time    : %.2f\n", result_fcfs.avg_response_time);
    printf("===================================\n");
    printf("SJF Scheduling Results:\n");
    printf("Average waiting time     : %.2f\n", result_sjf.avg_waiting_time);
    printf("Average turnaround time  : %.2f\n", result_sjf.avg_turnaround_time);
    printf("Average response time    : %.2f\n", result_sjf.avg_response_time);
    printf("===================================\n");
    printf("SRT Scheduling Results:\n");
    printf("Average waiting time     : %.2f\n", result_srt.avg_waiting_time);
    printf("Average turnaround time  : %.2f\n", result_srt.avg_turnaround_time);
    printf("Average response time    : %.2f\n", result_srt.avg_response_time);
    printf("===================================\n");
    printf("RR Scheduling Results:\n");
    printf("Average waiting time     : %.2f\n", result_rr.avg_waiting_time);
    printf("Average turnaround time  : %.2f\n", result_rr.avg_turnaround_time);
    printf("Average response time    : %.2f\n", result_rr.avg_response_time);
    printf("===================================\n");

    return 0;
}
