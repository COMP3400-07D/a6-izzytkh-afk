#include "parta.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Initialize an array of PCBs on the heap from an array of CPU burst times.
 *
 * Each PCB i is initialized as:
 *   - pid        = i
 *   - burst_left = bursts[i]
 *   - wait       = 0
 *
 * @param bursts Array of CPU burst times.
 * @param blen   Number of elements in bursts.
 * @return       Pointer to a newly allocated array of struct pcb of length blen,
 *               or NULL if blen <= 0, bursts is NULL, or allocation fails.
 */
struct pcb* init_procs(int* bursts, int blen) {
    if (blen <= 0 || bursts == NULL) {
        return NULL;
    }

    struct pcb* procs = malloc(sizeof(struct pcb) * blen);
    if (!procs) {
        return NULL;
    }

    for (int i = 0; i < blen; i++) {
        procs[i].pid        = i;
        procs[i].burst_left = bursts[i];
        procs[i].wait       = 0;
    }

    return procs;
}

/**
 * Helper function for debugging: print each PCB's pid, burst_left, and wait.
 */
void printall(struct pcb* procs, int plen) {
    if (!procs || plen <= 0) {
        return;
    }

    for (int i = 0; i < plen; i++) {
        printf("P%d: burst_left=%d wait=%d\n",
               procs[i].pid, procs[i].burst_left, procs[i].wait);
    }
}

/**
 * "Run" the process at index 'current' for 'amount' units of CPU time.
 *
 * - The current process's burst_left is reduced by the time actually used.
 * - Every *other* process that is not yet finished (burst_left > 0)
 *   has its wait time increased by that same amount.
 *
 * If 'amount' is larger than the remaining burst of the current process,
 * only the remaining burst is actually used.
 */
void run_proc(struct pcb* procs, int plen, int current, int amount) {
    if (!procs || plen <= 0 || current < 0 || current >= plen || amount <= 0) {
        return;
    }

    int remaining = procs[current].burst_left;
    if (remaining <= 0) {
        return;
    }

    int used = amount;
    if (used > remaining) {
        used = remaining;
    }

    procs[current].burst_left -= used;
    if (procs[current].burst_left < 0) {
        procs[current].burst_left = 0;
    }

    // Everyone else who is still not finished waits for 'used' time units.
    for (int i = 0; i < plen; i++) {
        if (i == current) {
            continue;
        }
        if (procs[i].burst_left > 0) {
            procs[i].wait += used;
        }
    }
}

/**
 * Run a First-Come-First-Serve (FCFS) schedule on the given processes.
 *
 * Starting with P0, each process runs until completion before moving on to
 * the next one. This function mutates the 'procs' array (burst_left and wait)
 * and returns the total time elapsed when all processes are done.
 */
int fcfs_run(struct pcb* procs, int plen) {
    if (!procs || plen <= 0) {
        return 0;
    }

    int time = 0;

    for (int i = 0; i < plen; i++) {
        if (procs[i].burst_left <= 0) {
            continue;
        }

        int amount = procs[i].burst_left;
        run_proc(procs, plen, i, amount);
        time += amount;
    }

    return time;
}

/**
 * Compute the next process to run for Round-Robin scheduling.
 *
 * @param current Index of the process that ran most recently (previous).
 *                Pass -1 to select the first runnable process.
 * @param procs   Array of PCBs.
 * @param plen    Number of PCBs.
 *
 * @return Index of the next process to run, or -1 if all processes are complete.
 *
 * Rules:
 *  - Search starting from current+1 (wrapping around).
 *  - Return the first index with burst_left > 0.
 *  - If no *other* such index exists but 'current' still has burst_left > 0,
 *    return 'current' again.
 *  - If all processes have burst_left == 0, return -1.
 */
int rr_next(int current, struct pcb* procs, int plen) {
    if (!procs || plen <= 0) {
        return -1;
    }

    // Check if all processes are done.
    int any_remaining = 0;
    for (int i = 0; i < plen; i++) {
        if (procs[i].burst_left > 0) {
            any_remaining = 1;
            break;
        }
    }
    if (!any_remaining) {
        return -1;
    }

    // First call or invalid 'current': pick the first runnable process.
    if (current < 0 || current >= plen) {
        for (int i = 0; i < plen; i++) {
            if (procs[i].burst_left > 0) {
                return i;
            }
        }
        return -1;
    }

    // Look for the next runnable process after 'current'.
    int i = (current + 1) % plen;
    while (i != current) {
        if (procs[i].burst_left > 0) {
            return i;
        }
        i = (i + 1) % plen;
    }

    // No other runnable process found. If current is still runnable, stay on it.
    if (procs[current].burst_left > 0) {
        return current;
    }

    // Safety fallback (should not normally happen).
    for (int j = 0; j < plen; j++) {
        if (procs[j].burst_left > 0) {
            return j;
        }
    }

    return -1;
}

/**
 * Run a Round-Robin (RR) schedule on the given processes with the given quantum.
 *
 * Each iteration:
 *  - rr_next(...) selects the next runnable process.
 *  - That process runs for min(quantum, burst_left) time units.
 *  - run_proc(...) updates burst_left and wait for all processes.
 *
 * The function mutates the 'procs' array and returns the total time elapsed
 * when all processes are finished.
 */
int rr_run(struct pcb* procs, int plen, int quantum) {
    if (!procs || plen <= 0 || quantum <= 0) {
        return 0;
    }

    int time = 0;
    int current = -1; // previous process index for rr_next

    while (1) {
        current = rr_next(current, procs, plen);
        if (current == -1) {
            break; // all processes finished
        }

        int remaining = procs[current].burst_left;
        if (remaining <= 0) {
            continue;
        }

        int amount = (remaining < quantum) ? remaining : quantum;
        run_proc(procs, plen, current, amount);
        time += amount;
    }

    return time;
}
