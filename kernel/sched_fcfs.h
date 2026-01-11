#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <uapi/linux/fcfs_sched.h>

SYSCALL_DEFINE3(fcfs_schedule,
    struct fcfs_process __user *, procs,
    int, n,
    struct fcfs_result __user *, result)
{
    struct fcfs_process *kprocs;
    struct fcfs_result kres = {0};

    int i;
    int current_time = 0;
    double total_response = 0;
    double total_waiting = 0;
    double total_turnaround = 0;

    if (n <= 0 || n > MAX_PROCESSES)
        return -EINVAL;

    kprocs = kmalloc_array(n, sizeof(*kprocs), GFP_KERNEL);
    if (!kprocs)
        return -ENOMEM;

    if (copy_from_user(kprocs, procs, n * sizeof(*kprocs))) {
        kfree(kprocs);
        return -EFAULT;
    }

    /* FCFS scheduling simulation */
    for (i = 0; i < n; i++) {
        int start_time;

        if (current_time < kprocs[i].arrival_time)
            current_time = kprocs[i].arrival_time;

        start_time = current_time;

        int response_time   = start_time - kprocs[i].arrival_time;
        int waiting_time    = response_time;
        int turnaround_time = waiting_time + kprocs[i].burst_time;

        total_response   += response_time;
        total_waiting    += waiting_time;
        total_turnaround += turnaround_time;

        current_time += kprocs[i].burst_time;
    }

    kres.avg_response_time   = total_response / n;
    kres.avg_waiting_time    = total_waiting / n;
    kres.avg_turnaround_time = total_turnaround / n;

    if (copy_to_user(result, &kres, sizeof(kres))) {
        kfree(kprocs);
        return -EFAULT;
    }

    kfree(kprocs);
    return 0;
}
