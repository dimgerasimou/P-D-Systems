#ifndef CONNECTED_COMPONENTS_H
#define CONNECTED_COMPONENTS_H

#include "../core/matrix.h"

/**
 * @brief Count connected components using sequential label propagation
 * @param matrix Input sparse binary matrix in CSC format
 * @param n_threads Number of threads to use (unused in sequential algorithm)
 * @return Number of connected components, or -1 on error
 */
int cc_count_sequential(const CSCBinaryMatrix *matrix, const int n_threads);

/**
 * @brief Count connected components using parallel label propagationwith openmp
 * @param matrix Input sparse binary matrix in CSC format
 * @param n_threads Number of threads to use
 * @return Number of connected components, or -1 on error
 */
int cc_count_parallel_omp(const CSCBinaryMatrix *matrix, const int n_threads);

#endif