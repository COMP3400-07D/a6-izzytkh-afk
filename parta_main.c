#include "parta.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/**
 * Command-line front-end for the simple CPU scheduler.
 *
 * Usage:
 *   ./parta_main fcfs <burst0> <burst1> ...
 *   ./parta_main rr <quantum> <burst0> <burst1> ...
 *
 * It:
 *   - Parses the arguments.
 *   - Builds the PCB array via init_procs().
 *   - Runs either FCFS or RR(quantum).
 *   - Prints the accepted processes and the average wait time (2 decimals).
 *
 * On error (e.g., missing arguments), it prints:
 *   ERROR: Missing arguments
 * and exits with status code 1.
 */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("ERROR: Missing arguments\n");
        return 1;
    }

    const char* alg = argv[1];

    if (strcmp(alg, "fcfs") == 0) {
        // Need at least one burst time: ./parta_main fcfs <burst...>
        if (argc < 3) {
            printf("ERROR: Missing arguments\n");
            return 1;
        }

        int plen = argc - 2;
        int* bursts = malloc(sizeof(int) * plen);
        if (!bursts) {
            fprintf(stderr, "ERROR: Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < plen; i++) {
            bursts[i] = atoi(argv[2 + i]);
        }

        printf("Using FCFS\n\n");

        for (int i = 0; i < plen; i++) {
            printf("Accepted P%d: Burst %d\n", i, bursts[i]);
        }

        struct pcb* procs = init_procs(bursts, plen);
        if (!procs) {
            fprintf(stderr, "ERROR: Failed to initialize processes\n");
            free(bursts);
            return 1;
        }

        (void)fcfs_run(procs, plen);

        double sum_wait = 0.0;
        for (int i = 0; i < plen; i++) {
            sum_wait += procs[i].wait;
        }
        double avg_wait = sum_wait / (double)plen;

        printf("Average wait time: %.2f\n", avg_wait);

        free(procs);
        free(bursts);
    } else if (strcmp(alg, "rr") == 0) {
        // Need at least: ./parta_main rr <quantum> <burst...>
        if (argc < 4) {
            printf("ERROR: Missing arguments\n");
            return 1;
        }

        int quantum = atoi(argv[2]);
        int plen = argc - 3;

        int* bursts = malloc(sizeof(int) * plen);
        if (!bursts) {
            fprintf(stderr, "ERROR: Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < plen; i++) {
            bursts[i] = atoi(argv[3 + i]);
        }

        printf("Using RR(%d).\n\n", quantum);

        for (int i = 0; i < plen; i++) {
            printf("Accepted P%d: Burst %d\n", i, bursts[i]);
        }

        struct pcb* procs = init_procs(bursts, plen);
        if (!procs) {
            fprintf(stderr, "ERROR: Failed to initialize processes\n");
            free(bursts);
            return 1;
        }

        (void)rr_run(procs, plen, quantum);

        double sum_wait = 0.0;
        for (int i = 0; i < plen; i++) {
            sum_wait += procs[i].wait;
        }
        double avg_wait = sum_wait / (double)plen;

        printf("Average wait time: %.2f\n", avg_wait);

        free(procs);
        free(bursts);
    } else {
        // Algorithm not recognized
        printf("ERROR: Missing arguments\n");
        return 1;
    }

    return 0;
}
