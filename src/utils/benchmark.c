#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "error.h"
#include "benchmark.h"

static double now_sec(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int benchmark_cc(int (*cc_func)(const CSCBinaryMatrix*, const int), const CSCBinaryMatrix* mat,
                 int n_threads, int n_trials, const char* name)
{
    double *times;
    double start, end;
    int temp_result;
    int result = 0;

    times = malloc(n_trials * sizeof(double));

    if (!times) {
        print_error(__func__, "malloc() failed", errno);
        return 1;
    }

    for (int i = 0; i < n_trials; i++) {
        start = now_sec();
        temp_result = cc_func(mat, n_threads);
        end = now_sec();

        times[i] = end - start;

        if (result == -1) {
            free(times);
            return 1;
        }

        if (i == 0) {
            result = temp_result;
        } else {
            if (temp_result != result) {
                printf("[%s] Components between retries don't match\n", name);
                free(times);
                return 2;
            }
        }
    }

    double sum = 0.0, sum_sq = 0.0;
    for (int t = 0; t < n_trials; ++t) {
        sum += times[t];
        sum_sq += times[t] * times[t];
    }
    double avg = sum / n_trials;
    double stddev = sqrt(sum_sq / n_trials - avg * avg);

    printf("[%s] Components: %d, avg time over %d runs: %.6f s, stddev: %.6f s\n",
           name, result, n_trials, avg, stddev);

    free(times);

    return 0;
}
