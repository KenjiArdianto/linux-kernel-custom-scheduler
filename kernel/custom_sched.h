#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/limits.h>

#include <uapi/linux/custom_sched.h>

#define SCHED_FCFS 0
#define SCHED_SJF  1
#define SCHED_SRT  2
#define SCHED_RR   3

#define RR_QUANTUM 2

SYSCALL_DEFINE4(custom_sched,
    struct custom_sched_process __user *, procs,
    int, n,
    int, policy,
    struct custom_sched_result __user *, result)
{
    struct custom_sched_process *kprocs;
    struct custom_sched_result kres = {0};

    int i;
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

    /* ================= FCFS ================= */
    if (policy == SCHED_FCFS) {
        int current_time = 0;

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
    }

    /* ================= SJF (NON-PREEMPTIVE) ================= */
    else if (policy == SCHED_SJF) {
        int current_time = 0, completed = 0;
        int visited[MAX_PROCESSES] = {0};

        while (completed < n) {
            int idx = -1;
            int min_burst = INT_MAX;

            for (i = 0; i < n; i++) {
                if (!visited[i] && kprocs[i].arrival_time <= current_time) {
                    if (kprocs[i].burst_time < min_burst) {
                        min_burst = kprocs[i].burst_time;
                        idx = i;
                    }
                }
            }

            if (idx == -1) {
                current_time++;
                continue;
            }

            int response_time =
                current_time - kprocs[idx].arrival_time;
            int waiting_time = response_time;
            int turnaround_time =
                waiting_time + kprocs[idx].burst_time;

            total_response   += response_time;
            total_waiting    += waiting_time;
            total_turnaround += turnaround_time;

            current_time += kprocs[idx].burst_time;
            visited[idx] = 1;
            completed++;
        }
    }

    /* ================= SRT (PREEMPTIVE) ================= */
    else if (policy == SCHED_SRT) {
        int remaining[MAX_PROCESSES];
        int completed = 0;
        int current_time = 0;
        int first_exec[MAX_PROCESSES];

        for (i = 0; i < n; i++) {
            remaining[i] = kprocs[i].burst_time;
            first_exec[i] = -1;
        }

        while (completed < n) {
            int idx = -1;
            int min_remain = INT_MAX;

            for (i = 0; i < n; i++) {
                if (kprocs[i].arrival_time <= current_time &&
                    remaining[i] > 0 &&
                    remaining[i] < min_remain) {
                    min_remain = remaining[i];
                    idx = i;
                }
            }

            if (idx == -1) {
                current_time++;
                continue;
            }

            if (first_exec[idx] == -1) {
                first_exec[idx] = current_time;
                total_response +=
                    current_time - kprocs[idx].arrival_time;
            }

            remaining[idx]--;
            current_time++;

            if (remaining[idx] == 0) {
                int turnaround_time =
                    current_time - kprocs[idx].arrival_time;
                int waiting_time =
                    turnaround_time - kprocs[idx].burst_time;

                total_waiting    += waiting_time;
                total_turnaround += turnaround_time;
                completed++;
            }
        }
    }

    /* ================= ROUND ROBIN ================= */
    else if (policy == SCHED_RR) {
        int remaining[MAX_PROCESSES];
        int visited[MAX_PROCESSES] = {0};
        int queue[MAX_PROCESSES];
        int front = 0, rear = 0;
        int completed = 0;
        int current_time = 0;

        for (i = 0; i < n; i++)
            remaining[i] = kprocs[i].burst_time;

        queue[rear++] = 0;
        visited[0] = 1;

        while (completed < n) {
            int idx = queue[front++];

            if (front == MAX_PROCESSES)
                front = 0;

            if (remaining[idx] == kprocs[idx].burst_time) {
                total_response +=
                    current_time - kprocs[idx].arrival_time;
            }

            int exec =
                (remaining[idx] > RR_QUANTUM)
                    ? RR_QUANTUM
                    : remaining[idx];

            remaining[idx] -= exec;
            current_time += exec;

            for (i = 0; i < n; i++) {
                if (!visited[i] &&
                    kprocs[i].arrival_time <= current_time) {
                    queue[rear++] = i;
                    visited[i] = 1;
                }
            }

            if (remaining[idx] > 0) {
                queue[rear++] = idx;
            } else {
                int turnaround_time =
                    current_time - kprocs[idx].arrival_time;
                int waiting_time =
                    turnaround_time - kprocs[idx].burst_time;

                total_waiting    += waiting_time;
                total_turnaround += turnaround_time;
                completed++;
            }
        }
    }

    else {
        kfree(kprocs);
        return -EINVAL;
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

