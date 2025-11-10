/**
 * @file benchmark.h
 * @brief Benchmarking framework for connected components algorithms.
 *
 * This module provides utilities to time and analyze the performance
 * of graph algorithms (e.g., connected components) operating on sparse
 * matrices in Compressed Sparse Column (CSC) format.
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "matrix.h"

/**
 * @brief Holds benchmark results and metadata.
 */
typedef struct {
    char   *algorithm_name;       /**< Name of the algorithm being benchmarked. */
    char   *dataset_filepath;     /**< Path to the dataset file used for benchmarking. */
    double *times;                /**< Array of trial execution times in seconds. */

    double time_max;              /**< Maximum execution time among trials. */
    double time_min;              /**< Minimum execution time among trials. */
    double time_median;           /**< Median execution time among trials. */
    double time_avg;              /**< Average execution time among trials. */
    double time_stddev;           /**< Standard deviation of execution times. */

    int    connected_components;  /**< Number of connected components found. */

    uint8_t  n_threads;           /**< Number of threads used. */
    uint16_t n_trials;            /**< Number of benchmark trials. */

    uint32_t matrix_rows;         /**< Number of rows in the input matrix. */
    uint32_t matrix_cols;         /**< Number of columns in the input matrix. */
    uint32_t matrix_nnz;          /**< Number of nonzero elements in the matrix. */
} Benchmark;

/**
 * @brief Initializes a benchmark structure.
 *
 * Allocates and populates a new Benchmark instance for the specified
 * algorithm and dataset. Also allocates memory for timing results.
 *
 * @param name Name of the algorithm being benchmarked.
 * @param filepath Path to the dataset file.
 * @param n_trials Number of trials to run.
 * @param n_threads Number of threads used in the algorithm.
 * @param mat Pointer to the CSCBinaryMatrix used as input.
 *
 * @return Pointer to a newly allocated Benchmark structure, or `NULL` on failure.
 */
Benchmark* benchmark_init(const char *name,
                          const char *filepath,
                          const unsigned int n_trials,
                          const unsigned int n_threads,
                          const CSCBinaryMatrix *mat);

/**
 * @brief Frees a Benchmark structure and all allocated resources.
 *
 * @param b Pointer to the Benchmark structure to free. Safe to call with NULL.
 */
void benchmark_free(Benchmark *b);

/**
 * @brief Runs a connected components benchmark.
 *
 * Executes the provided connected components function multiple times,
 * measuring execution time per trial and verifying consistency of results.
 *
 * @param cc_func Pointer to the connected components function to benchmark.
 * @param m Input CSCBinaryMatrix.
 * @param b Benchmark object containing configuration and result storage.
 *
 * @return
 * - `0` on success,
 * - `1` on algorithm failure or invalid data,
 * - `2` if results differ between trials.
 */
int benchmark_cc(int (*cc_func)(const CSCBinaryMatrix*, const int), const CSCBinaryMatrix *m, Benchmark *b);

/**
 * @brief Prints benchmark results in structured JSON format.
 *
 * Outputs benchmark metadata, timing statistics, system information,
 * and matrix properties in JSON form for easy parsing or logging.
 *
 * @param b Pointer to the Benchmark structure with populated data.
 */
void benchmark_print(Benchmark *b);

#endif /* BENCHMARK_H */
