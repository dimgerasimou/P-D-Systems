/**
 * @file args.c
 * @brief Command-line argument parsing implementation.
 *
 * Provides functions to parse program arguments that specify
 * the number of threads, number of trials, and input file path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "args.h"
#include "error.h"

extern const char *program_name;


/**
 * @brief Checks if a string represents an unsigned integer.
 *
 * @param[in] s String to check.
 * @return 1 if @p s is a valid unsigned integer, 0 otherwise.
 */
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


/**
 * @brief Prints program usage instructions to stdout.
 *
 * Displays the valid command-line options and their expected arguments.
 */
static void
usage(void) {
    printf("./%s [-t n_threads] [-n n_trials] ./data_filepath\n", program_name);
}

/**
 * @copydoc parseargs()
 */
int
parseargs(int argc, char *argv[], int *n_threads, int *n_trials, char **filepath)
{
    *n_threads = 8;
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
