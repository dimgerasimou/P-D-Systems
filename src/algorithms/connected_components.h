/**
 * @file connected_components.h
 * @brief Connected components counting algorithms for sparse binary matrices.
 *
 * Provides sequential and parallel implementations of label propagation
 * algorithms to count connected components in a sparse binary matrix (CSC format).
 *
 * Implementations:
 * - Sequential
 * - OpenMP
 * - Pthreads
 * - OpenCilk
 */

#ifndef CONNECTED_COMPONENTS_H
#define CONNECTED_COMPONENTS_H

#include "../core/matrix.h"

/**
 * @brief Count connected components using sequential label propagation
 * @param matrix Input sparse binary matrix in CSC format
 * @param n_threads Number of threads to use (unused in sequential algorithm)
 * @return Number of connected components, or -1 on error
 */
int cc_sequential(const CSCBinaryMatrix *matrix, const int n_threads);

/**
 * @brief Count connected components using parallel label propagation with openmp
 * @param matrix Input sparse binary matrix in CSC format
 * @param n_threads Number of threads to use
 * @return Number of connected components, or -1 on error
 */
int cc_openmp(const CSCBinaryMatrix *matrix, const int n_threads);

/**
 * @brief Count connected components using parallel label propagation with opencilk
 * @param matrix Input sparse binary matrix in CSC format
 * @param n_threads Number of threads to use
 * @return Number of connected components, or -1 on error
 */
int cc_cilk(const CSCBinaryMatrix *matrix, const int n_threads);

/**
 * @brief Count connected components using parallel label propagation with pthreads
 * @param matrix Input sparse binary matrix in CSC format
 * @param n_threads Number of threads to use
 * @return Number of connected components, or -1 on error
 */
int cc_pthreads(const CSCBinaryMatrix *matrix, const int n_threads);

#endif
