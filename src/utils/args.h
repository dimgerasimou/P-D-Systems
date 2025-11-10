/**
 * @file args.h
 * @brief Command-line argument parsing interface.
 *
 * This header declares the function used to parse command-line arguments
 * for configuring the program's execution parameters such as number of threads,
 * number of trials, and input file path.
 */

#ifndef ARGS_H
#define ARGS_H

/**
 * @brief Parses command-line arguments.
 *
 * This function reads command-line parameters to determine:
 * - Number of threads (`-t`), default: 8
 * - Number of trials (`-n`), default: 1
 * - Input data file path (mandatory)
 *
 * It validates each argument and reports errors using `print_error()`.
 *
 * @param[in] argc Number of command-line arguments.
 * @param[in] argv Array of command-line arguments.
 * @param[out] n_threads Pointer to store the parsed number of threads.
 * @param[out] n_trials Pointer to store the parsed number of trials.
 * @param[out] filepath Pointer to store the input file path string.
 *
 * @return
 * - `0` on success
 * - `1` on invalid or missing arguments
 * - `-1` if the help flag (`-h`) was used
 */
int parseargs(int argc, char *argv[], int *n_threads, int *n_trials, char **filepath);

#endif /* ARGS_H */
