#include "connected_components.h"
#include "matrix.h"
#include "error.h"
#include "benchmark.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(USE_OPENMP)
    #define IMPLEMENTATION_NAME "OpenMP"
#elif defined(USE_PTHREADS)
    #define IMPLEMENTATION_NAME "Pthreads"
#elif defined(USE_CILK)
    #define IMPLEMENTATION_NAME "OpenCilk"
#elif defined(USE_SEQUENTIAL)
    #define IMPLEMENTATION_NAME "Sequential"
#else
    #error "No implementation selected! Define USE_SEQUENTIAL, USE_OPENMP, USE_PTHREADS, or USE_CILK"
#endif

const char *program_name = "connected_components";

static int
isuint(const char *s)
{
    if (!s || s[0] == '\0')
        return 0;

    for (const char *ptr = s; *ptr != '\0'; ptr++) {
        if (!(*ptr >= '0' && *ptr <= '9'))
            return 0;
    }

    return 1;
}

static void
usage(void) {
    printf("./%s [-t n_threads] [-n n_trials] ./data_filepath\n", program_name);
}

static int parseargs(int argc, char *argv[], int *n_threads, int *n_trials, char **filepath) {
    *n_threads = 0;
    *n_trials = 1;
    *filepath = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) {
            if (i + 1 >= argc) {
                print_error(__func__, "missing argument for -t", 0);
                usage();
                return 1;
            }
            i++;
            if (!isuint(argv[i])) {
                print_error(__func__, "invalid argument type for -t", 0);
                usage();
                return 1;
            }
            *n_threads = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-n")) {
            if (i + 1 >= argc) {
                print_error(__func__, "missing argument for -n", 0);
                usage();
                return 1;
            }
            i++;
            if (!isuint(argv[i])) {
                print_error(__func__, "invalid argument type for -n", 0);
                usage();
                return 1;
            }
            *n_trials = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-h")) {
            usage();
            return -1;
        }
        else {
            if (*filepath != NULL) {
                print_error(__func__, "multiple file paths specified", 0);
                usage();
                return 1;
            }
            if (access(argv[i], R_OK) != 0) {
                char err[256];
                snprintf(err, sizeof(err), "cannot access file: \"%s\"", argv[i]);
                print_error(__func__, err, 0);
                usage();
                return 1;
            }
            *filepath = argv[i];
        }
    }

    if (*filepath == NULL) {
        print_error(__func__, "no input file specified", 0);
        usage();
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {

    CSCBinaryMatrix *matrix;
    char *filepath;
    int n_trials = 1;
    int n_threads = 0;
    int ret = 0;

    set_program_name(argv[0]);

    if (parseargs(argc, argv, &n_threads, &n_trials, &filepath)) {
        return 1;
    }
    
    matrix = csc_load_matrix(filepath, "Problem", "A");
    if (!matrix)
        return 1;

    #if defined(USE_OPENMP)
    ret = benchmark_cc(cc_openmp, matrix, n_threads, n_trials, IMPLEMENTATION_NAME);
    #elif defined(USE_PTHREADS)
    ret = benchmark_cc(cc_pthreads, matrix, n_threads, n_trials, IMPLEMENTATION_NAME);
    #elif defined(USE_CILK)
    ret = benchmark_cc(cc_cilk, matrix, n_threads, n_trials, IMPLEMENTATION_NAME);
    #elif defined(USE_SEQUENTIAL)
    ret = benchmark_cc(cc_sequential, matrix, n_threads, n_trials, IMPLEMENTATION_NAME);
    #endif

    csc_free_matrix(matrix);
    return ret;
}
